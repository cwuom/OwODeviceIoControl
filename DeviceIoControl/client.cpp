#include "framework.h"
#include "client.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000DL)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif
#ifndef WSAETIMEDOUT
#define WSAETIMEDOUT 10060L
#endif

MemoryAccessClient::MemoryAccessClient() : clientSocket(INVALID_SOCKET), connected(false) {
	initialize_winsock();
}

MemoryAccessClient::~MemoryAccessClient() {
	disconnect_server();
	cleanup_winsock();
}

void MemoryAccessClient::initialize_winsock() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
	}
}

void MemoryAccessClient::cleanup_winsock() {
	WSACleanup();
}

bool MemoryAccessClient::connect(const std::string& ip, int port) {
	// 创建socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
		return false;
	}

	{
		BOOL nodelay = TRUE;
		setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
	}

	// 设置收发超时，避免阻塞在某一次请求里
	{
		DWORD timeoutMs = 1000;
		setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
		setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	}

	// 设置服务器地址
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

	// 连接服务器
	if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
		return false;
	}

	connected = true;
	std::cout << "Connected to server " << ip << ":" << port << std::endl;
	return true;
}

void MemoryAccessClient::disconnect_server() {
	if (clientSocket != INVALID_SOCKET) {
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	connected = false;
	std::cout << "Disconnected from server" << std::endl;
}

bool MemoryAccessClient::send_packet(const NETWORK_PACKET& packet) {
	if (!connected) {
		std::cerr << "Not connected to server" << std::endl;
		return false;
	}

	int bytesSent = send(clientSocket, (const char*)&packet, sizeof(NETWORK_PACKET), 0);
	if (bytesSent == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAETIMEDOUT) {
			std::cerr << "Send timeout" << std::endl;
		}
		else {
			std::cerr << "Failed to send packet: " << err << std::endl;
		}
		return false;
	}

	return true;
}

bool MemoryAccessClient::receive_packet(NETWORK_PACKET& packet) {
	if (!connected) {
		std::cerr << "Not connected to server" << std::endl;
		return false;
	}

	int totalReceived = 0;
	char* buffer = (char*)&packet;

	while (totalReceived < (int)sizeof(NETWORK_PACKET)) {
		int bytesReceived = recv(clientSocket, buffer + totalReceived,
			sizeof(NETWORK_PACKET) - totalReceived, 0);

		if (bytesReceived == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAETIMEDOUT) {
				std::cerr << "Receive timeout" << std::endl;
			}
			else {
				std::cerr << "Failed to receive packet: " << err << std::endl;
			}
			return false;
		}

		if (bytesReceived == 0) {
			std::cerr << "Connection closed by server" << std::endl;
			return false;
		}

		totalReceived += bytesReceived;
	}

	return true;
}

NTSTATUS MemoryAccessClient::read_memory(ULONG processId, ULONGLONG address, PVOID outBuffer, SIZE_T size) {
	if (!outBuffer || size == 0) return STATUS_INVALID_PARAMETER;

	std::lock_guard<std::mutex> _g(ioMutex);

	SIZE_T offset = 0;
	while (offset < size) {
		NETWORK_PACKET packet = { 0 };
		packet.Command = NET_CMD_READ_MEMORY;
		packet.ProcessId = processId;
		packet.Address = address + offset;

		ULONG chunk = (ULONG)std::min<SIZE_T>(size - offset, sizeof(packet.Data));
		packet.Size = chunk;

		if (!send_packet(packet)) return STATUS_UNSUCCESSFUL;
		if (!receive_packet(packet)) return STATUS_UNSUCCESSFUL;
		if (!NT_SUCCESS(packet.Result)) return packet.Result;

		std::memcpy(static_cast<char*>(outBuffer) + offset, packet.Data, chunk);
		offset += chunk;
	}
	return 0;
}

NTSTATUS MemoryAccessClient::read_memory_batch(ULONG processId, const std::vector<BatchReadRequestItem>& requests, PVOID outBuffer, SIZE_T totalSize) {
	if (requests.empty() || !outBuffer || totalSize == 0) {
		return STATUS_INVALID_PARAMETER;
	}

	// 检查所有请求的总大小是否超过了我们网络包的数据容量
	if (requests.size() * sizeof(BatchReadRequestItem) > sizeof(NETWORK_PACKET::Data)) {
		std::cerr << "Batch read request too large for a single packet." << std::endl;
		return STATUS_INVALID_PARAMETER;
	}

	// 检查请求的总读取大小是否与提供的缓冲区大小匹配
	SIZE_T expectedSize = 0;
	for (const auto& req : requests) {
		expectedSize += req.Size;
	}
	if (expectedSize != totalSize) {
		std::cerr << "Batch read: Mismatch between total request size and outBuffer size." << std::endl;
		return STATUS_INVALID_PARAMETER;
	}

	std::lock_guard<std::mutex> _g(ioMutex);

	NETWORK_PACKET packet;
	packet = { 0 };
	packet.Command = NET_CMD_READ_MEMORY_BATCH;
	packet.ProcessId = processId;
	// packet.Address 在批量模式下未使用
	packet.Size = (ULONG)requests.size(); // Size 字段现在表示请求的数量

	// 将请求列表打包到 Data 缓冲区中
	memcpy(packet.Data, requests.data(), requests.size() * sizeof(BatchReadRequestItem));

	if (!send_packet(packet)) return STATUS_UNSUCCESSFUL;
	if (!receive_packet(packet)) return STATUS_UNSUCCESSFUL;

	if (NT_SUCCESS(packet.Result)) {
		// 如果成功，服务器返回的数据会紧凑地排列在 Data 字段中
		// 我们将其直接拷贝到用户的输出缓冲区
		memcpy(outBuffer, packet.Data, totalSize);
	}

	return packet.Result;
}

NTSTATUS MemoryAccessClient::write_memory(ULONG processId, ULONGLONG address, const PVOID data, SIZE_T size) {
	if (size > 256) {
		std::cerr << "Write size too large. Maximum is 256 bytes." << std::endl;
		return STATUS_INVALID_PARAMETER;
	}

	NETWORK_PACKET packet = { 0 };
	packet.Command = NET_CMD_WRITE_MEMORY;
	packet.ProcessId = processId;
	packet.Address = address;
	packet.Size = (ULONG)size;
	std::memcpy(packet.Data, data, size);

	std::lock_guard<std::mutex> _g(ioMutex);

	if (!send_packet(packet)) {
		return STATUS_UNSUCCESSFUL;
	}

	if (!receive_packet(packet)) {
		return STATUS_UNSUCCESSFUL;
	}

	if (NT_SUCCESS(packet.Result)) {
		// std::cout << "Write memory successful. Result: 0x" << std::hex << packet.Result << std::dec << std::endl;
	}
	else {
		std::cerr << "Write memory failed. Result: 0x" << std::hex << packet.Result << std::dec << std::endl;
	}

	return packet.Result;
}

ULONG MemoryAccessClient::get_process_id_by_name(const std::wstring& processName) {
	NETWORK_PACKET packet = { 0 };
	packet.Command = NET_CMD_GET_PROCESS_ID;

	// 将进程名复制到数据缓冲区
	size_t nameLength = processName.length();
	if (nameLength * sizeof(wchar_t) >= sizeof(packet.Data)) {
		std::cerr << "Process name too long" << std::endl;
		return 0;
	}

	std::memcpy(packet.Data, processName.c_str(), nameLength * sizeof(wchar_t));
	packet.Data[nameLength * sizeof(wchar_t)] = L'\0'; // 确保以 null 结尾

	std::lock_guard<std::mutex> _g(ioMutex);

	std::wcout << L"Sending get process ID request for: " << processName << std::endl;

	if (!send_packet(packet)) {
		return 0;
	}

	if (!receive_packet(packet)) {
		return 0;
	}

	if (NT_SUCCESS(packet.Result)) {
		ULONG processId = *(ULONG*)packet.Data;
		std::cout << "Got process ID: " << processId << std::endl;
		return processId;
	}
	else {
		std::cerr << "Failed to get process ID. Result: 0x" << std::hex << packet.Result << std::dec << std::endl;
		return 0;
	}
}

ULONGLONG MemoryAccessClient::get_module_base_address(ULONG processId, const std::wstring& moduleName) {
	NETWORK_PACKET packet = { 0 };
	packet.Command = NET_CMD_GET_MODULE_BASE;
	packet.ProcessId = processId;

	// 将模块名复制到数据缓冲区
	size_t nameLength = moduleName.length();
	if (nameLength * sizeof(wchar_t) >= sizeof(packet.Data)) {
		std::cerr << "Module name too long" << std::endl;
		return 0;
	}

	std::memcpy(packet.Data, moduleName.c_str(), nameLength * sizeof(wchar_t));
	packet.Data[nameLength * sizeof(wchar_t)] = L'\0'; // 确保以 null 结尾

	std::lock_guard<std::mutex> _g(ioMutex);

	std::wcout << L"Sending get module base request for PID " << processId
		<< L", module: " << moduleName << std::endl;

	if (!send_packet(packet)) {
		return 0;
	}

	if (!receive_packet(packet)) {
		return 0;
	}

	if (NT_SUCCESS(packet.Result)) {
		ULONGLONG baseAddress = *(ULONGLONG*)packet.Data;
		std::cout << "Got module base address: 0x" << std::hex << baseAddress << std::dec << std::endl;
		return baseAddress;
	}
	else {
		std::cerr << "Failed to get module base address. Result: 0x" << std::hex << packet.Result << std::dec << std::endl;
		return 0;
	}
}

void MemoryAccessClient::test_connection() {
	std::cout << "Testing connection..." << std::endl;

	// 测试获取系统进程 ID
	ULONG systemPid = get_process_id_by_name(L"System");
	if (systemPid != 0) {
		std::cout << "System process ID: " << systemPid << std::endl;
	}

	// 测试获取当前进程 ID 和模块基址
	ULONG currentPid = GetCurrentProcessId();
	std::cout << "Current process ID: " << currentPid << std::endl;

	ULONGLONG baseAddress = get_module_base_address(currentPid, L"");
	if (baseAddress != 0) {
		std::cout << "Current process base address: 0x" << std::hex << baseAddress << std::dec << std::endl;
	}

	UCHAR buffer[16] = { 0 };
	NTSTATUS status = read_memory(currentPid, baseAddress, buffer, sizeof(buffer));

	if (NT_SUCCESS(status)) {
		std::cout << "Read memory successful. Data: ";
		for (int i = 0; i < 16; i++) {
			printf("%02X ", buffer[i]);
		}
		std::cout << std::endl;
	}
}

MemoryAccessClient g_MemoryAccessClient;