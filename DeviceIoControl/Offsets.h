#pragma once

// 为了兼容性，我们可以定义一个别名，但更好的做法是直接替换
using ULONG64 = uint64_t;

namespace Offsets
{
	// ---------------------------- 核心全局对象 ------------------------
	constexpr ULONG64 Uworld = 0x14169018;
	constexpr ULONG64 Gname = 0x14A81B00;
	constexpr ULONG64 ViewMatrix_Base = 0x14161A00;
	constexpr ULONG64 ViewMatrix_Offset1 = 0x20;
	constexpr ULONG64 ViewMatrix_Offset2 = 0x280;

	// ---------------------------- UWorld 相关 ------------------------
	constexpr ULONG64 GameState = 0x0140;                       // class UWorld : public UObject
	constexpr ULONG64 OwningGameInstance = 0x0190;              // class UWorld : public UObject
	constexpr ULONG64 UlevelS = 0x158;                          // class UWorld : public UObject = Levels
	constexpr ULONG64 UlevelSCount = 0x160;                     // class UWorld : public UObject
	constexpr ULONG64 WorldComposition = 0x668;                 // class UWorld : public UObject = WorldComposition

	// ---------------------------- 游戏状态与模式 ----------------------
	constexpr ULONG64 DFMGamePlayerMode = 0x0659;               // class ADFMGameState : public AGPGameState
	constexpr ULONG64 MapConfig = 0x0660;                       // DFMGameState -> MapConfig
	constexpr ULONG64 MainWorld = 0x68;                         // class UOpenWorldComposition : public UWorldComposition = MainWorld

	// ---------------------------- 玩家系统 ----------------------------
	// UGameInstance 相关
	constexpr ULONG64 LocalPlayers = 0x0038;                    // class UGameInstance : public UObject

	// UPlayer 相关
	constexpr ULONG64 PlayerController = 0x0030;                // class UPlayer : public UObject

	// APlayerController 相关
	constexpr ULONG64 LocalPlayerPtr_PawnOffset = 0x03F0;       // APlayerController -> AcknowledgedPawn
	constexpr ULONG64 PlayerCameraManager = 0x0408;             // class APlayerController : public AController
	constexpr ULONG64 MyHUD = 0x0400;                           // class APlayerController : public AController

	// APlayerCameraManager 相关
	constexpr ULONG64 CameraCachePrivate = 0x2F2E0;             // class APlayerCameraManager : public AActor

	// HUD 相关
	constexpr ULONG64 HUDarray = 0x0468;                        // BaseHUD -> Controllers
	constexpr ULONG64 ChildControllers = 0x00B0;                // BatchUIController -> ChildControllers
	constexpr ULONG64 View = 0x0088;                            // BaseUIController -> View
	constexpr ULONG64 preShouldDrawOffset = 0x093C;             // BigMapHUDView -> preShouldDraw
	constexpr ULONG64 MapinfoOffset = 0x0818 + 0x8;             // BigMapHUDView -> MarkSoundEvent

	// ---------------------------- 实体(Actor)基础 ----------------------------
	constexpr ULONG64 Actor = 0x98;
	constexpr ULONG64 Count = 0xA0;
	constexpr ULONG64 ObjectID = 0x1C;
	constexpr ULONG64 RootComponent = 0x0180;                    // class AActor : public UObject = RootComponent

	// USceneComponent 相关
	constexpr ULONG64 RelativeLocation = 0x168;                 // class USceneComponent : public UActorComponent
	constexpr ULONG64 RelativeRotation = 0x178;                 // class USceneComponent : public UActorComponent
	constexpr ULONG64 ComponentToWorld = 0x210;                 // USceneComponent -> ComponentToWorld transform

	// ---------------------------- 骨骼与网格体 ----------------------------
	constexpr ULONG64 Mesh = 0x3D0;                             // class ACHARACTER : public APawn
	constexpr ULONG64 BoneArray = 0x728;                        // class USkinnedMeshComponent : public UMeshComponent

	// ---------------------------- 玩家状态与属性 ----------------------------
	// APawn 相关
	constexpr ULONG64 PlayerState = 0x390;                      // class APawn : public AActor ==(APlayerState*)

	// APlayerState 相关
	constexpr ULONG64 PlayerId = 0x0398;                        // class APlayerState : public AInfo
	constexpr ULONG64 bIsABot = 0x039E;                         // class APlayerState : public AInfo
	constexpr ULONG64 PlayerNamePrivate = 0x0478;               // class APlayerState : public AInfo

	// AGPPlayerState 相关
	constexpr ULONG64 bFinishGame = 0x04C8;                     // class AGPPlayerState : public ABasePlayerState
	constexpr ULONG64 TeamID = 0x066C;                          // class AGPPlayerState : public ABasePlayerState
	constexpr ULONG64 bIsDeadBox = 0x0968;                      // class AGPPlayerState : public ABasePlayerState

	// ADFMPlayerState 相关
	constexpr ULONG64 HeroID = 0x09B8;                          // class ADFMPlayerState : public AGPPlayerState

	// ---------------------------- 角色属性 ----------------------------
	// AGPCharacterBase 相关
	constexpr ULONG64 HealthComp = 0x1060;                      // class AGPCharacterBase : public ACharacterBase = HealthComp
	constexpr ULONG64 CacheCurWeapon = 0x16F0;                  // class AGPCharacterBase : public ACharacterBase = CacheCurWeapon
	constexpr ULONG64 BlackBoardOffset = 0x0FC8;                // AGPCharacterBase -> BlackBoard

	// UGPHealthDataComponent 相关
	constexpr ULONG64 HealthSet = 0x248;                        // class UGPHealthDataComponent : public UGPAttributeBaseComponent = HealthSet

	// UGPAttributeSetHealth 相关
	constexpr ULONG64 DFMHealthDataComponent = 0x38;            // class UGPAttributeSetHealth : public UGPAttributeSet = Health(这里SDK里的地址都要+8才是对的)

	// UGPBlackboardComponent 相关
	constexpr ULONG64 bIsFiringOffset = 0x050E;                 // class UGPBlackboardComponent : public UGPNetworkComponent

	// ---------------------------- 装备系统 ----------------------------
	// ADFMCharacter 相关
	constexpr ULONG64 CharacterEquipComponentCache = 0x2188;    // class ADFMCharacter : public AGPCharacter

	// ADFMPlayerCharacter 相关
	constexpr ULONG64 CharacterEquipComponent = 0x3260;         // class ADFMPlayerCharacter : public ADFMCharacter

	// UCharacterEquipComponent 相关
	constexpr ULONG64 EquipmentInfoArray = 0x01D8;              // class UCharacterEquipComponent : public UActorComponent
	constexpr ULONG64 EquipmentSlot_Helmet = 0x30;
	constexpr ULONG64 EquipmentSlot_Armor = 0xf0;

	// ---------------------------- 武器系统 ----------------------------
	// AWeaponBase 相关
	constexpr ULONG64 WeaponID = 0x0828;                        // class AWeaponBase : public AGPWeaponBase = WeaponID

	// ---------------------------- 物品系统 ----------------------------
	// AInventoryPickup 相关
	constexpr ULONG64 PickupItemInfo = 0x1160;                  // class AInventoryPickup : public APickupBase
	constexpr ULONG64 PickupItemSize = 0x0578;

	// FInventoryItemInfo 相关
	constexpr ULONG64 ItemID = 0x0010;                          // struct FInventoryItemInfo : public FFastArraySerializerItem

	// FDFMCommonItemRow 相关
	constexpr ULONG64 FDFMCommonItemRow = PickupItemInfo + PickupItemSize;  // struct FDFMCommonItemRow : public FDescRowBase
	constexpr ULONG64 ItemName_1 = 0x18;
	constexpr ULONG64 ItemName_2 = 0x18;
	constexpr ULONG64 ItemName_3 = 0x38;
	constexpr ULONG64 ItemName_4 = 0x10;
	constexpr ULONG64 Quality = 0x0068;
	constexpr ULONG64 InitialGuidePrice = 0x00D8;

	// ---------------------------- 容器系统 ----------------------------
	// AInteractorBase 相关
	constexpr ULONG64 MarkingItemType = 0x071A;                 // class AInteractorBase : public AGPInteractorBase

	// AInteractor_CodedLock 相关
	constexpr ULONG64 PwdSum = 0x1048;                          // class AInteractor_CodedLock : public AInteractorBaseProxy
	constexpr ULONG64 OpenLockTeamId = 0x1244;                  // class AInteractor_CodedLock : public AInteractorBaseProxy

	// AInteractor_SingleItemContainer 相关
	constexpr ULONG64 boxId_1 = 0x0FE8;                         // class AInteractor_SingleItemContainer : public AInteractorBaseProxy
	constexpr ULONG64 Password = 0x0FEC;                        // class AInteractor_SingleItemContainer : public AInteractorBaseProxy
	constexpr ULONG64 bFirstOpened = 0x1008;                    // class AInteractor_SingleItemContainer : public AInteractorBaseProxy

	// AInteractor_HackPC 相关
	constexpr ULONG64 DownloadExecuteComponent = 0x1008;        // class AInteractor_HackPC : public AInteractorBaseProxy
	constexpr ULONG64 DownloadState = 0x0180;                   // class AInteractor_HackPC : public AInteractorBaseProxy

	// AInventoryPickup_OpenBox 相关
	constexpr ULONG64 boxId_3 = 0x2070;                         // class AInventoryPickup_OpenBox : public AInventoryPickup_Container
	constexpr ULONG64 bIsOpened = 0x207A;                       // class AInventoryPickup_OpenBox : public AInventoryPickup_Container

	// ---------------------------- 尸体盒子 ----------------------------
	// AInventoryPickup_DeadBody 相关
	constexpr ULONG64 OwnerPlayerState = 0x1F70;                // class AInventoryPickup_DeadBody : public AInventoryPickup_Container -> APlayerState*
	constexpr ULONG64 bLooted = 0x2290;                         // class AInventoryPickup_DeadBody : public AInventoryPickup_Container -> bLooted

	// ---------------------------- 人机系统 ----------------------------
	constexpr ULONG64 AICharacterTag = 0x3690;
	// constexpr ULONG64 CharacterTags = 0x3690;                  // class ADFMAICharacter : public ADFMCharacter
}