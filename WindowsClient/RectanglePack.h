#ifndef SIMPLE_ARCHIVE_H
#define SIMPLE_ARCHIVE_H

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

namespace SimpleArchive {
    namespace fs = std::filesystem;

    inline void packFiles(const fs::path& baseDir, const std::vector<fs::path>& files, const fs::path& archiveName) {
        std::ofstream out(archiveName, std::ios::binary);
        for (auto& file : files) {
            if (!fs::exists(file) || fs::is_directory(file)) continue;
            std::ifstream in(file, std::ios::binary);
            fs::path relPath = fs::relative(file, baseDir);
            std::string relPathStr = relPath.generic_string();
            size_t nameLen = relPathStr.size();
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            in.seekg(0, std::ios::beg);
            out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            out.write(relPathStr.c_str(), nameLen);
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));
            out << in.rdbuf();
        }
    }

    inline void unpackFiles(const fs::path& archiveName, const fs::path& targetDir) {
        std::ifstream in(archiveName, std::ios::binary);
        while (in.peek() != EOF) {
            size_t nameLen;
            in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            std::string relPathStr(nameLen, '\0');
            in.read(relPathStr.data(), nameLen);
            size_t size;
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            fs::path outPath = targetDir / fs::path(relPathStr);
            fs::create_directories(outPath.parent_path());
            std::ofstream out(outPath, std::ios::binary);
            std::vector<char> buffer(size);
            in.read(buffer.data(), size);
            out.write(buffer.data(), size);
        }
    }
}

#endif