//////////////////// NetworkServer.h ////////////////////

#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <winsock2.h>
#include <ws2def.h>

class NetworkServerUdp {
public:
	NetworkServerUdp();
	~NetworkServerUdp();
	NetworkServerUdp(const NetworkServerUdp&) = delete;
	NetworkServerUdp& operator=(const NetworkServerUdp&) = delete;

	bool start(int port);
	void stop();
	void send_packet(const void* data, size_t size);
	bool is_client_connected() const;

	std::atomic<bool> newClientJustConnected{ false };

private:
	void listen_loop();
	void check_connection_loop();

	SOCKET serverSocket;
	sockaddr_in clientAddr{};
	bool clientConnected;
	std::thread listenThread;
	std::thread checkConnectionThread;
	std::atomic<bool> running;
	mutable std::mutex clientMutex;
	std::atomic<std::chrono::steady_clock::time_point> last_heartbeat_time;
};
