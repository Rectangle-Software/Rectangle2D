#pragma once
#include "raylib.h"
#include "rlgl.h"

inline void DrawTriangleTexture(Texture2D tex, Vector2 p0, Vector2 p1, Vector2 p2, Rectangle texRect, Color tint)
{
    rlEnableTexture(tex.id);

    rlBegin(RL_TRIANGLES);

    rlColor4ub(tint.r, tint.g, tint.b, tint.a);

    float u0 = texRect.x / tex.width;
    float v0 = texRect.y / tex.height;
    float u1 = (texRect.x + texRect.width) / tex.width;
    float v1 = (texRect.y + texRect.height) / tex.height;

    rlTexCoord2f(u0, v0); rlVertex2f(p0.x, p0.y);
    rlTexCoord2f(u0, v1); rlVertex2f(p1.x, p1.y);
    rlTexCoord2f(u1, v1); rlVertex2f(p2.x, p2.y);

    rlEnd();

    rlDisableTexture();
}

void DrawCircleTextureMasked(Texture2D tex, Vector2 center, float radius, Color tint, int segments = 64)
{
    float angleStep = 360.0f / segments;
    for (int i = 0; i < segments; i++)
    {
        float a0 = i * angleStep;
        float a1 = (i + 1) * angleStep;

        Vector2 p0 = center;
        Vector2 p1 = { center.x + cosf(a0 * PI / 180) * radius,
                       center.y + sinf(a0 * PI / 180) * radius };
        Vector2 p2 = { center.x + cosf(a1 * PI / 180) * radius,
                       center.y + sinf(a1 * PI / 180) * radius };

        Vector2 uv0 = { 0.5f * tex.width, 0.5f * tex.height };
        Vector2 uv1 = { (0.5f + 0.5f * cosf(a0 * PI / 180)) * tex.width,
                        (0.5f + 0.5f * sinf(a0 * PI / 180)) * tex.height };
        Vector2 uv2 = { (0.5f + 0.5f * cosf(a1 * PI / 180)) * tex.width,
                        (0.5f + 0.5f * sinf(a1 * PI / 180)) * tex.height };

        DrawTriangleTexture(tex, p0, p1, p2, { 0, 0, (float)tex.width, (float)tex.height }, tint);
    }
}