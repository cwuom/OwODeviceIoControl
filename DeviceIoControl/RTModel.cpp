//////////////////// RTModel.cpp ////////////////////

#include "framework.h"
#include "RTModel.h"
#include <future>
#include <fstream>
#include <iostream>

// Global definitions
bool isModelLoaded = false;
bool isModelUnloadable = false;

static physx::PxDefaultAllocator gAllocator;
static physx::PxDefaultErrorCallback gErrorCallback;
physx::PxFoundation* RTModel::gFoundation = nullptr;
physx::PxPhysics* RTModel::gPhysics = nullptr;
physx::PxScene* RTModel::gScene = nullptr;
physx::PxDefaultCpuDispatcher* RTModel::gDispatcher = nullptr;
physx::PxMaterial* RTModel::gMaterial = nullptr;
physx::PxCooking* RTModel::gCooking = nullptr;
std::mutex RTModel::gPhysXMutex;


// Model manager
class ModelManager {
private:
    std::vector<Ver> models;
public:
    void delModel() {
        models.clear();
    }

    bool loadAllFromBinary(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            std::cerr << "[PhysX] Map model: " << filename << " failed to load! Make sure the 'LModes' folder is in the .exe directory." << std::endl;
            return false;
        }
        else
        {
            size_t modelCount;
            inFile.read(reinterpret_cast<char*>(&modelCount), sizeof(modelCount));
            models.reserve(modelCount); // Pre-allocate memory

            for (size_t i = 0; i < modelCount; ++i) {
                Ver model;
                model.read(inFile);
                models.push_back(model);
            }
            inFile.close();
            return true;
        }
    }

    const std::vector<Ver>& getModels() const {
        return models;
    }
};

static ModelManager manager; // Static instance

// Cooking structures and function
struct CookedMeshData {
    bool valid = false;
    std::vector<uint8_t> buffer;
    Ver temp;
};

CookedMeshData CookModelMesh(const Ver& temp) {
    CookedMeshData result;

    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = temp.VerticesCount;
    meshDesc.points.stride = sizeof(physx::PxVec3);
    meshDesc.points.data = const_cast<Vector3*>(&temp.Vertices[0]); // PxVec3 and Vector3 are layout-compatible
    meshDesc.triangles.count = temp.IndicesCount;
    meshDesc.triangles.stride = 3 * sizeof(physx::PxU16);
    meshDesc.triangles.data = const_cast<uint16_t*>(&temp.Indices[0]);
    meshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES;

    if (!meshDesc.isValid()) {
        return result;
    }

    if (!RTModel::gCooking) {
        return result;
    }

    physx::PxDefaultMemoryOutputStream writeBuffer;
    physx::PxTriangleMeshCookingResult::Enum resultCode;

    bool success = RTModel::gCooking->cookTriangleMesh(meshDesc, writeBuffer, &resultCode);
    if (!success) {
        return result;
    }

    result.valid = true;
    result.buffer.resize(writeBuffer.getSize());
    memcpy(result.buffer.data(), writeBuffer.getData(), writeBuffer.getSize());
    result.temp = temp; // Copy metadata

    return result;
}


// --- RTModel Namespace Function Implementations ---

void RTModel::Init() {
    std::lock_guard<std::mutex> lock(gPhysXMutex);
    if (gFoundation) return; // Prevent double-init

    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) { std::cerr << "[PhysX] PxCreateFoundation failed!" << std::endl; return; }

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true);
    if (!gPhysics) { std::cerr << "[PhysX] PxCreatePhysics failed!" << std::endl; return; }

    physx::PxCookingParams params(gPhysics->getTolerancesScale());
    params.suppressTriangleMeshRemapTable = true;
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, params);
    if (!gCooking) { std::cerr << "[PhysX] PxCreateCooking failed!" << std::endl; return; }

    gDispatcher = physx::PxDefaultCpuDispatcherCreate(2); // Use 2 threads
    if (!gDispatcher) { std::cerr << "[PhysX] PxDefaultCpuDispatcherCreate failed!" << std::endl; return; }

    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f); // Z-axis up (UE)
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);
    if (!gScene) { std::cerr << "[PhysX] createScene failed!" << std::endl; return; }

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.5f);
    if (!gMaterial) { std::cerr << "[PhysX] createMaterial failed!" << std::endl; return; }
}

bool RTModel::DelModel() {
    {
        std::lock_guard<std::mutex> lock(gPhysXMutex);
        // Release resources in reverse order of creation
        if (gMaterial) { gMaterial->release(); gMaterial = nullptr; }

        if (gScene) {
            gScene->release();
            gScene = nullptr;
        }
        // Dispatcher 必须在 Scene 之后释放
        if (gDispatcher) {
            gDispatcher->release();
            gDispatcher = nullptr;
        }

        if (gCooking) { gCooking->release(); gCooking = nullptr; }
        if (gPhysics) { gPhysics->release(); gPhysics = nullptr; }
        if (gFoundation) { gFoundation->release(); gFoundation = nullptr; }
    }

    manager.delModel(); // Clear model data from manager
    isModelLoaded = false;
    isModelUnloadable = false;
    return true;
}

bool RTModel::OfflineUpdate(std::string Name) {
    if (manager.loadAllFromBinary(Name))
    {
        const auto& models = manager.getModels();
        const size_t modelCount = models.size();
        if (modelCount == 0) {
            std::cerr << "[PhysX] " << Name << " contains 0 models." << std::endl;
            return false;
        }

        // Cook models asynchronously
        std::vector<CookedMeshData> cookedResults(modelCount);
        std::vector<std::future<CookedMeshData>> futures;
        futures.reserve(modelCount);

        for (const auto& temp : models) {
            futures.push_back(std::async(std::launch::async, CookModelMesh, temp));
        }

        for (size_t i = 0; i < futures.size(); i++) {
            cookedResults[i] = futures[i].get();
        }

        // Add cooked models to the scene
        std::lock_guard<std::mutex> lock(gPhysXMutex);
        if (!gScene) {
            std::cerr << "[PhysX] Scene is null during OfflineUpdate." << std::endl;
            return false;
        }

        gScene->lockWrite();
        for (const auto& result : cookedResults) {
            if (!result.valid) continue;

            physx::PxDefaultMemoryInputData readBuffer(
                const_cast<physx::PxU8*>(result.buffer.data()),
                static_cast<physx::PxU32>(result.buffer.size())
            );

            physx::PxTriangleMesh* triangleMesh = gPhysics->createTriangleMesh(readBuffer);
            if (!triangleMesh) continue;

            physx::PxTransform actorTransform;
            actorTransform.q = physx::PxQuat(result.temp.VerticesQ.x, result.temp.VerticesQ.y, result.temp.VerticesQ.z, result.temp.VerticesQ.w);
            actorTransform.p = physx::PxVec3(result.temp.MmodelLocation.x, result.temp.MmodelLocation.y, result.temp.MmodelLocation.z);

            physx::PxRigidStatic* Actor = gPhysics->createRigidStatic(actorTransform);

            physx::PxTriangleMeshGeometry geometry(
                triangleMesh,
                physx::PxVec3(result.temp.Verticesscale.x, result.temp.Verticesscale.y, result.temp.Verticesscale.z)
            );

            physx::PxShape* shape = gPhysics->createShape(geometry, *gMaterial);
            if (shape) {
                Actor->attachShape(*shape);
                shape->release();
            }

            triangleMesh->release();
            gScene->addActor(*Actor);
        }
        gScene->unlockWrite();

        return true;
    }
    return false;
}

bool RTModel::IsVisible(Vector3 Start, Vector3 End) {
    std::lock_guard<std::mutex> lock(gPhysXMutex); // 获取锁，确保线程安全
    if (!gScene) {
        return true;
    }

    physx::PxVec3 pxStart(Start.x, Start.y, Start.z);
    physx::PxVec3 pxEnd(End.x, End.y, End.z);

    if (!pxStart.isFinite() || !pxEnd.isFinite()) {
        return true;
    }

    physx::PxVec3 direction = pxEnd - pxStart;
    physx::PxReal distance = direction.magnitude();
    if (distance < 1e-6f) {
        return true;
    }

    direction.normalize(); // 标准化方向向量
    physx::PxRaycastBuffer hit; // 用于存储光线投射结果
    // 使用 PxQueryFlag::eSTATIC 确保只检测静态地图几何体
    bool isBlocked = gScene->raycast(pxStart, direction, distance, hit, physx::PxHitFlag::eDEFAULT, physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC));

    bool final_visible = !isBlocked || !hit.hasBlock; // 如果没有阻挡物，或者有阻挡物但 hit 无效，则视为可见

    return final_visible;
}

// --- Map loading helper functions ---
void RTModel::loadFiringRangeModel() {
    if (OfflineUpdate("LModes/bachang")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}
void RTModel::loadDamModel() {
    if (OfflineUpdate("LModes/linghaodaba")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}
void RTModel::loadValleyModel() {
    if (OfflineUpdate("LModes/changgongxigu")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}
void RTModel::loadBaqqashModel() {
    if (OfflineUpdate("LModes/bakeshen")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}
void RTModel::loadSpaceCenterModel() {
    if (OfflineUpdate("LModes/hangtianjidi")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}
void RTModel::loadTidalPrisonModel() {
    if (OfflineUpdate("LModes/chaoxijianyu")) {
        isModelLoaded = true;
        isModelUnloadable = true;
    }
}