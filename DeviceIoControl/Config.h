//////////////////// Config.h ////////////////////

#pragma once

#include <atomic>
#include <cstdint>
#include <vector>
#include <shared_mutex>
#include <string>
#include <windows.h>

#include "common_packets.h"

// GameLogic.h 的前向声明
class GameLogic;

// ======================== 运行时配置 ========================
struct Config {
    // --- 常规配置 ---
    std::atomic<bool>     debug{ false };
    std::atomic<bool>     enableWLBL{ true };
    std::atomic<bool>     filterOpened{ false };
    std::atomic<uint32_t> consoleMaxBoxes{ 10 };
    std::atomic<float>    maxBoxRangeM{ 400.f };
    std::atomic<float>    minBoxRangeM{ 0.f };

    // --- 自瞄配置 ---
    std::atomic<bool>     aimbot_enabled{ false };
    std::atomic<int>      aimbot_key{ VK_XBUTTON2 };
    std::atomic<float>    aimbot_fov{ 100.0f };
    std::atomic<float>    aimbot_smooth{ 15.0f };
    std::atomic<float>    aimbot_max_distance{ 300.0f };
    std::atomic<bool>     aimbot_visible_only{ true };
    std::atomic<bool>     aimbot_ignore_ai{ false };
    std::atomic<bool>     aimbot_aim_nearest_on_visible{ true };

    // --- 自瞄骨骼配置 ---
    std::vector<AimbotBoneSetting> aimbot_target_bones;
    mutable std::shared_mutex aimbot_bones_mutex;

    // --- 模拟压枪配置 ---
    std::atomic<bool> simulated_recoil_enabled{ true };

    std::vector<RecoilSetting> weapon_recoil_settings;
    mutable std::shared_mutex recoil_settings_mutex;

    Config() {
        weapon_recoil_settings.resize(static_cast<int>(WeaponType::MaxTypes));

        // 格式: { X1, Y1, 延迟ms, X2, Y2 }

        // 步枪 (Rifle)
        weapon_recoil_settings[static_cast<int>(WeaponType::Rifle)] = { 0.1f, 4.8f, 450.0f, 0.2f, 1.8f };
        // 冲锋枪 (SMG)
        weapon_recoil_settings[static_cast<int>(WeaponType::SMG)] = { 0.1f, 3.2f, 300.0f, 0.1f, 0.8f };
        // 霰弹枪 (Shotgun) - 压枪需求小，主要是Y轴
        weapon_recoil_settings[static_cast<int>(WeaponType::Shotgun)] = { 0.0f, 2.5f, 100.0f, 0.0f, 1.0f };
        // 轻机枪 (LMG)
        weapon_recoil_settings[static_cast<int>(WeaponType::LMG)] = { 0.0f, 6.5f, 550.0f, 0.4f, 2.2f };
        // 射手步枪 (DMR) - 模拟点射压枪
        weapon_recoil_settings[static_cast<int>(WeaponType::DMR)] = { 0.0f, 2.8f, 200.0f, 0.1f, 0.5f };
        // 狙击步枪 (Sniper) - 无自动压枪需求
        weapon_recoil_settings[static_cast<int>(WeaponType::Sniper)] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        // 手枪 (Pistol)
        weapon_recoil_settings[static_cast<int>(WeaponType::Pistol)] = { 0.0f, 0.8f, 100.0f, 0.0f, 0.2f };
        // 未知 (Unknown)
        weapon_recoil_settings[static_cast<int>(WeaponType::Unknown)] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    }
};

// 声明一个全局配置变量，实体在 Config.cpp 中定义
extern Config g_cfg;