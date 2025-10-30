//////////////////// Offsets.h ////////////////////

#pragma once

namespace Offsets {
    // 为了兼容性，我们可以定义一个别名，但更好的做法是直接替换
    using ULONG64 = uint64_t;

    // ===== 核心引擎指针 =====
    constexpr ULONG64 Uworld = 0x13A415E8;            // UWorld address
    constexpr ULONG64 Gname = 0x1435E940;             // GName address for object name resolution
    constexpr ULONG64 ViewMatrix_Base = 0x13A39FF0;   // 矩阵2
    constexpr ULONG64 ViewMatrix_Offset1 = 0x20;      // 矩阵一级
    constexpr ULONG64 ViewMatrix_Offset2 = 0x280;     // 矩阵二级
    constexpr ULONG64 OwningGameInstance = 0x190;
    constexpr ULONG64 GameState = 0x0140;  // UWorld -> GameState
	constexpr ULONG64 MapConfig = 0x0660;  // DFMGameState -> MapConfig

    // ===== 世界与 Actor 遍历 =====
    constexpr ULONG64 UlevelS = 0x158;                // UWorld -> PersistentLevel array
    constexpr ULONG64 UlevelSCount = 0x160;           // UWorld -> PersistentLevel array count
    constexpr ULONG64 Actor = 0x98;                   // ULevel -> Actor array
    constexpr ULONG64 Count = 0xA0;                   // ULevel -> Actor array count
    constexpr ULONG64 ObjectID = 0x1C;                // AActor -> ObjectID for GetName
    constexpr ULONG64 WorldComposition = 0x668;       //class UWorld : public UObject = WorldComposition
    constexpr ULONG64 MainWorld = 0x68;               //class UOpenWorldComposition : public UWorldComposition = MainWorld

    // ===== 本地玩家指针链 =====
    constexpr ULONG64 LocalPlayerPtr_Base = 0x13a3d2a8;
    constexpr ULONG64 LocalPlayerPtr_Offset1 = 0xFA0;
    constexpr ULONG64 LocalPlayerPtr_Offset2 = 0xE8;
    constexpr ULONG64 LocalPlayerPtr_Offset3 = 0x560;
    constexpr ULONG64 PlayerController = 0x30;        // ULocalPlayer -> PlayerController
    constexpr ULONG64 LocalPlayers = 0x38;        // ULocalPlayer -> PlayerController
    constexpr ULONG64 LocalPlayerPtr_PawnOffset = 0x3B0; // APlayerController -> AcknowledgedPawn
    constexpr ULONG64 LocalPlayerPtr = 0x144B1450; // APlayerController -> AcknowledgedPawn



    // ===== 通用Actor组件 =====
    constexpr ULONG64 RootComponent = 0x180;          // AActor -> RootComponent (for location)
    constexpr ULONG64 RelativeLocation = 0x168;       // USceneComponent -> RelativeLocation
    constexpr ULONG64 isplayer = 0x48;                // 获取是否人物

    // ===== 玩家状态 =====
    constexpr ULONG64 PlayerState = 0x390;            // APawn -> PlayerState
    constexpr ULONG64 PlayerNamePrivate = 0x478;      // APlayerState -> PlayerNamePrivate (FString)
    constexpr ULONG64 TeamID = 0x66C;                 // AGPPlayerState -> TeamId
    constexpr ULONG64 HeroID = 0x9B8;                 // 获取干员名称
    constexpr ULONG64 bFinishGame = 1224;             // APlayerState -> bFinishGame (used for checking if extracted)
    constexpr ULONG64 BlackBoardOffset = 0x0ED0;      // AGPCharacterBase -> BlackBoard
    constexpr ULONG64 bIsFiringOffset = 0x050A;       // GPBlackboardComponent -> bIsFiring

    // ===== AI 相关 =====
    constexpr ULONG64 AICharacterTag = 0x34F0;        // ADFMCharacter -> CharacterTags (for AI type)
    constexpr ULONG64 bIsABot = 0x39E;                // APlayerState -> bIsABot

    // ===== 生命值组件 =====
    constexpr ULONG64 HealthComp = 0xF68;             // AGPCharacterBase -> HealthComponent
    constexpr ULONG64 HealthSet = 0x248;              // UHealthComponent -> HealthSet
    constexpr ULONG64 DFMHealthDataComponent = 0x38;  // UHealthSet -> DFMHealthDataComponent (actual health value)

    // ===== 装备与护甲 =====
    constexpr ULONG64 CharacterEquipComponent = 0x30b8; // ADFMPlayerCharacter -> CharacterEquipComponent
    constexpr ULONG64 EquipmentInfoArray = 0x1d8;        // UCharacterEquipComponent -> EquipmentInfoArray
    constexpr ULONG64 EquipmentSlot_Helmet = 0x30;       // Offset within EquipmentInfoArray for helmet
    constexpr ULONG64 EquipmentSlot_Armor = 0xf0;       // Offset within EquipmentInfoArray for armor

    // ===== 手持武器 =====
    constexpr ULONG64 CacheCurWeapon = 0x15E8;         // ADFMCharacter -> CacheCurWeapon
    constexpr ULONG64 WeaponID = 0x820;                // AWeapon -> WeaponID

    // ===== 骨骼与网格体 =====
    constexpr ULONG64 Mesh = 0x3D0;                   // ACharacter -> Mesh
    constexpr ULONG64 BoneArray = 0x728;              // USkinnedMeshComponent -> BoneArray
    constexpr ULONG64 ComponentToWorld = 0x210;       // USceneComponent -> ComponentToWorld transform

    // ===== 物品与容器 =====
    constexpr ULONG64 ItemComponent = 0xed0;           // AInventoryPickup -> ItemComponent
    constexpr ULONG64 ItemPrice = 0xd8;                // UInventoryItemComponent -> InitialGuidePrice
    constexpr ULONG64 Quality = 0x68;                  // UInventoryItemComponent -> Quality
    constexpr ULONG64 MarkingItemType = 1818;         // AInteractorBase -> MarkingItemType (used for broad classification)
    constexpr ULONG64 ItemName_1 = 0x18;              // Path to item name
    constexpr ULONG64 ItemName_2 = 0x30;              // Path to item name
    constexpr ULONG64 ItemName_3 = 0x10;              // Path to item name
    constexpr ULONG64 bIsOpened = 0x1d52;               // AInventoryPickup_OpenBox -> bIsOpened
    constexpr ULONG64 bLooted = 0x1F68;                 // AInventoryPickup_DeadBody -> bLooted
    constexpr ULONG64 bDonwload = 0x0E74;                 // AInventoryPickup_DeadBody -> bLooted
    constexpr ULONG64 OwnerPlayerState = 0x1F70;      // AInventoryPickup_DeadBody -> OwnerPlayerState

    // ===== 密码相关 =====
    // class AInteractor_HackPC : public AInteractorBaseProxy
    constexpr ULONG64 Password = 0xdcc;                // Hacker PC Password
    // class AInteractor_CodedLock : public AInteractorBaseProxy
    constexpr ULONG64 PwdSum = 0xe28;                  // Coded Lock (Password Door) Password

    // ===== 地图 UI 偏移量 =====
    constexpr ULONG64 MyHUD = 0x0400;                  // APlayerController -> MyHUD
    constexpr ULONG64 HUDarray = 0x0468;               // BaseHUD -> Controllers
    constexpr ULONG64 ChildControllers = 0x00B0;       // UBatchUIController -> ChildControllers
    constexpr ULONG64 View = 0x0088;                   // UBaseUIController -> View
    constexpr ULONG64 preShouldDrawOffset = 0x092C;    // BigMapHUDView -> preShouldDraw
    constexpr ULONG64 MapinfoOffset = 0x0818 + 0x8;          // BigMapHUDView -> MarkSoundEvent + 0x8

    // ===== 废弃或备用坐标偏移 =====
    constexpr ULONG64 ContainerLocation_Base = 0x1f08;
    constexpr ULONG64 ItemLocation_Base = 0x1840;
    constexpr ULONG64 BoxLocation_Base = 0x118;
    constexpr ULONG64 Location_Offset2 = 0x148;
}