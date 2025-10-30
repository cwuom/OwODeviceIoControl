//////////////////// main.cpp ////////////////////

#include "framework.h"
#include "NetworkServer.h"
#include "client.h"
#include "GameLogic.h"

// 全局游戏窗口尺寸变量
int g_game_width = 0;
int g_game_height = 0;

std::unique_ptr<GameLogic> game;

// ======================== 分辨率获取辅助 ========================
struct MonitorInfo {
	MONITORINFO m_moniforInfo;
	UINT dpiX{};
	UINT dpiY{};
};

static std::vector<MonitorInfo> monitors;

static BOOL CALLBACK enum_proc(HMONITOR monitor, HDC, LPRECT, LPARAM) {
	MONITORINFO monitorInfo{ sizeof(MONITORINFO) };
	GetMonitorInfo(monitor, &monitorInfo);
	UINT dpix;
	UINT dpiy;
	GetDpiForMonitor(monitor, MDT_RAW_DPI, &dpix, &dpiy);
	monitors.push_back(MonitorInfo{ monitorInfo, dpix, dpiy });
	return TRUE;
}

static void get_monitor_real_resolution(HMONITOR monitor, int* pixelsWidth, int* pixelsHeight) {
	MONITORINFOEX info = { sizeof(MONITORINFOEX) };
	if (!GetMonitorInfo(monitor, &info)) {
		throw std::runtime_error("GetMonitorInfo failed");
	}
	DEVMODE devmode = {};
	devmode.dmSize = sizeof(DEVMODE);
	if (!EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devmode)) {
		throw std::runtime_error("EnumDisplaySettings failed");
	}
	*pixelsWidth = devmode.dmPelsWidth;
	*pixelsHeight = devmode.dmPelsHeight;
}

// ======================== 入口函数 ========================
int main() {
	// 设置控制台为 UTF-8 编码，以正确显示中文
	SetConsoleOutputCP(65001);
	SetConsoleTitleA("OwODeviceIOControl - Server");

	// 获取主显示器的真实分辨率
	try {
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		EnumDisplayMonitors(nullptr, nullptr, enum_proc, 0);
		if (!monitors.empty()) {
			HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
			get_monitor_real_resolution(primaryMonitor, &g_game_width, &g_game_height);
			std::cout << "[Main] Primary monitor real resolution: " << g_game_width << "x" << g_game_height << std::endl;
		}
		else {
			throw std::runtime_error("No monitors found!");
		}
	}
	catch (const std::exception& e) {
		std::cerr << "[Main] Failed to get monitor resolution: " << e.what() << ". Falling back to 1920x1080." << std::endl;
		g_game_width = 1920;
		g_game_height = 1080;
	}

	try {
		// 启动网络服务器
		NetworkServerUdp networkServer;
		if (!networkServer.start(25560)) {
			std::cerr << "Failed to start UDP network server." << std::endl;
			system("pause");
			return 1;
		}

		// 连接到内核驱动
		MemoryAccessClient kernel;
		std::cout << "Attempting to connect to kernel driver..." << std::endl;
		if (!kernel.connect("127.0.0.1", 25598)) {
			std::cerr << "Connection failed. Make sure the driver is loaded and running." << std::endl;
			system("pause");
			return 1;
		}
		std::cout << "Connected to driver successfully!" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));

		// 初始化并启动核心游戏逻辑
		game = std::make_unique<GameLogic>(kernel, networkServer);
		game->start();

		std::cout << "\n[INFO] All threads running. Press Enter to exit." << std::endl;
		std::cin.get();

		// 等待用户输入后，停止并清理资源
		game->stop();

		system("pause");
	}
	catch (const std::exception& e) {
		std::cerr << "A critical error occurred: " << e.what() << std::endl;
		system("pause");
		return 1;
	}

	std::cout << "Program exited gracefully." << std::endl;
	return 0;
}