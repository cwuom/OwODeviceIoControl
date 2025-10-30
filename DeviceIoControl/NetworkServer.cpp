//////////////////// NetworkServer.cpp ////////////////////

#include "framework.h"
#include "NetworkServer.h"
#include "common_packets.h"
#include "Config.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

NetworkServerUdp::NetworkServerUdp() : serverSocket(INVALID_SOCKET), clientConnected(false), running(false) {}

NetworkServerUdp::~NetworkServerUdp() {
    stop();
}

bool NetworkServerUdp::start(int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Network] WSAStartup failed." << std::endl;
        return false;
    }
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[Network] Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    int rcvBufSize = 1 * 1024 * 1024; // 1MB receive buffer
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBufSize, sizeof(rcvBufSize));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Network] Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }
    std::cout << "[Network] UDP Server started on port " << port << std::endl;
    running = true;
    last_heartbeat_time = std::chrono::steady_clock::now();
    listenThread = std::thread(&NetworkServerUdp::listen_loop, this);
    checkConnectionThread = std::thread(&NetworkServerUdp::check_connection_loop, this);
    return true;
}

void NetworkServerUdp::stop() {
    running = false;
    if (serverSocket != INVALID_SOCKET) {
        shutdown(serverSocket, SD_BOTH);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    if (listenThread.joinable()) listenThread.join();
    if (checkConnectionThread.joinable()) checkConnectionThread.join();
    WSACleanup();
    std::cout << "[Network] UDP Server stopped." << std::endl;
}

void NetworkServerUdp::send_packet(const void* data, size_t size) {
    if (clientConnected && serverSocket != INVALID_SOCKET) {
        std::lock_guard lock(clientMutex); // 保护 clientAddr
        sendto(serverSocket, reinterpret_cast<const char*>(data), (int)size, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), sizeof(clientAddr));
    }
}

bool NetworkServerUdp::is_client_connected() const {
    return clientConnected;
}

void NetworkServerUdp::check_connection_loop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (clientConnected) {
            auto now = std::chrono::steady_clock::now();
            auto last_beat = last_heartbeat_time.load();
            auto timeout = std::chrono::duration_cast<std::chrono::seconds>(now - last_beat);

            if (timeout.count() > 5) {
                std::cout << "[Network] Client timed out. Waiting for reconnection...\n";
                clientConnected = false;
            }
        }
    }
    std::cout << "[Network] Check connection loop stopped." << std::endl;
}

void NetworkServerUdp::listen_loop() {
    // 使用智能指针管理缓冲区，大小设为UDP最大可能大小
    auto buffer = std::make_unique<char[]>(65535);
    sockaddr_in fromAddr{};         // 用于存储发送方地址信息
    int fromLen = sizeof(fromAddr); // 地址信息结构的大小
    DWORD timeoutMs = 500;          // 500ms 接收超时

    // 设置接收超时
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeoutMs, sizeof(timeoutMs)) == SOCKET_ERROR) {
        std::cerr << "[Network] Warning: Failed to set SO_RCVTIMEO: " << WSAGetLastError() << std::endl;
        // 可以选择在这里返回或继续，取决于错误处理策略
    }

    std::cout << "[Network] Listen loop started." << std::endl;

    while (running) { // 使用 running 原子变量控制循环
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "[Network] Listen loop exiting: Socket is invalid." << std::endl;
            break;
        }

        // 接收数据
        int bytes = recvfrom(serverSocket, buffer.get(), 65535, 0, reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);

        if (bytes > 0) { // 成功接收到数据
            // 首先检查是否是通用头部
            if (bytes < sizeof(PacketHeader)) continue; // 数据太短
            const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer.get());

            bool configPacketHandled = false; // 标记是否处理了配置包

            // 检查是否是自瞄配置包
            if (header->type == PacketType::AIMBOT_CONFIG_UPDATE && bytes == sizeof(AimbotConfigPacket)) {
                const auto* config_pkt = reinterpret_cast<const AimbotConfigPacket*>(buffer.get());
                // 更新服务端自瞄配置
                g_cfg.aimbot_enabled = config_pkt->aimbot_enabled;
                g_cfg.aimbot_key = config_pkt->aimbot_key;
                g_cfg.aimbot_fov = config_pkt->aimbot_fov;
                g_cfg.aimbot_smooth = config_pkt->aimbot_smooth;
                g_cfg.aimbot_max_distance = config_pkt->aimbot_max_distance;
                g_cfg.aimbot_visible_only = config_pkt->aimbot_visible_only;
                g_cfg.aimbot_ignore_ai = config_pkt->aimbot_ignore_ai;
                g_cfg.aimbot_aim_nearest_on_visible = config_pkt->aimbot_aim_nearest_on_visible;

                // 更新骨骼列表，需要加锁
                {
                    std::unique_lock lock(g_cfg.aimbot_bones_mutex);
                    g_cfg.aimbot_target_bones.clear();
                    size_t count = std::min((size_t)config_pkt->num_bones, (size_t)MAX_AIMBOT_BONES);
                    if (count > 0) {
                        g_cfg.aimbot_target_bones.assign(config_pkt->aimbot_bones, config_pkt->aimbot_bones + count);
                    }
                } // 锁在此处释放
                //std::cout << "[Network] Aimbot config updated." << std::endl; // Debug
                configPacketHandled = true;
            }
            // 检查是否是压枪配置包
            else if (header->type == PacketType::RECOIL_CONFIG_UPDATE && bytes == sizeof(RecoilConfigPacket)) {
                const auto* recoil_pkt = reinterpret_cast<const RecoilConfigPacket*>(buffer.get());

                // 更新服务端的压枪配置
                g_cfg.simulated_recoil_enabled.store(recoil_pkt->enabled); // 更新总开关

                { // 加锁更新详细设置
                    std::unique_lock lock(g_cfg.recoil_settings_mutex);
                    // 确保大小匹配，然后复制
                    constexpr size_t expectedSize = static_cast<size_t>(WeaponType::MaxTypes);
                    if (g_cfg.weapon_recoil_settings.size() == expectedSize && sizeof(recoil_pkt->settings) == expectedSize * sizeof(RecoilSetting)) {
                        memcpy(g_cfg.weapon_recoil_settings.data(), recoil_pkt->settings, sizeof(recoil_pkt->settings));
                        //std::cout << "[Network] Recoil config updated." << std::endl; // Debug
                    }
                    else {
                        std::cerr << "[Network] Error: Recoil settings size mismatch during update!" << std::endl;
                        std::cerr << "  Expected size: " << expectedSize << ", g_cfg size: " << g_cfg.weapon_recoil_settings.size()
                            << ", packet size: " << sizeof(recoil_pkt->settings) / sizeof(RecoilSetting) << std::endl;
                    }
                } // 锁释放
                configPacketHandled = true;
            }

            // 如果处理了任何一种配置包
            if (configPacketHandled) {
                last_heartbeat_time = std::chrono::steady_clock::now(); // 更新心跳时间
                if (!clientConnected) { // 如果是第一个配置包，建立连接
                    std::lock_guard<std::mutex> lock(clientMutex); // clientAddr 需要锁
                    clientAddr = fromAddr;
                    clientConnected = true;
                    newClientJustConnected = true; // 标记新客户端连接，以便 GameLogic 同步状态
                    char ipstr[INET_ADDRSTRLEN] = { 0 };
                    InetNtopA(AF_INET, &fromAddr.sin_addr, ipstr, INET_ADDRSTRLEN);
                    std::cout << "[Network] Client connected via config packet from " << ipstr << ":" << ntohs(fromAddr.sin_port) << "\n";
                }
                continue; // 处理完配置包，继续下一次循环
            }

            // 如果不是配置包，处理其他短包 (心跳、发现等)
            char* received_str = buffer.get();
            // 安全地添加 null 终止符
            if (bytes < 65535) { received_str[bytes] = '\0'; }
            else { received_str[65534] = '\0'; } // 防止越界

            if (strcmp(received_str, "DISCOVER_DELTA_FORCE_SERVER") == 0) {
                char ipstr[INET_ADDRSTRLEN] = { 0 };
                InetNtopA(AF_INET, &fromAddr.sin_addr, ipstr, INET_ADDRSTRLEN);
                std::cout << "[Network] Discovery from " << ipstr << ". Responding.\n";
                const char* ack = "DELTA_FORCE_SERVER_ACK";
                sendto(serverSocket, ack, (int)strlen(ack), 0, reinterpret_cast<sockaddr*>(&fromAddr), fromLen);
            }
            else if (strcmp(received_str, "heartbeat") == 0) {
                last_heartbeat_time = std::chrono::steady_clock::now();
                if (!clientConnected) { // 允许通过心跳重新建立连接
                    std::lock_guard lock(clientMutex);
                    clientAddr = fromAddr;
                    clientConnected = true;
                    newClientJustConnected = true; // 标记以便同步
                    char ipstr[INET_ADDRSTRLEN] = { 0 };
                    InetNtopA(AF_INET, &fromAddr.sin_addr, ipstr, INET_ADDRSTRLEN);
                    std::cout << "[Network] Client reconnected via heartbeat from " << ipstr << ":" << ntohs(fromAddr.sin_port) << "\n";
                }
            }
            else if (strcmp(received_str, "hello") == 0) { // 初始连接请求
                std::lock_guard<std::mutex> lock(clientMutex);
                clientAddr = fromAddr;
                clientConnected = true;
                last_heartbeat_time = std::chrono::steady_clock::now();
                newClientJustConnected = true; // 标记以便同步
                char ipstr[INET_ADDRSTRLEN] = { 0 };
                InetNtopA(AF_INET, &fromAddr.sin_addr, ipstr, INET_ADDRSTRLEN);
                std::cout << "[Network] Client connected via hello from " << ipstr << ":" << ntohs(fromAddr.sin_port) << "\n";
            }
            // 可以添加 else if 来处理其他短命令或忽略未知命令

        }
        else if (bytes == SOCKET_ERROR) { // recvfrom 出错
            int e = WSAGetLastError();
            // WSAETIMEDOUT 是正常的，因为我们设置了超时
            if (e != WSAETIMEDOUT && running) { // 忽略超时错误，报告其他错误
                std::cerr << "[Network] recvfrom error: " << e << "\n";
                // 短暂休眠避免错误刷屏
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        else { // bytes == 0 或 bytes < 0 (通常不应发生在UDP上，除非套接字关闭)
            if (running) {
                std::cerr << "[Network] recvfrom returned " << bytes << ". Error: " << WSAGetLastError() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    } // end while(running)
    std::cout << "[Network] Listen loop stopped." << std::endl;
}