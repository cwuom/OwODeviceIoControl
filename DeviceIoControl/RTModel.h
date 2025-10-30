//////////////////// RTModel.h ////////////////////

#pragma once
#include <PxPhysicsAPI.h>
#include <vector>
#include <mutex>
#include <string>
#include "GameTypes.h"

extern bool isModelLoaded;
extern bool isModelUnloadable;

namespace RTModel
{
    // PhysX Core Objects
    extern physx::PxFoundation* gFoundation;
    extern physx::PxPhysics* gPhysics;
    extern physx::PxScene* gScene;
    extern physx::PxDefaultCpuDispatcher* gDispatcher;
    extern physx::PxMaterial* gMaterial;
    extern physx::PxCooking* gCooking;
    extern std::mutex gPhysXMutex;

    // Init and Shutdown
    void Init();
    bool DelModel();

    // Core Functionality
    bool OfflineUpdate(std::string Name);
    bool IsVisible(Vector3 Start, Vector3 End);

    // Map loading helpers
    void loadFiringRangeModel();
    void loadDamModel();
    void loadValleyModel();
    void loadBaqqashModel();
    void loadSpaceCenterModel();
    void loadTidalPrisonModel();
}