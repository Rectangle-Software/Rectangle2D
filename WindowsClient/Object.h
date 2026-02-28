#pragma once
#include <string>
#include <unordered_map>
#include "vector2.h"

class GameMaterial;

enum class ObjectType {
    Rectangle,
    Text,
    Sphere,
    Wedge,
    Pyramid,
    Unknown
};

class Object {
public:
    Object();
    Object(ObjectType type);

    void SetPosition(float x, float y);
    Vec2 GetPosition() const;

    void SetSize(float w, float h);
    Vec2 GetSize() const;

    void SetRotation(float degrees);
    float GetRotation() const;

    void SetVisible(bool v);
    bool IsVisible() const;

    ObjectType GetType() const;

    void SetMaterial(GameMaterial* mat);
    GameMaterial* GetMaterial() const;

    void SetCustomProperty(const std::string& key, const std::string& value);
    std::string GetCustomProperty(const std::string& key, const std::string& defaultValue = "") const;
    bool HasCustomProperty(const std::string& key) const;


    void Draw() const;

private:
    Vec2 position;
    Vec2 size;
    float rotation;
    ObjectType type;
    bool visible;

    GameMaterial* material = nullptr;
    std::unordered_map<std::string, std::string> customProperties;
};