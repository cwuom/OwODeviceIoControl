//////////////////// GameLogic.h ////////////////////

#pragma once
#include "framework.h"
#include "GameTypes.h"
#include "client.h"
#include "NetworkServer.h"
#include <filesystem>

#define ZYDIS_STATIC_BUILD
#define ZYCORE_STATIC_BUILD
#include <Zydis/Zydis.h>
#include <Zydis/Decoder.h>
#include "pe_loader.hpp"

namespace fs = std::filesystem;


#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

class GameLogic {
public:
	// 构造函数和析构函数
	explicit GameLogic(MemoryAccessClient& k, NetworkServerUdp& ns);
	~GameLogic();

	// 禁用拷贝
	GameLogic(const GameLogic&) = delete;
	GameLogic& operator=(const GameLogic&) = delete;

	template <typename T>
	T read_mem(uint64_t address, int max_retries = 0, int retry_delay_ms = 0) const {
		T value{};  // 默认值（读取失败时返回）
		if ((address > 0x10000) && (address < 0x000F000000000000)) {
			int retry_count = 0;
			bool read_success = false;

			// 循环重试，直到成功或达到最大重试次数
			while (retry_count < max_retries && !read_success) {
				// 调用底层读取函数，判断是否成功
				read_success = kernel.read_memory(pid, address, &value, sizeof(T));

				if (!read_success) {
					if (retry_count < max_retries - 1) {
						std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
					}
					retry_count++;
				}
			}
		}
		return value;
	}

	// 公共接口：从 main 调用
	void start();
	void stop();
	void clear_decrypt_cache() const;

private:
	// 内部函数
	void viewAndSelfUpdateLoop();
	void aimbotLoop();
	void actorDiscoveryLoop();
	void playerStateUpdateLoop();
	void staticStateUpdateLoop();
	void consoleLoop() const;
	void mapModelLoop();
	bool initialize();
	uintptr_t TranslateRemotePointer(uintptr_t remotePtr) const;

	// 自瞄辅助函数
	bool WorldToScreen(const Vector3& worldPos, DirectX::XMFLOAT2& screenPos) const;
	static std::filesystem::path get_temp_lua_path();
	static bool atomic_write_text(const std::filesystem::path& path, const std::string& text);

	bool read_is_firing_flag() const;

	Vector3 read_enc_location(ULONG64 comp_ptr) const;
	void decrypt(uint32_t* a2, unsigned int a3, short a4) const;
	ULONG64 decrypt_shift(ULONG64 Address) const;

	static GameLogic* s_instance;

	Vector3 get_actor_location(ULONGLONG actor_ptr) const;
	void get_bone_world_positions(ULONGLONG mesh_ptr, DirectX::XMFLOAT3* out_skeleton) const;

	// 内存读取
	template <typename T>
	T read(uint64_t address) const {
		T value{};
		if ((address > 0x10000) && (address < 0x000F000000000000)) {
			kernel.read_memory(pid, address, &value, sizeof(T));
		}
		return value;
	}

	bool read_bytes(uint64_t address, void* buffer, size_t size) const {
		if (!((address > 0x10000) && (address < 0x000F000000000000)) || size == 0) return false;
		return NT_SUCCESS(kernel.read_memory(pid, address, buffer, size));
	}

	// 名称解析
	std::string get_name_fast(int key);
	std::string resolve_name_internal(int key) const;
	static unsigned short calculate_xor_key1(int name_length);

	// 数据提取辅助函数
	std::string get_ai_type(uint64_t actor_ptr) const;
	std::string get_operator_name(uint64_t player_state_ptr) const;
	std::string get_item_display_name(uint64_t item_component_ptr) const;
	std::string get_held_weapon(uint64_t actor_ptr) const;
	std::optional<bool> try_read_open_state_once(uint64_t actor, StaticClass cls) const;
	bool is_in_match_check(uint64_t my_pawn);
	void clear_shift_cache() const;
	std::string get_main_world(ULONG64 Uworld);

	// 静态辅助函数
	static StaticClass classify_by_marking_type(unsigned char t);
	static int categorize_armor_level(unsigned char raw_level);
	static bool is_dead_body_class(const std::string& name);
	static bool valid_location(const Vector3& p);
	static std::string translate_container_name(const std::string& fullName);
	static bool match_container_keywords(const std::string& n);
	static bool container_strict_blacklist(const std::string& n);
	static bool name_blacklist_strict(const std::string& n);
	static std::string get_weapon_name_from_id(long long id);
	static DirectX::XMMATRIX FTransformToMatrix(const FTransform& transform);
	static DirectX::XMMATRIX FBoneTransformToMatrix(const FBoneTransform& transform);
	static void clear_console();
	static bool is_actor_aiming_at(
		const Vector3& actorPos,
		const DirectX::XMFLOAT4& actorRot,
		const Vector3& targetPos,
		float fov_degrees = 180.0f
	);

	uint64_t get_map_object(uint64_t PlayerController);
	float get_my_yaw_from_root(ULONGLONG root_component_ptr) const;
	float get_my_yaw() const; // 用于获取自身Yaw

	// 引用成员
	MemoryAccessClient& kernel;
	NetworkServerUdp& networkServer;

	// 类内初始化所有成员变量，避免竞态条件和未初始化警告
	unsigned long pid = 0;
	uint64_t baseAddress = 0;
	uint64_t cached_player_controller_ptr = 0;
	uint64_t cached_view_matrix_final_ptr = 0;

	std::unordered_map<uint64_t, int> pendingActors;
	mutable std::shared_mutex pendingActorsMutex;

	std::atomic<bool> reloadRequested{ false }; // 手动热重载触发
	bool enableFileHotReload = true;             // 是否开启文件时间戳热重载

	std::atomic<bool> running{ false };
	std::atomic<int> teamIdDelta{ 0 };
	std::atomic<bool> mapLoaded{ false };
	std::string currentMapName = "";

	// 线程成员变量
	std::thread viewUpdateThread, actorDiscoveryThread, playerStateUpdateThread, staticStateUpdateThread, consoleThread, mapModelThread;
	std::thread aimbotThread;

	uint64_t mapObjPtr = 0; // 缓存地图UI对象指针
	uint64_t miniMapObjPtr = 0; // 缓存小地图UI对象指针

	struct SharedData {
		Vector3 my_location{ 0,0,0 };
		int my_team_id = -1;
		uint64_t my_pawn_ptr = 0;
		bool is_in_match = false;
		DirectX::XMFLOAT4X4 view_matrix{}; // 视图矩阵
		float camera_fov = 120.0f;           // FOV
		mutable std::shared_mutex mutex;
	} sharedData;

	std::unordered_map<uint64_t, CachedPlayer> playerCache;
	mutable std::shared_mutex playerCacheMutex;

	std::unordered_map<uint64_t, CachedStatic> staticCache;
	mutable std::shared_mutex staticCacheMutex;

	std::unordered_map<int, std::string> nameCache;
	mutable std::shared_mutex nameCacheMutex;


	typedef void (*DecFunc_t)(ULONG64 EncTable, void* data, DWORD size, WORD handle);
	typedef void (*DecFunc2_t)(void* data, DWORD size, WORD handle, ULONG64 EncTable);

	// 用于模拟执行的成员
	ZydisDecoder decoder;
	ZydisFormatter formatter;
	PEImageLoader loader;
	PVOID veh_handle;
	uintptr_t enc_table_addr;
	DecFunc_t dec_func_ptr;

	// 用于安全调用
	static bool CallDecFuncSafely(DecFunc_t DecFunc, ULONG64 remoteTableAddr, int* position, DWORD size, WORD handler);
	bool CallDecFuncSafely2(DecFunc2_t DecFunc2, uint32_t* a2, unsigned int a3, short a4,
	                        ULONG64 remoteTableAddr) const;

	// 静态的 VEH 异常处理程序
	static LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo);

	// 用于模拟器的静态辅助函数
	static uintptr_t GetRegisterValue(PCONTEXT context, ZydisRegister reg);
	static void SetRegisterValue(PCONTEXT context, ZydisRegister reg, uintptr_t value);
	static bool FixBaseDisplacementMemoryAccess(PCONTEXT context, uintptr_t value);

	// 用于 viewAndSelfUpdateLoop: 缓存自己的 RootComponent 指针
	ULONGLONG cached_my_root_component_ptr = 0;

	// 用于 decrypt: 缓存当前有效的解密函数
	mutable std::shared_mutex decryptCacheMutex; // 互斥锁，保证线程安全
	mutable uintptr_t cachedDecFuncPtr = 0;
	mutable DecFunc_t cachedDecFunc = nullptr;
	mutable ULONG64 cachedEffectiveTableAddr = 0;

	// 用于 decrypt_shift: 缓存加密指针的解密结果 (Key: Address, Value: DecryptedPtr)
	mutable std::shared_mutex shiftCacheMutex; // 互斥锁，保证线程安全
	mutable std::unordered_map<ULONG64, ULONG64> shiftCache;

	ULONGLONG cached_uworld_ptr = 0;
	ULONGLONG cached_gamestate_ptr = 0;
	ULONGLONG cached_my_player_state_ptr = 0;
	int cached_my_team_id = -1;
	int cached_map_id = 0;
	ULONGLONG cached_local_player_ptr = 0; // (lp4)

	bool re_cache_all_pointers(ULONGLONG current_pawn_ptr, ULONGLONG player_controller);
	bool is_in_lobby = false;
};