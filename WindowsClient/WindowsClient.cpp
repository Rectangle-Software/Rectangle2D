#include "raylib.h"
#include <thread>
#include "bindings.h"
#include "RectanglePack.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <random>

namespace fs = std::filesystem;

fs::path getTempDir() {
    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, "TEMP") != 0 || buffer == nullptr) {
        return fs::current_path() / "temp";
    }

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1000, 999999);

    fs::path dir = fs::path(buffer) / ("unpack_" + std::to_string(dist(mt)));
    free(buffer);
    fs::create_directories(dir);
    return dir;
}

int main(int argc, char* argv[]) {

    fs::path tempDir = getTempDir();

    if (argc < 1) return 1;
    fs::path exePath = argv[0];
    std::ifstream exe(exePath, std::ios::binary | std::ios::ate);
    if (!exe) return 1;

    std::streamsize size = exe.tellg();
    exe.seekg(0, std::ios::beg);

    std::vector<char> data(size);
    exe.read(data.data(), size);

    if (size < 4) return 1;
    uint32_t archiveSize = *reinterpret_cast<uint32_t*>(&data[size - 4]);
    if (archiveSize > size - 4) return 1;

    fs::path archiveFile = tempDir / "archive.bin";
    std::ofstream out(archiveFile, std::ios::binary);
    out.write(data.data() + (size - 4 - archiveSize), archiveSize);
    out.close();

    SimpleArchive::unpackFiles(archiveFile, tempDir);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    RegisterBindings(L);

    fs::path luaBytecode = tempDir / "main.luac";
    std::ifstream bytecodeFile(luaBytecode, std::ios::binary | std::ios::ate);
    if (!bytecodeFile) {
        printf("Failed to open Lua bytecode\n");
        std::error_code ec;
        fs::remove_all(tempDir, ec);
        if (ec) {

        }
        lua_close(L);
        return 1;
    }

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string currentPath = lua_tostring(L, -1);
    lua_pop(L, 1);

    std::string newPath = tempDir.string() + "/?.luac;" + tempDir.string() + "/?.lua;" + currentPath;
    lua_pushstring(L, newPath.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    std::streamsize bcSize = bytecodeFile.tellg();
    bytecodeFile.seekg(0, std::ios::beg);
    std::vector<char> bcData(bcSize);
    bytecodeFile.read(bcData.data(), bcSize);

    int result = luaL_loadbuffer(L, bcData.data(), bcData.size(), luaBytecode.string().c_str());
    if (result == LUA_OK) {
        result = lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    if (result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        printf("Lua Error: %s\n", error);
    }

    lua_close(L);

    bytecodeFile.close();

    std::error_code ec;
    fs::remove_all(tempDir, ec);
    if (ec) {
        
    }

    return 0;
}