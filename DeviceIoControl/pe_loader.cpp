#include "pe_loader.hpp"
#include <fstream>
#include <algorithm>

// 从文件读取 PE 文件到内存
static bool ReadFileToBuffer(const char* filePath, std::vector<uint8_t>& buffer) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[PE Loader] Failed to open file: " << filePath << std::endl;
        return false;
    }

    size_t size = file.tellg();
    buffer.resize(size);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return file.good();
}

// 获取 PE 头
static PIMAGE_NT_HEADERS GetNTHeadersFromBuffer(const uint8_t* buffer, size_t bufferSize) {
    if (bufferSize < sizeof(IMAGE_DOS_HEADER)) {
        return nullptr;
    }

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)buffer;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "[PE Loader] Invalid DOS signature" << std::endl;
        return nullptr;
    }

    uintptr_t ntHeaderOffset = dosHeader->e_lfanew;
    if (ntHeaderOffset + sizeof(IMAGE_NT_HEADERS64) > bufferSize) {
        std::cerr << "[PE Loader] Invalid NT header offset" << std::endl;
        return nullptr;
    }

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(buffer + ntHeaderOffset);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "[PE Loader] Invalid NT signature" << std::endl;
        return nullptr;
    }

    return ntHeaders;
}

bool PEImageLoader::Load(const char* filePath, uintptr_t preferredBase, MapMode mapMode) {
    std::cout << "[PELoader] Loading PE file: " << filePath << std::endl;

    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[PELoader] Failed to open file: " << filePath
            << ", error=" << GetLastError() << std::endl;
        return false;
    }

    LARGE_INTEGER fileSize64{};
    if (!GetFileSizeEx(hFile, &fileSize64)) {
        std::cerr << "[PELoader] Failed to get file size, error=" << GetLastError() << std::endl;
        CloseHandle(hFile);
        return false;
    }
    if (fileSize64.QuadPart == 0 || fileSize64.QuadPart > SIZE_MAX) {
        std::cerr << "[PELoader] Invalid file size" << std::endl;
        CloseHandle(hFile);
        return false;
    }
    size_t fileSize = static_cast<size_t>(fileSize64.QuadPart);
    std::cout << "[PELoader] File size: " << fileSize << " bytes" << std::endl;

    std::vector<uint8_t> fileData(fileSize);
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, fileData.data(), static_cast<DWORD>(fileSize), &bytesRead, nullptr) ||
        bytesRead != fileSize) {
        std::cerr << "[PELoader] Failed to read file, error=" << GetLastError() << std::endl;
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    std::cout << "[PELoader] File read successfully" << std::endl;

    if (fileSize < sizeof(IMAGE_DOS_HEADER)) {
        std::cerr << "[PELoader] File too small for DOS header" << std::endl;
        return false;
    }

    auto* dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(fileData.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "[PELoader] Invalid DOS signature" << std::endl;
        return false;
    }
    std::cout << "[PELoader] DOS header valid, e_lfanew=0x" << std::hex << dosHeader->e_lfanew << std::dec << std::endl;

    if (dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) > fileSize) {
        std::cerr << "[PELoader] Invalid e_lfanew offset" << std::endl;
        return false;
    }

    auto* ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(fileData.data() + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "[PELoader] Invalid NT signature" << std::endl;
        return false;
    }
    std::cout << "[PELoader] NT headers valid, Machine=0x" << std::hex
        << ntHeaders->FileHeader.Machine << std::dec << std::endl;

    if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        std::cerr << "[PELoader] Unsupported or invalid optional header" << std::endl;
        return false;
    }

    size_t imageSize = ntHeaders->OptionalHeader.SizeOfImage;
    uintptr_t imageBase = ntHeaders->OptionalHeader.ImageBase;
    size_t headerSize = ntHeaders->OptionalHeader.SizeOfHeaders;

    std::cout << "[PELoader] ImageBase=0x" << std::hex << imageBase
        << ", ImageSize=0x" << imageSize
        << ", HeaderSize=0x" << headerSize << std::dec << std::endl;

    uintptr_t loadBase = (preferredBase != 0) ? preferredBase : imageBase;
    std::cout << "[PELoader] Attempting to load at 0x" << std::hex << loadBase << std::dec << std::endl;

    auto& relocDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    bool hasRelocations = (relocDir.VirtualAddress != 0 && relocDir.Size != 0);
    if (!hasRelocations) {
        std::cout << "[PELoader] WARNING: No relocation table found! Must load at exact base address 0x"
            << std::hex << loadBase << std::dec << std::endl;
    }

    void* allocatedBase = VirtualAlloc(reinterpret_cast<void*>(loadBase), imageSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocatedBase) {
        std::cerr << "[PELoader] Failed to allocate memory at 0x" << std::hex
            << loadBase << std::dec << ", error=" << GetLastError() << std::endl;
        if (!hasRelocations) {
            std::cerr << "[PELoader] FATAL: Cannot load without relocations!" << std::endl;
            return false;
        }
        allocatedBase = VirtualAlloc(nullptr, imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!allocatedBase) {
            std::cerr << "[PELoader] Failed to allocate memory at any address" << std::endl;
            return false;
        }
        std::cout << "[PELoader] Allocated at different address: 0x" << std::hex
            << allocatedBase << std::dec << std::endl;
    }
    else {
        std::cout << "[PELoader] Successfully allocated at requested address: 0x"
            << std::hex << allocatedBase << std::dec << std::endl;
    }

    if (!hasRelocations && reinterpret_cast<uintptr_t>(allocatedBase) != loadBase) {
        std::cerr << "[PELoader] FATAL: Loaded at wrong address!" << std::endl;
        VirtualFree(allocatedBase, 0, MEM_RELEASE);
        return false;
    }

    m_loadedImage.imageBase = allocatedBase;
    m_loadedImage.imageSize = imageSize;
    m_loadedImage.preferredBase = imageBase;

    size_t ntHeadersSize = dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) +
        (ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    m_loadedImage.ntHeaderData.resize(ntHeadersSize);
    memcpy(m_loadedImage.ntHeaderData.data(), fileData.data(), ntHeadersSize);
    std::cout << "[PELoader] Copied " << ntHeadersSize << " bytes of NT headers" << std::endl;

    if (headerSize > fileSize) {
        std::cerr << "[PELoader] Invalid SizeOfHeaders" << std::endl;
        VirtualFree(allocatedBase, 0, MEM_RELEASE);
        m_loadedImage = {};
        return false;
    }
    memcpy(allocatedBase, fileData.data(), headerSize);
    std::cout << "[PELoader] Copied " << headerSize << " bytes of headers to imageBase" << std::endl;
    if (!MapSections(allocatedBase, fileData.data(), fileSize, mapMode)) {
        std::cerr << "[PELoader] Failed to map sections" << std::endl;
        VirtualFree(allocatedBase, 0, MEM_RELEASE);
        m_loadedImage = {};
        return false;
    }

    uintptr_t actualBase = reinterpret_cast<uintptr_t>(allocatedBase);
    if (actualBase != imageBase && hasRelocations) {
        std::cout << "[PELoader] Applying relocations..." << std::endl;
        if (!ApplyRelocations(actualBase, imageBase)) {
            std::cerr << "[PELoader] Failed to apply relocations" << std::endl;
            VirtualFree(allocatedBase, 0, MEM_RELEASE);
            m_loadedImage = {};
            return false;
        }
    }
    else if (actualBase == imageBase) {
        std::cout << "[PELoader] No relocation needed (loaded at preferred base)" << std::endl;
    }

    std::cout << "[PELoader] Skipping imports and TLS" << std::endl;
    std::cout << "[PELoader] Setting section protections..." << std::endl;
    SetSectionProtections();

    std::cout << "[PELoader] === Load Complete ===" << std::endl;
    std::cout << "[PELoader] Final imageBase: 0x" << std::hex
        << m_loadedImage.imageBase << std::dec << std::endl;

    return true;
}

bool PEImageLoader::MapSections(void* imageBase, const uint8_t* fileBuffer, size_t fileSize, MapMode mode) {
    const auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(fileBuffer);
    const auto* ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(fileBuffer + dosHeader->e_lfanew);

    const IMAGE_SECTION_HEADER* sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);
    WORD numberOfSections = ntHeaders->FileHeader.NumberOfSections;

    std::cout << "[MapSections] Mapping " << numberOfSections << " sections..." << std::endl;
    std::cout << "[MapSections] Mode = " << (mode == MapMode::DiskFile ? "DiskFile" : "MemoryDump") << std::endl;

    for (WORD i = 0; i < numberOfSections; i++) {
        const IMAGE_SECTION_HEADER* s = &sectionHeaders[i];

        char name[9] = { 0 };
        memcpy(name, s->Name, 8);

        std::cout << "[MapSections] Section[" << i << "]: " << name
            << ", VA=0x" << std::hex << s->VirtualAddress
            << ", VS=0x" << s->Misc.VirtualSize
            << ", SR=0x" << s->SizeOfRawData
            << ", PR=0x" << s->PointerToRawData << std::dec << std::endl;

        uint8_t* dest = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(imageBase) + s->VirtualAddress);

        const uint8_t* src = nullptr;
        size_t copySize = 0;

        if (mode == MapMode::DiskFile) {
            size_t cappedRaw = (std::min)(static_cast<size_t>(s->SizeOfRawData),
                static_cast<size_t>(s->Misc.VirtualSize));
            bool ok = s->PointerToRawData != 0 &&
                cappedRaw > 0 &&
                static_cast<size_t>(s->PointerToRawData) + cappedRaw <= fileSize;

            if (ok) {
                src = fileBuffer + s->PointerToRawData;
                copySize = cappedRaw;
                std::cout << "  [MapSections]   Using DISK mapping: src=file+0x"
                    << std::hex << s->PointerToRawData << ", size=0x" << copySize << std::dec << std::endl;
            }
            else {
                // RAW 不可用时，兜底按 VA 复制
                size_t avail = (s->VirtualAddress < fileSize) ? (fileSize - static_cast<size_t>(s->VirtualAddress)) : 0;
                size_t cappedVa = (std::min)(static_cast<size_t>(s->Misc.VirtualSize), avail);
                if (cappedVa > 0) {
                    src = fileBuffer + s->VirtualAddress;
                    copySize = cappedVa;
                    std::cout << "  [MapSections]   Fallback MEMDUMP: src=file+0x"
                        << std::hex << s->VirtualAddress << ", size=0x" << copySize << std::dec << std::endl;
                }
                else {
                    std::cout << "  [MapSections]   No bytes available; will zero." << std::endl;
                }
            }
        }
        else { // MemoryDump 模式
            size_t avail = (s->VirtualAddress < fileSize) ? (fileSize - static_cast<size_t>(s->VirtualAddress)) : 0;
            size_t cappedVa = (std::min)(static_cast<size_t>(s->Misc.VirtualSize), avail);
            if (cappedVa > 0) {
                src = fileBuffer + s->VirtualAddress;
                copySize = cappedVa;
                std::cout << "  [MapSections]   MEMDUMP mapping: src=file+0x"
                    << std::hex << s->VirtualAddress << ", size=0x" << copySize << std::dec << std::endl;
            }
            else {
                std::cout << "  [MapSections]   MEMDUMP no bytes; will zero." << std::endl;
            }
        }

        if (src && copySize > 0) {
            memcpy(dest, src, copySize);
        }

        size_t vsz = static_cast<size_t>(s->Misc.VirtualSize);
        if (vsz > copySize) {
            size_t zeroSize = vsz - copySize;
            memset(dest + copySize, 0, zeroSize);
            if (zeroSize) {
                std::cout << "  [MapSections]   Zeroed 0x" << std::hex << zeroSize << std::dec << " bytes" << std::endl;
            }
        }
    }

    std::cout << "[MapSections] All sections mapped successfully" << std::endl;
    return true;
}

PIMAGE_NT_HEADERS PEImageLoader::GetNTHeaders() const {
    if (!m_loadedImage.imageBase) {
        std::cerr << "[GetNTHeaders] imageBase is null" << std::endl;
        return nullptr;
    }

    // 从实际加载的内存中读取 DOS 头
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(m_loadedImage.imageBase);

    __try {
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            std::cerr << "[GetNTHeaders] Invalid DOS signature in loaded image" << std::endl;
            return nullptr;
        }

        // 从实际加载的内存中读取 NT 头
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uintptr_t>(m_loadedImage.imageBase) + dosHeader->e_lfanew
            );

        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            std::cerr << "[GetNTHeaders] Invalid NT signature in loaded image" << std::endl;
            return nullptr;
        }

        return ntHeaders;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[GetNTHeaders] Exception while reading headers" << std::endl;
        return nullptr;
    }
}

bool PEImageLoader::ApplyRelocations(uintptr_t newBase, uintptr_t oldBase) {
    if (newBase == oldBase) {
        std::cout << "[ApplyRelocations] No relocation needed (same base)\n";
        return true;
    }

    PIMAGE_NT_HEADERS ntHeaders = GetNTHeaders();
    if (!ntHeaders) {
        std::cerr << "[ApplyRelocations] Failed to get NT headers\n";
        return false;
    }

    IMAGE_DATA_DIRECTORY& relocationDir =
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (relocationDir.VirtualAddress == 0 || relocationDir.Size == 0) {
        std::cout << "[ApplyRelocations] No relocation table found\n";
        return true;
    }

    PIMAGE_SECTION_HEADER relocSection = FindSectionByRVA(relocationDir.VirtualAddress);
    if (!relocSection) {
        std::cerr << "[ApplyRelocations] Relocation directory RVA 0x"
            << std::hex << relocationDir.VirtualAddress
            << " is not within any section!\n";
        return false;
    }

    std::cout << "[ApplyRelocations] Relocation table in section: " << relocSection->Name << "\n";

    uintptr_t relocationRVA = relocationDir.VirtualAddress;
    uintptr_t relocationSize = relocationDir.Size;
    intptr_t delta = newBase - oldBase;

    std::cout << "[ApplyRelocations] Delta=0x" << std::hex << delta
        << ", RelocationRVA=0x" << relocationRVA
        << ", Size=0x" << relocationSize << std::dec << "\n";

    PIMAGE_BASE_RELOCATION reloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
        newBase + relocationRVA
        );

    size_t processedSize = 0;
    int relocCount = 0;

    while (processedSize < relocationSize) {
        if (reloc->SizeOfBlock == 0 || reloc->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION)) {
            std::cerr << "[ApplyRelocations] Invalid SizeOfBlock: " << reloc->SizeOfBlock << "\n";
            break;
        }

        if (processedSize + reloc->SizeOfBlock > relocationSize) {
            std::cerr << "[ApplyRelocations] Relocation block exceeds directory size\n";
            break;
        }

        DWORD numEntries = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        WORD* relocEntry = reinterpret_cast<WORD*>(reloc + 1);

        for (DWORD i = 0; i < numEntries; i++) {
            WORD type = (relocEntry[i] >> 12);
            WORD offset = (relocEntry[i] & 0x0FFF);

            if (type == IMAGE_REL_BASED_ABSOLUTE) {
                continue;
            }

            uintptr_t* patchAddr = reinterpret_cast<uintptr_t*>(
                newBase + reloc->VirtualAddress + offset
                );

            if (reinterpret_cast<uintptr_t>(patchAddr) < newBase ||
                reinterpret_cast<uintptr_t>(patchAddr) >= newBase + m_loadedImage.imageSize) {
                std::cerr << "[ApplyRelocations] Patch address 0x" << std::hex << patchAddr
                    << " is out of image bounds!\n";
                continue;
            }

            if (type == IMAGE_REL_BASED_DIR64) {
                *patchAddr += delta;
                relocCount++;
            }
        }

        processedSize += reloc->SizeOfBlock;
        reloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
            reinterpret_cast<uintptr_t>(reloc) + reloc->SizeOfBlock
            );
    }

    std::cout << "[ApplyRelocations] Applied " << relocCount << " relocations\n";
    return true;
}

bool PEImageLoader::FixImports() {
    std::cout << "[FixImports] Skipping import fixing (not required for this use case)" << std::endl;
    return true;
}

bool PEImageLoader::InitializeTLS() {
    PIMAGE_NT_HEADERS ntHeaders = GetNTHeaders();
    if (!ntHeaders) {
        return true;  // 不是致命错误
    }

    IMAGE_DATA_DIRECTORY& tlsDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (tlsDir.Size == 0 || tlsDir.VirtualAddress == 0) {
        std::cout << "[PE Loader] No TLS found" << std::endl;
        return true;
    }

    uintptr_t imageBase = reinterpret_cast<uintptr_t>(m_loadedImage.imageBase);
    PIMAGE_TLS_DIRECTORY tlsDir64 = (PIMAGE_TLS_DIRECTORY)(imageBase + tlsDir.VirtualAddress);

    std::cout << "[PE Loader] TLS found, calling callbacks..." << std::endl;

    // 获取 TLS 回调列表
    PIMAGE_TLS_CALLBACK* callbacks = (PIMAGE_TLS_CALLBACK*)(imageBase + (uintptr_t)tlsDir64->AddressOfCallBacks - m_loadedImage.preferredBase);

    if (callbacks) {
        for (PIMAGE_TLS_CALLBACK* callback = callbacks; *callback != nullptr; callback++) {
            std::cout << "[PE Loader] Calling TLS callback: 0x" << std::hex << (uintptr_t)*callback << std::endl;
            (*callback)((PVOID)imageBase, DLL_THREAD_ATTACH, nullptr);
        }
    }

    return true;
}

PIMAGE_SECTION_HEADER PEImageLoader::FindSectionByRVA(uintptr_t rva) const {
    PIMAGE_NT_HEADERS ntHeaders = GetNTHeaders();
    if (!ntHeaders) return nullptr;

    PIMAGE_SECTION_HEADER sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);

    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER section = &sectionHeaders[i];

        if (rva >= section->VirtualAddress &&
            rva < section->VirtualAddress + section->Misc.VirtualSize) {
            return section;
        }
    }

    return nullptr;
}

void PEImageLoader::SetSectionProtections() {
    PIMAGE_NT_HEADERS ntHeaders = GetNTHeaders();
    if (!ntHeaders) {
        return;
    }

    uintptr_t imageBase = reinterpret_cast<uintptr_t>(m_loadedImage.imageBase);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

    for (USHORT i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        DWORD protect = PAGE_EXECUTE_READWRITE; // 默认保护

        if (section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            if (section[i].Characteristics & IMAGE_SCN_MEM_WRITE) {
                protect = PAGE_EXECUTE_READWRITE;
            }
            else if (section[i].Characteristics & IMAGE_SCN_MEM_READ) {
                protect = PAGE_EXECUTE_READ;
            }
            else {
                protect = PAGE_EXECUTE;
            }
        }
        else if (section[i].Characteristics & IMAGE_SCN_MEM_WRITE) {
            protect = PAGE_READWRITE;
        }
        else if (section[i].Characteristics & IMAGE_SCN_MEM_READ) {
            protect = PAGE_READONLY;
        }

        DWORD oldProtect;
        VirtualProtect(
            (void*)(imageBase + section[i].VirtualAddress),
            section[i].Misc.VirtualSize,
            protect,
            &oldProtect
        );

        std::cout << "[PE Loader] Section " << section[i].Name << " protection set to 0x" << std::hex << protect << std::endl;
    }
}

void* PEImageLoader::GetFunctionByRVA(uintptr_t rva) const {
    if (!m_loadedImage.imageBase) {
        std::cerr << "[GetFunctionByRVA] ERROR: imageBase is NULL!" << std::endl;
        return nullptr;
    }

    uintptr_t imageBase = reinterpret_cast<uintptr_t>(m_loadedImage.imageBase);
    uintptr_t funcAddr = imageBase + rva;

    return reinterpret_cast<void*>(funcAddr);
}

void* PEImageLoader::GetFunctionByName(const char* name) const {
    // TODO: 实现按名称导出查找
    return nullptr;
}

void PEImageLoader::Unload() {
    if (m_loadedImage.imageBase) {
        VirtualFree(m_loadedImage.imageBase, 0, MEM_RELEASE);
        m_loadedImage.imageBase = nullptr;
    }
    m_loadedImage.ntHeaderData.clear();
}