#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <sstream>

// 原子化覆盖写入文本文件
static bool atomic_write_text(const std::filesystem::path& path, const std::string& text) {
    try {
        std::filesystem::path temp_path = path;
        temp_path += ".tmp";

        std::ofstream ofs(temp_path, std::ios::out | std::ios::trunc);
        if (!ofs) {
            std::cerr << "[Error] Cannot open temp file for writing: " << temp_path << std::endl;
            return false;
        }
        ofs << text;
        ofs.close(); // 确保写入完成

        if (!ofs) { // 检查写入错误
            std::cerr << "[Error] Failed to write to temp file: " << temp_path << std::endl;
            std::filesystem::remove(temp_path); // 清理临时文件
            return false;
        }

        std::filesystem::rename(temp_path, path); // 原子性替换
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[Error] Filesystem error during atomic write: " << e.what() << std::endl;
        // 尝试非原子写入作为后备
        try {
            std::ofstream ofs_fallback(path, std::ios::out | std::ios::trunc);
            if (ofs_fallback) {
                ofs_fallback << text;
                ofs_fallback.close();
                return ofs_fallback.good();
            }
        }
        catch (...) {} // 忽略后备写入的异常
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "[Error] Exception during atomic write: " << e.what() << std::endl;
        return false;
    }
}

// 获取临时文件路径
std::filesystem::path get_temp_lua_path() {
    return std::filesystem::temp_directory_path() / "configc.lua";
}