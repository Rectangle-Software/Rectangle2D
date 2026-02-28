
#ifndef TEXTURE2D_WRAPPER_H
#define TEXTURE2D_WRAPPER_H


#include "raylib.h"
#include <string>

class Texture2DWrapper {
private:
    Texture2D texture;
    bool loaded;

public:
    Texture2DWrapper() : loaded(false) {}

    Texture2DWrapper(const std::string& path) : loaded(false) {
        Load(path);
    }

    ~Texture2DWrapper() {
        Unload();
    }

    bool Load(const std::string& path) {
        if (loaded) Unload();

        texture = LoadTexture(path.c_str());
        loaded = texture.id != 0;
        return loaded;
    }

    void Unload() {
        if (loaded) {
            UnloadTexture(texture);
            loaded = false;
        }
    }

    bool IsLoaded() const { return loaded; }

    const Texture2D& Get() const { return texture; }
};

#endif