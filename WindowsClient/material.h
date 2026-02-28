#pragma once
#include "raylib.h"

class Texture2DWrapper;

class GameMaterial {
public:
    GameMaterial()
        : color(WHITE), texture(nullptr) {
    }

    GameMaterial(Color c)
        : color(c), texture(nullptr) {
    }

    void SetColor(Color c) { color = c; }
    Color GetColor() const { return color; }

    void SetTexture(Texture2DWrapper* tex) { texture = tex; }
    void RemoveTexture() { texture = nullptr; }

    Texture2DWrapper* GetTexture() const { return texture; }

    bool HasTexture() const {
        return texture != nullptr;
    }

private:
    Color color;
    Texture2DWrapper* texture;
};