//////////////////// client.h ////////////////////

#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <winsock2.h>

// 这些类型定义现在依赖于 framework.h 在 .cpp 文件中被首先包含
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

// 通讯结构
typedef struct _NETWORK_PACKET {
	ULONG Command;
	ULONG ProcessId;
	ULONGLONG Address;
	ULONG Size;
	UCHAR Data[256];
	NTSTATUS Result;
} NETWORK_PACKET, * PNETWORK_PACKET;

// 命令定义
enum
{
	NET_CMD_READ_MEMORY = 1,
	NET_CMD_WRITE_MEMORY = 2,
	NET_CMD_GET_PROCESS_ID = 3,
	NET_CMD_GET_MODULE_BASE = 4,
	NET_CMD_READ_MEMORY_BATCH = 5
};

struct BatchReadRequestItem {
	ULONGLONG Address;
	ULONG Size;
};

class MemoryAccessClient {
	SOCKET clientSocket;
	sockaddr_in serverAddr;
	bool connected;
	std::mutex ioMutex;

public:
	MemoryAccessClient();
	~MemoryAccessClient();

	bool connect(const std::string& ip, int port);
	void disconnect_server();

	NTSTATUS read_memory(ULONG processId, ULONGLONG address, PVOID buffer, SIZE_T size);
	NTSTATUS read_memory_batch(ULONG processId, const std::vector<BatchReadRequestItem>& requests, PVOID outBuffer, SIZE_T totalSize);
	NTSTATUS write_memory(ULONG processId, ULONGLONG address, const PVOID data, SIZE_T size);
	ULONG get_process_id_by_name(const std::wstring& processName);
	ULONGLONG get_module_base_address(ULONG processId, const std::wstring& moduleName);
	void test_connection();

private:
	bool send_packet(const NETWORK_PACKET& packet);
	bool receive_packet(NETWORK_PACKET& packet);
	void initialize_winsock();
	void cleanup_winsock();
};