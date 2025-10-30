//////////////////// common_packets.h ////////////////////

#pragma once
#include <DirectXMath.h>
#include <cstdint>


/**
 * @brief 定义武器类别
 * * 用于索引压枪设置中的压枪设置
 */
enum class WeaponType : int {
    Rifle = 0,    // 步枪
    SMG,          // 冲锋枪
    Shotgun,      // 霰弹枪
    LMG,          // 轻机枪
    DMR,          // 射手步枪
    Sniper,       // 狙击步枪
    Pistol,       // 手枪
    Unknown,      // 未知或其他
    MaxTypes      // 用于确定数组大小 (总共 9 种)
};

struct RecoilSetting {
    float recoil_x{ 0.0f };
    float recoil_y{ 0.0f };
    float recoil_delay_ms{ 0.0f };
    float recoil_x_2{ 0.0f };
    float recoil_y_2{ 0.0f };
    float recoil_multiplier{ 1.0f }; // <-- 新增的压枪倍率 (默认为1.0, 即无缩放)
};

constexpr size_t MAX_AIMBOT_BONES = 68;
// 骨骼ID枚举
enum class BoneID : uint32_t
{
    root = 0,
    Hips = 1,
    Spine = 2,
    Spine1 = 3,
    Spine2 = 4,
    RightShoulder = 5,
    RightArm = 6,
    RightForeArm = 7,
    RightForeArmTwist = 8,
    RightForeArmTwist_1 = 9,
    RightHand = 10,
    RightHandThumb1 = 11,
    RightHandThumb2 = 12,
    RightHandThumb3 = 13,
    Weapon_RightHand_Joint = 14,
    RightInHandRing = 15,
    RightHandRing1 = 16,
    RightHandRing2 = 17,
    RightHandRing3 = 18,
    RightInHandPinky = 19,
    RightHandPinky1 = 20,
    RightHandPinky2 = 21,
    RightHandPinky3 = 22,
    RightHandMiddle1 = 23,
    RightHandMiddle2 = 24,
    RightHandMiddle3 = 25,
    RightHandIndex1 = 26,
    RightHandIndex2 = 27,
    RightHandIndex3 = 28,
    RightArmTwist = 29,
    Neck = 30,
    Head = 31,
    Head_Joint = 32,
    LeftShoulder = 33,
    LeftArm = 34,
    LeftForeArm = 35,
    LeftForeArmTwist = 36,
    LeftForeArmTwist_1 = 37,
    LeftHand = 38,
    LeftHandThumb1 = 39,
    LeftHandThumb2 = 40,
    LeftHandThumb3 = 41,
    Weapon_LeftHand_Joint = 42,
    LeftInHandRing = 43,
    LeftHandRing1 = 44,
    LeftHandRing2 = 45,
    LeftHandRing3 = 46,
    LeftInHandPinky = 47,
    LeftHandPinky1 = 48,
    LeftHandPinky2 = 49,
    LeftHandPinky3 = 50,
    LeftHandMiddle1 = 51,
    LeftHandMiddle2 = 52,
    LeftHandMiddle3 = 53,
    LeftHandIndex1 = 54,
    LeftHandIndex2 = 55,
    LeftHandIndex3 = 56,
    LeftArmTwist = 57,
    LeftUpLeg = 58,
    LeftLeg = 59,
    LeftFoot = 60,
    LeftToeBase = 61,
    RightUpLeg = 62,
    RightLeg = 63,
    RightFoot = 64,
    RightToeBase = 65,
    b_IK_Hand_Root = 66,
    b_IK_Gun = 67,
    b_IK_Hand_R = 68,
    b_IK_Hand_L = 69,
    Camera_Root = 70,
    Camera_Joint = 71,
    b_IK_Foot_Root = 72,
    b_IK_Foot_L = 73,
    b_IK_Foot_R = 74,
    VB_RightArm_RightForeArm = 75,
    VB_LeftArm_LeftForeArm = 76,
    VB_b_IK_Foot_L_LeftToeBase = 77,
    VB_b_IK_Foot_R_RightToeBase = 78,
    VB_Spine_b_IK_Gun = 79
};

// --- 自瞄配置 ---
struct AimbotBoneSetting {
    BoneID bone_id;            // 骨骼ID
    bool enabled;              // 是否启用该骨骼作为目标
};

constexpr int NUM_BONES = 68;

enum class PacketType : uint8_t {
    VIEW_UPDATE,            // 视角和自身信息更新
    ENTITY_CREATE,          // 玩家首次出现时的静态信息
    ENTITY_UPDATE_STATE,    // 实体状态更新 (例如箱子打开)
    ENTITY_DESTROY,         // 实体消失
    ITEM_CREATE,            // 物品创建
    BOX_CREATE,             // 容器创建
    HACKER_PC_CREATE,       // 骇客电脑/密码门创建
    EXTRACTION_CREATE,      // 撤离点创建
    AIMBOT_CONFIG_UPDATE,   // 自瞄配置更新 (从客户端到服务器)
    PLAYER_LOCATION_UPDATE, // 玩家位置更新 (高频)
    PLAYER_SKELETON_UPDATE, // 玩家骨骼和可见性更新 (中频)
    PLAYER_STATE_UPDATE,     // 玩家状态更新 (血量/武器/瞄准) (低频)
    RECOIL_CONFIG_UPDATE // 压枪配置更新 (从客户端到服务器)
};

// 通用包头
struct PacketHeader {
    PacketType type;
};

// 地图信息
struct MapInfoPacket {
    float X, Y, W, H;
    int MapX, MapY;
    int MapId;
};

// 视角更新包
struct ViewUpdatePacket {
    PacketHeader header;       // type = VIEW_UPDATE
    DirectX::XMFLOAT4X4 view_matrix;
    DirectX::XMFLOAT3 my_location;
    unsigned int game_width;
    unsigned int game_height;
    bool is_main_map_open;
    MapInfoPacket map_info;
    int my_team_id;
    uint64_t my_pawn_ptr;
    float my_yaw;
};

#pragma pack(push, 1) // 确保紧密打包
// 玩家位置包 (高频)
struct PlayerLocationPacket {
    PacketHeader header;       // type = PLAYER_LOCATION_UPDATE
    uint64_t actor_id;         // 玩家Actor的唯一ID
    DirectX::XMFLOAT3 location; // 世界坐标
    DirectX::XMFLOAT4 rotation; // 旋转四元数 (通常与位置一起更新)
    float distance;            // 距离本地玩家的距离 (服务器计算)
    uint64_t pawn_ptr;         // 玩家Pawn指针
};
#pragma pack(pop)

#pragma pack(push, 1)
// 玩家骨骼包 (中频)
struct PlayerSkeletonPacket {
    PacketHeader header;           // type = PLAYER_SKELETON_UPDATE
    uint64_t actor_id;             // 玩家Actor的唯一ID
    DirectX::XMFLOAT3 skeleton[NUM_BONES]; // 所有骨骼的世界坐标
    bool bone_visibility[NUM_BONES]; // 每个骨骼相对于本地玩家的可见性
};
#pragma pack(pop)

#pragma pack(push, 1)
// 玩家状态包 (低频)
struct PlayerStatePacket {
    PacketHeader header;        // type = PLAYER_STATE_UPDATE
    uint64_t actor_id;         // 玩家Actor的唯一ID
    float health;              // 当前血量
    char held_weapon[32];      // 当前手持武器名称
    bool is_aiming_at_me;      // 该玩家是否正在瞄准本地玩家
    int   helmet_level;       // 当前头盔等级
    int   armor_level;        // 当前护甲等级
    float helmet_hp;          // 当前头盔耐久度
    float armor_hp;           // 当前护甲耐久度
    float helmet_max_hp;      // 当前头盔最大耐久度
    float armor_max_hp;       // 当前护甲最大耐久度
};
#pragma pack(pop)

// --- 其他实体包 ---

// 玩家创建包 (发送一次)
struct PlayerCreatePacket {
    PacketHeader header;        // type = ENTITY_CREATE
    uint64_t actor_id;
    char name[32];             // 玩家名称
    char ai_check[12];         // "Is AI" 或 "Is Player"
    char ai_type[32];          // 如果是AI，其类型
    char operator_name[16];    // 干员名称
    int team_id;               // 队伍ID
    int helmet_level;          // 头盔等级 (0-6)
    int armor_level;           // 护甲等级 (0-6)
};

// 物品创建包
struct ItemCreatePacket {
    PacketHeader header;        // type = ITEM_CREATE
    uint64_t actor_id;
    char name[48];             // 物品名称
    int price;                 // 价格
    int quality;               // 品质 (0-6)
    DirectX::XMFLOAT3 world_location; // 世界坐标
};

// 容器创建包
struct BoxCreatePacket {
    PacketHeader header;        // type = BOX_CREATE
    uint64_t actor_id;
    char name[32];             // 容器名称 (例如 "玩家盒子", "保险箱")
    bool is_looted;            // 是否已被拾取/打开
    DirectX::XMFLOAT3 world_location; // 世界坐标
};

// 实体状态更新包 (例如容器被打开)
struct EntityStateUpdatePacket {
    PacketHeader header;        // type = ENTITY_UPDATE_STATE
    uint64_t actor_id;
    bool is_opened;            // 新的状态 (通常用于 is_looted)
};

// 实体销毁包
struct EntityDestroyPacket {
    PacketHeader header;        // type = ENTITY_DESTROY
    uint64_t actor_id;         // 要销毁的实体的ID
};

// 骇客电脑/密码门创建包
struct HackerPcCreatePacket {
    PacketHeader header;        // type = HACKER_PC_CREATE
    uint64_t actor_id;
    char name[32];             // "骇客电脑" 或 "密码门"
    bool is_looted;            // 是否已被骇入/打开
    DirectX::XMFLOAT3 world_location; // 世界坐标
    int32_t password;          // 密码
};

// 撤离点创建包
struct ExtractionCreatePacket {
    PacketHeader header;        // type = EXTRACTION_CREATE
    uint64_t actor_id;
    DirectX::XMFLOAT3 world_location; // 世界坐标
    char name[32];             // 名称 (例如 "撤离点")
};

#pragma pack(push, 1)
struct RecoilConfigPacket {
    PacketHeader header; // type = RECOIL_CONFIG_UPDATE
    bool enabled;        // 模拟压枪总开关
    // 包含所有武器类型的设置
    RecoilSetting settings[static_cast<int>(WeaponType::MaxTypes)];
};
#pragma pack(pop)

struct AimbotConfigPacket {
    PacketHeader header;       // type = AIMBOT_CONFIG_UPDATE
    bool  aimbot_enabled;      // 自瞄开关
    int   aimbot_key;          // 自瞄按键
    float aimbot_fov;          // FOV范围 (像素)
    float aimbot_smooth;       // 平滑系数
    float aimbot_max_distance; // 最大距离 (米)
    bool  aimbot_visible_only; // 仅瞄准可见目标
    bool  aimbot_ignore_ai;    // 忽略AI
    bool  aimbot_aim_nearest_on_visible; // 可见后吸附模式开关
    uint32_t num_bones;        // aimbot_bones数组中实际使用的元素数量
    AimbotBoneSetting aimbot_bones[MAX_AIMBOT_BONES]; // 目标骨骼设置数组
};
