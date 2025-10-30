#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <winnt.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

class PEImageLoader {
public:
    enum class MapMode {
        DiskFile,   // 使用 PointerToRawData/SizeOfRawData
        MemoryDump  // 使用 VirtualAddress/VirtualSize 作为文件内偏移
    };

    struct LoadedImage {
        void* imageBase{};
        size_t imageSize{};
        std::vector<uint8_t> ntHeaderData;
        uintptr_t preferredBase{};
    };

private:
    LoadedImage m_loadedImage{};

public:
    // mapMode：明确指定文件来源布局
    bool Load(const char* filePath, uintptr_t preferredBase = 0, MapMode mapMode = MapMode::DiskFile);

    const LoadedImage& GetLoadedImage() const { return m_loadedImage; }

    void* GetFunctionByRVA(uintptr_t rva) const;
    void* GetFunctionByName(const char* name) const;

    void Unload();

    uintptr_t RVAToVA(uintptr_t rva) const {
        return reinterpret_cast<uintptr_t>(m_loadedImage.imageBase) + rva;
    }

    ~PEImageLoader() { Unload(); }

private:
    bool MapSections(void* imageBase, const uint8_t* fileBuffer, size_t fileSize, MapMode mode);
    bool ApplyRelocations(uintptr_t newBase, uintptr_t oldBase);
    bool FixImports();
    bool InitializeTLS();
    PIMAGE_SECTION_HEADER FindSectionByRVA(uintptr_t rva) const;
    void SetSectionProtections();
    PIMAGE_NT_HEADERS GetNTHeaders() const;
};
