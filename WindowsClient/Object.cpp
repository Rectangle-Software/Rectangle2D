#include "object.h"
#include "raylib.h"
#include <cmath>
#include "material.h"
#include "texture2d_wrapper.h"
#include <unordered_map>
#include "RayHelp.h"

Object::Object()
    : position(0, 0),
    size(50, 50),
    rotation(0),
    type(ObjectType::Unknown),
    visible(true),
    material(nullptr)
{
}

Object::Object(ObjectType t)
    : position(0, 0),
    size(50, 50),
    rotation(0),
    type(t),
    visible(true),
    material(nullptr)
{
}

void Object::SetPosition(float x, float y) { position = Vec2(x, y); }
Vec2 Object::GetPosition() const { return position; }

void Object::SetSize(float w, float h) { size = Vec2(w, h); }
Vec2 Object::GetSize() const { return size; }

void Object::SetRotation(float degrees) { rotation = degrees; }
float Object::GetRotation() const { return rotation; }

void Object::SetVisible(bool v) { visible = v; }
bool Object::IsVisible() const { return visible; }

ObjectType Object::GetType() const { return type; }

void Object::SetCustomProperty(const std::string& key, const std::string& value) {
    customProperties[key] = value;
}

std::string Object::GetCustomProperty(const std::string& key, const std::string& defaultValue) const {
    auto it = customProperties.find(key);
    if (it != customProperties.end()) return it->second;
    return defaultValue;
}

bool Object::HasCustomProperty(const std::string& key) const {
    return customProperties.find(key) != customProperties.end();
}

void Object::SetMaterial(GameMaterial* mat) {
    material = mat;
}

GameMaterial* Object::GetMaterial() const {
    return material;
}
void Object::Draw() const {
    if (!visible) return;

    Color colDiffuse = material ? material->GetColor() : RED;

    switch (type) {
    case ObjectType::Rectangle: {
        if (material && material->HasTexture()) {
            Texture2D tex = material->GetTexture()->Get();
            DrawTexturePro(
                tex,
                { 0, 0, (float)tex.width, (float)tex.height },
                { position.x, position.y, size.x, size.y },
                { size.x / 2, size.y / 2 },
                rotation,
                WHITE
            );
        }
        else {
            DrawRectanglePro(
                { position.x, position.y, size.x, size.y },
                { size.x / 2, size.y / 2 },
                rotation,
                colDiffuse
            );
        }
        break;
    }

    case ObjectType::Text: {
        float fontSize = std::stof(GetCustomProperty("fontSize", std::to_string(size.y / 20.0f)));
        DrawTextPro(
            GetFontDefault(),
            GetCustomProperty("text", "").c_str(),
            { position.x, position.y },
            { 0, 0 },
            rotation,
            fontSize,
            1.0f,
            colDiffuse
        );
        break;
    }

    case ObjectType::Sphere: {
        float radius = size.x / 2;
        DrawCircleV({ position.x, position.y }, radius, colDiffuse);
        break;
    }

    case ObjectType::Wedge: {
        float radius = size.x / 2;
        float startAngle = std::stof(GetCustomProperty("startAngle", "0"));
        float endAngle = std::stof(GetCustomProperty("endAngle", "90"));

        const int segments = 32;
        float step = (endAngle - startAngle) / segments;

        for (int i = 0; i < segments; i++) {
            float a0 = startAngle + i * step;
            float a1 = startAngle + (i + 1) * step;

            Vector2 p0 = { position.x, position.y };
            Vector2 p1 = { position.x + cosf(a0 * PI / 180) * radius,
                           position.y + sinf(a0 * PI / 180) * radius };
            Vector2 p2 = { position.x + cosf(a1 * PI / 180) * radius,
                           position.y + sinf(a1 * PI / 180) * radius };

            DrawTriangle(p0, p1, p2, colDiffuse);
        }
        break;
    }

    case ObjectType::Pyramid: {
        Vector2 p0 = { position.x, position.y - size.y / 2 };
        Vector2 p1 = { position.x - size.x / 2, position.y + size.y / 2 };
        Vector2 p2 = { position.x + size.x / 2, position.y + size.y / 2 };

        DrawTriangle(p0, p1, p2, colDiffuse);
        break;
    }

    default:
        break;
    }
}