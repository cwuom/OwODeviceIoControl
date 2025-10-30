//////////////////// DMAWrapper.h ////////////////////

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif


#pragma once
#include "framework.h"
#include "client.h"

// Forward declare GameLogic to avoid circular dependency
class GameLogic;

class DMAWrapper {
    MemoryAccessClient* client;
    ULONG pid;
    // Add a static pointer to the GameLogic instance
    static GameLogic* gameLogicInstance;

public:
    DMAWrapper(MemoryAccessClient* memClient, ULONG processId)
        : client(memClient), pid(processId) {
    }

    // Add a static method to set the GameLogic instance
    static void SetGameLogicInstance(GameLogic* instance) {
        gameLogicInstance = instance;
    }

    // Static read method using the GameLogic instance's DMAWrapper
    template<typename T>
    static T StaticRead(uint64_t address); // Declaration only

    // Static read bytes method
    static bool StaticReadBytes(uint64_t address, void* buffer, size_t size); // Declaration only



    // 模板方法，支持读取不同类型的数据
    template<typename T>
    T Read(uint64_t address) const {
        if (!client || pid == 0 || address == 0) {
            return T{};
        }
        T value{};
        NTSTATUS status = client->read_memory(pid, address, &value, sizeof(T));
        if (!NT_SUCCESS(status)) {
            return T{};
        }
        return value;
    }

    // 专门用于读取字节数组
    bool ReadBytes(uint64_t address, void* buffer, size_t size) const {
        if (!client || pid == 0 || address == 0 || !buffer || size == 0) {
            return false;
        }
        return NT_SUCCESS(client->read_memory(pid, address, buffer, size));
    }

    // 设置进程ID
    void SetProcessId(ULONG processId) {
        pid = processId;
    }

    // 设置内存访问客户端
    void SetClient(MemoryAccessClient* memClient) {
        client = memClient;
    }

    // --- Add getter for PID needed by static methods ---
    ULONG GetPid() const { return pid; }
};

// Define the static member outside the class definition (e.g., in DMAWrapper.cpp or GameLogic.cpp)
// GameLogic* DMAWrapper::gameLogicInstance = nullptr; // Add this line in a .cpp file


// --- Implement static methods after GameLogic is fully defined ---
// These implementations need to be moved to a .cpp file where GameLogic is included,
// or GameLogic needs to be fully defined before these template implementations.
// For simplicity here, assume GameLogic.h includes DMAWrapper.h and GameLogic is defined.

/* Move these implementations to GameLogic.cpp or a new DMAWrapper.cpp */
/*
#include "GameLogic.h" // Make sure to include GameLogic.h

template<typename T>
T DMAWrapper::StaticRead(uint64_t address) {
    if (!gameLogicInstance || address == 0) return T{};
    DMAWrapper& dma = gameLogicInstance->GetDMA(); // Get the instance's DMAWrapper
    if (!dma.client || dma.pid == 0) return T{};

    T value{};
    NTSTATUS status = dma.client->read_memory(dma.pid, address, &value, sizeof(T));
    if (!NT_SUCCESS(status)) {
        return T{};
    }
    return value;
}

bool DMAWrapper::StaticReadBytes(uint64_t address, void* buffer, size_t size) {
    if (!gameLogicInstance || address == 0 || !buffer || size == 0) return false;
    DMAWrapper& dma = gameLogicInstance->GetDMA(); // Get the instance's DMAWrapper
     if (!dma.client || dma.pid == 0) return false;

    return NT_SUCCESS(dma.client->read_memory(dma.pid, address, buffer, size));
}
*/