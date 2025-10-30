//////////////////// GameTypes.h ////////////////////

#pragma once
#include <DirectXMath.h>
#include <string>
#include <fstream>
#include <vector>
#include "common_packets.h"
#include <chrono>

// 3D 向量
struct Vector3 {
	float x, y, z;

    float Distance(const Vector3& v) const {
        float dx = v.x - x;
        float dy = v.y - y;
        float dz = v.z - z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

// 4x4 矩阵结构体，用于手动计算
struct Matrix4x4 {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

// ScriptStruct CoreUObject.EncHandler
// Struct Size::0x0004
#pragma pack(push, 1)
struct FEncHandler
{
    uint16_t Index;        // 0x0000(0x0002)
    int8_t   bEncrypted;   // 0x0002(0x0001)
    char     pa_0003[1];   // 0x0003(0x0001)
};
#pragma pack(pop)

// ScriptStruct CoreUObject.EncVector
// Struct Size::0x0010
#pragma pack(push, 1)
struct FEncVector
{
    float       X;         // 0x0000(0x0004)
    float       Y;         // 0x0004(0x0004)
    float       Z;         // 0x0008(0x0004)
    FEncHandler EncHandler;  // 0x000C(0x0004)
};
#pragma pack(pop)

// UE变换结构体 FTransform
// 用于 ComponentToWorld
// Struct Size::0x0040
#pragma pack(push, 1)
struct FTransform
{
    DirectX::XMFLOAT4 Rotation;    // 0x0000(0x0010)
    DirectX::XMFLOAT3 Translation; // 0x0010(0x000C)
    char              pa_001C[4];  // 0x001C(0x0004)
    DirectX::XMFLOAT3 Scale3D;     // 0x0020(0x000C)
    char              pa_002C[4];  // 0x002C(0x0004)
    FEncHandler       EncHandler;  // 0x0030(0x0004)
    char              pa_0034[12]; // 0x0034(0x000C)
};
#pragma pack(pop)

// 骨骼变换结构体 FBoneTransform
// 用于 BoneArray 数组
// Struct Size::0x0030
#pragma pack(push, 1)
struct FBoneTransform {
    DirectX::XMFLOAT4 Rotation;    // 0x00 (16 字节)
    DirectX::XMFLOAT3 Translation; // 0x10 (12 字节)
    char              pa_001C[4];  // 0x1C (4 字节)
    DirectX::XMFLOAT3 Scale3D;     // 0x20 (12 字节)
    char              pa_002C[4];  // 0x2C (4 字节)
}; // 总大小: 16+12+4+12+4 = 48 字节 (0x30)
#pragma pack(pop)

// ScriptStruct DFMGameplay.EquipmentInfo
// Struct Size::0x0030
#pragma pack(push, 1)
struct EquipmentInfo
{
    uint64_t ItemID; // 0x0000(0x0008)
    uint64_t Gid; // 0x0008(0x0008)
    float Health; // 0x0010(0x0004)
    float MaxHealth; // 0x0014(0x0004)
    float Durability; // 0x0018(0x0004)
    float MaxDurability; // 0x001C(0x0004)
    float TotalEquipSeceonds; // 0x0020(0x0004)
    float LastEquipTimeSeconds; // 0x0024(0x0004)
    float TotalApplyDamage; // 0x0028(0x0004)
    char pa_002C[0x0004]; // 0x002C(0x0004)
};
#pragma pack(pop)

// 静态物体的分类
enum class StaticClass : uint8_t {
    Unknown = 0,
    Item = 1,
    PlayerLootBox = 2,
    Container = 3,
    Safe = 4,
    HackerPC = 5,
    ExtractionPoint = 6
};

// 战利品盒子的所有者类型
enum class LootBoxOwnerType : uint8_t {
    Unknown,
    Player,
    AI
};

// 用于解决数据加载延迟问题的实体验证状态机
enum class ValidationState : uint8_t {
    Unverified,
    Pending_Player,
    IsPlayer,
    IsAI,
    IsTeammate
};

// 缓存的玩家信息结构体
struct CachedPlayer {
    uint64_t actor_ptr = 0;
    uint64_t player_state_ptr = 0;
    uint64_t health_set_ptr = 0;
    uint64_t root_component_ptr = 0;
    uint64_t equip_info_array_ptr = 0;
    std::string name;
    std::string class_name;
    ValidationState validation_state = ValidationState::Unverified;
    Vector3 location{ 0,0,0 };
    DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
    float health = 0.f;
    int team_id = -1;
    int helmet_level = 0;
    int armor_level = 0;
    float helmet_hp;
    float armor_hp;
    float armor_max_hp;
    float helmet_max_hp;
    bool is_ai = false;
    std::string operator_name;
    std::string ai_type;
    std::string held_weapon;
    DirectX::XMFLOAT3 skeleton[NUM_BONES]{};
    bool client_knows_about_it = false;
    int verification_retries = 0;
    std::chrono::steady_clock::time_point last_aiming_at_me_time;
};

// 缓存的静态物体（物品、容器等）信息结构体
struct CachedStatic {
    uint64_t actor_ptr = 0;
    uint64_t root_component_ptr = 0;
    uint64_t item_component_ptr = 0;
    int object_id = 0;
    std::string class_name;
    StaticClass cls = StaticClass::Unknown;
    Vector3 location{ 0,0,0 };
    LootBoxOwnerType box_owner = LootBoxOwnerType::Unknown;
    bool is_opened = false;
    bool last_sent_opened_state = false;
};

// 地图信息
struct Mapinfo
{
    float X = 0;
    float Y = 0;
    float W = 0;
    float H = 0;
    int MapX = 0;
    int MapY = 0;
};

// --- Physics ---

// 从文件中读取 FVector 数据
inline void readFVector(std::ifstream& in, Vector3& vector) {
    in.read(reinterpret_cast<char*>(&vector.x), sizeof(vector.x));
    in.read(reinterpret_cast<char*>(&vector.y), sizeof(vector.y));
    in.read(reinterpret_cast<char*>(&vector.z), sizeof(vector.z));
}

// 从文件中读取 FQuat 数据
inline void readFQuat(std::ifstream& in, DirectX::XMFLOAT4& quat) {
    in.read(reinterpret_cast<char*>(&quat.x), sizeof(quat.x));
    in.read(reinterpret_cast<char*>(&quat.y), sizeof(quat.y));
    in.read(reinterpret_cast<char*>(&quat.z), sizeof(quat.z));
    in.read(reinterpret_cast<char*>(&quat.w), sizeof(quat.w));
}

// 将 FVector 数据写入文件
inline void writeFVector(std::ofstream& out, Vector3& vector) {
    out.write(reinterpret_cast<char*>(&vector.x), sizeof(vector.x));
    out.write(reinterpret_cast<char*>(&vector.y), sizeof(vector.y));
    out.write(reinterpret_cast<char*>(&vector.z), sizeof(vector.z));
}


// 将 FQuat 数据写入文件
inline void writeFQuat(std::ofstream& out, DirectX::XMFLOAT4& quat) {
    out.write(reinterpret_cast<char*>(&quat.x), sizeof(quat.x));
    out.write(reinterpret_cast<char*>(&quat.y), sizeof(quat.y));
    out.write(reinterpret_cast<char*>(&quat.z), sizeof(quat.z));
    out.write(reinterpret_cast<char*>(&quat.w), sizeof(quat.w));
}

struct Ver {
    Vector3 MmodelLocation; // 模型世界坐标
    Vector3 Verticesscale; // 缩放比例
    DirectX::XMFLOAT4 VerticesQ; // 模型朝向
    std::vector<Vector3> Vertices; // 模型网格坐标
    int VerticesCount; // 模型网格数量
    std::vector<uint16_t> Indices; // 模型连线三角
    int IndicesCount; // 模型三角数量

    // 将 Ver 数据写入文件
    void write(std::ofstream& out) {
        writeFVector(out, MmodelLocation);
        writeFVector(out, Verticesscale);
        writeFQuat(out, VerticesQ);

        size_t vecSize = Vertices.size();
        out.write(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
        for (auto& vertex : Vertices) {
            writeFVector(out, vertex);
        }
        out.write(reinterpret_cast<char*>(&VerticesCount), sizeof(VerticesCount));

        size_t indicesSize = Indices.size();
        out.write(reinterpret_cast<char*>(&indicesSize), sizeof(indicesSize));
        for (auto& index : Indices) {
            out.write(reinterpret_cast<char*>(&index), sizeof(index));
        }
        out.write(reinterpret_cast<char*>(&IndicesCount), sizeof(IndicesCount));
    }

    // 从文件中读取 Ver 数据
    void read(std::ifstream& in) {
        readFVector(in, MmodelLocation);
        readFVector(in, Verticesscale);
        readFQuat(in, VerticesQ);

        size_t vecSize;
        in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));
        Vertices.resize(vecSize);
        for (auto& vertex : Vertices) {
            readFVector(in, vertex);
        }

        in.read(reinterpret_cast<char*>(&VerticesCount), sizeof(VerticesCount));
        size_t indicesSize;
        in.read(reinterpret_cast<char*>(&indicesSize), sizeof(indicesSize));
        Indices.resize(indicesSize);
        for (auto& index : Indices) {
            in.read(reinterpret_cast<char*>(&index), sizeof(index));
        }
        in.read(reinterpret_cast<char*>(&IndicesCount), sizeof(IndicesCount));
    }
};