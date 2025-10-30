//////////////////// DMAWrapper.cpp ////////////////////

#include "framework.h"
#include "DMAWrapper.h"
#include "GameLogic.h"

GameLogic* DMAWrapper::gameLogicInstance = nullptr;

template<typename T>
T DMAWrapper::StaticRead(uint64_t address) {
    if (!gameLogicInstance || address == 0) return T{};
    DMAWrapper& dma = gameLogicInstance->GetDMA();
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
    DMAWrapper& dma = gameLogicInstance->GetDMA();
    if (!dma.client || dma.pid == 0) return false;

    return NT_SUCCESS(dma.client->read_memory(dma.pid, address, buffer, size));
}

// Explicit template instantiations for the types used in decrypt.hpp
template BYTE DMAWrapper::StaticRead<BYTE>(uint64_t address);
template USHORT DMAWrapper::StaticRead<USHORT>(uint64_t address);
template ULONG32 DMAWrapper::StaticRead<ULONG32>(uint64_t address);
template ULONG64 DMAWrapper::StaticRead<ULONG64>(uint64_t address);
