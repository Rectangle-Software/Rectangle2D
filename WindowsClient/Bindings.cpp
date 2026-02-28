#include "bindings.h"
#include "raylib.h"
#include "object.h"
#include <vector>
#include <thread>
#include <chrono>
#include "material.h"
#include "texture2d_wrapper.h"

extern "C" {
#include "lauxlib.h"
}

std::vector<Object*> g_Objects;


// Raylib

static int Lua_InitWindow(lua_State* L) {
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    const char* title = luaL_checkstring(L, 3);
    InitWindow(w, h, title);
    return 0;
}

static int Lua_WindowShouldClose(lua_State* L) {
    lua_pushboolean(L, WindowShouldClose());
    return 1;
}

static int Lua_SetExitKey(lua_State* L) {
    int key = luaL_checkinteger(L, 1);

    SetExitKey(key);

    return 0;
}

static int Lua_BeginDrawing(lua_State* L) {
    BeginDrawing();
    return 0;
}

static int Lua_EndDrawing(lua_State* L) {
    EndDrawing();
    return 0;
}

int Lua_ClearBackground(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);

    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;

    Color bg = { (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
    ClearBackground(bg);

    return 0;
}



// Uhh deprecated?

static int Lua_DrawText(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    int size = luaL_optinteger(L, 4, 20);
    DrawText(text, (int)x, (int)y, size, BLACK);
    return 0;
}



// Object

static int Lua_Object_SetPosition(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    obj->SetPosition(x, y);
    return 0;
}

static int Lua_Object_SetRotation(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    float rot = (float)luaL_checknumber(L, 2);
    obj->SetRotation(rot);
    return 0;
}

static int Lua_Object_SetSize(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    float w = (float)luaL_checknumber(L, 2);
    float h = (float)luaL_checknumber(L, 3);
    obj->SetSize(w, h);
    return 0;
}

static int Lua_Object_GetPositionX(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushnumber(L, obj->GetPosition().x);
    return 1;
}

static int Lua_Object_GetPositionY(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushnumber(L, obj->GetPosition().y);
    return 1;
}

static int Lua_Object_GetSizeX(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushnumber(L, obj->GetSize().x);
    return 1;
}

static int Lua_Object_GetSizeY(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushnumber(L, obj->GetSize().y);
    return 1;
}

static int Lua_Object_GetRotation(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushnumber(L, obj->GetRotation());
    return 1;
}

static int Lua_Object_Draw(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    obj->Draw();
    return 0;
}

static int Lua_CreateObject(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float w = (float)luaL_checknumber(L, 4);
    float h = (float)luaL_checknumber(L, 5);
    float rot = (float)luaL_checknumber(L, 6);

    ObjectType type = ObjectType::Unknown;
    if (strcmp(name, "Rectangle") == 0) type = ObjectType::Rectangle;
    else if (strcmp(name, "Text") == 0) type = ObjectType::Text;
    else if (strcmp(name, "Sphere") == 0) type = ObjectType::Sphere;
    else if (strcmp(name, "Wedge") == 0) type = ObjectType::Wedge;
    else if (strcmp(name, "Pyramid") == 0) type = ObjectType::Pyramid;

    Object* obj = new Object(type);
    obj->SetPosition(x, y);
    obj->SetSize(w, h);
    obj->SetRotation(rot);

    g_Objects.push_back(obj);

    Object** udata = (Object**)lua_newuserdata(L, sizeof(Object*));
    *udata = obj;

    luaL_getmetatable(L, "ObjectMeta");
    lua_setmetatable(L, -2);

    return 1;
}

static int Lua_Object_SetVisible(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    bool visible = lua_toboolean(L, 2);
    obj->SetVisible(visible);
    return 0;
}

static int Lua_Object_GetVisible(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    lua_pushboolean(L, obj->IsVisible());
    return 1;
}

static int Lua_Object_SetMaterial(lua_State* L) {
    Object* obj =
        *(Object**)luaL_checkudata(L, 1, "ObjectMeta");

    GameMaterial* mat =
        *(GameMaterial**)luaL_checkudata(L, 2, "MaterialMeta");

    obj->SetMaterial(mat);
    return 0;
}

static int Lua_Object_RemoveMaterial(lua_State* L) {
    Object* obj =
        *(Object**)luaL_checkudata(L, 1, "ObjectMeta");

    obj->SetMaterial(nullptr);
    return 0;
}

static int Lua_Object_SetCustomProperty(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    const char* key = luaL_checkstring(L, 2);
    const char* value = luaL_checkstring(L, 3);

    obj->SetCustomProperty(key, value);
    return 0;
}

static int Lua_Object_GetCustomProperty(lua_State* L) {
    Object* obj = *(Object**)luaL_checkudata(L, 1, "ObjectMeta");
    const char* key = luaL_checkstring(L, 2);
    const char* defaultValue = luaL_optstring(L, 3, "");

    std::string value = obj->GetCustomProperty(key, defaultValue);
    lua_pushstring(L, value.c_str());
    return 1;
}

void RegisterObjectMetatable(lua_State* L) {
    luaL_newmetatable(L, "ObjectMeta");

    lua_pushcfunction(L, Lua_Object_SetPosition);
    lua_setfield(L, -2, "SetPosition");

    lua_pushcfunction(L, Lua_Object_SetRotation);
    lua_setfield(L, -2, "SetRotation");

    lua_pushcfunction(L, Lua_Object_SetSize);
    lua_setfield(L, -2, "SetSize");

    lua_pushcfunction(L, Lua_Object_GetPositionX);
    lua_setfield(L, -2, "GetPositionX");

    lua_pushcfunction(L, Lua_Object_GetPositionY);
    lua_setfield(L, -2, "GetPositionY");

    lua_pushcfunction(L, Lua_Object_GetSizeX);
    lua_setfield(L, -2, "GetSizeX");

    lua_pushcfunction(L, Lua_Object_GetSizeY);
    lua_setfield(L, -2, "GetSizeY");

    lua_pushcfunction(L, Lua_Object_GetRotation);
    lua_setfield(L, -2, "GetRotation");

    lua_pushcfunction(L, Lua_Object_SetVisible);
    lua_setfield(L, -2, "SetVisible");

    lua_pushcfunction(L, Lua_Object_GetVisible);
    lua_setfield(L, -2, "GetVisible");

    lua_pushcfunction(L, Lua_Object_SetMaterial);
    lua_setfield(L, -2, "SetMaterial");

    lua_pushcfunction(L, Lua_Object_RemoveMaterial);
    lua_setfield(L, -2, "RemoveMaterial");

    lua_pushcfunction(L, Lua_Object_SetCustomProperty);
    lua_setfield(L, -2, "SetCustomProperty");

    lua_pushcfunction(L, Lua_Object_GetCustomProperty);
    lua_setfield(L, -2, "GetCustomProperty");

    lua_pushcfunction(L, Lua_Object_Draw);
    lua_setfield(L, -2, "Draw");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);
}



// Texture

static int Lua_Texture_New(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    Texture2DWrapper* tex = new Texture2DWrapper(path);

    Texture2DWrapper** udata =
        (Texture2DWrapper**)lua_newuserdata(L, sizeof(Texture2DWrapper*));

    *udata = tex;

    luaL_getmetatable(L, "TextureMeta");
    lua_setmetatable(L, -2);

    return 1;
}

static int Lua_Texture_Unload(lua_State* L) {
    Texture2DWrapper* tex =
        *(Texture2DWrapper**)luaL_checkudata(L, 1, "TextureMeta");

    delete tex;
    return 0;
}

void RegisterTexture(lua_State* L) {
    luaL_newmetatable(L, "TextureMeta");

    lua_pushcfunction(L, Lua_Texture_Unload);
    lua_setfield(L, -2, "Unload");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushcfunction(L, Lua_Texture_New);
    lua_setfield(L, -2, "New");
    lua_setglobal(L, "Texture");
}


// Material

static int Lua_Material_New(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    int a = luaL_optinteger(L, 4, 255);

    GameMaterial* mat = new GameMaterial(Color{ (unsigned char)r,
                                        (unsigned char)g,
                                        (unsigned char)b,
                                        (unsigned char)a });

    GameMaterial** udata =
        (GameMaterial**)lua_newuserdata(L, sizeof(GameMaterial*));

    *udata = mat;

    luaL_getmetatable(L, "MaterialMeta");
    lua_setmetatable(L, -2);

    return 1;
}

static int Lua_Material_SetTexture(lua_State* L) {
    GameMaterial* mat =
        *(GameMaterial**)luaL_checkudata(L, 1, "MaterialMeta");

    Texture2DWrapper* tex =
        *(Texture2DWrapper**)luaL_checkudata(L, 2, "TextureMeta");

    mat->SetTexture(tex);
    return 0;
}

static int Lua_Material_RemoveTexture(lua_State* L) {
    GameMaterial* mat =
        *(GameMaterial**)luaL_checkudata(L, 1, "MaterialMeta");

    mat->SetTexture(nullptr);
    return 0;
}

void RegisterMaterial(lua_State* L) {
    luaL_newmetatable(L, "MaterialMeta");

    lua_pushcfunction(L, Lua_Material_SetTexture);
    lua_setfield(L, -2, "SetTexture");

    lua_pushcfunction(L, Lua_Material_RemoveTexture);
    lua_setfield(L, -2, "RemoveTexture");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushcfunction(L, Lua_Material_New);
    lua_setfield(L, -2, "New");
    lua_setglobal(L, "Material");
}



// Generic

static int Lua_Sleep(lua_State* L) {
    float seconds = (float)luaL_checknumber(L, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
    return 0;
}

static int Lua_GetDeltaTime(lua_State* L) {
    lua_pushnumber(L, GetFrameTime());
    return 1;
}



// Input

static int Lua_Input_IsKeyDown(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyDown(key));
    return 1;
}

static int Lua_Input_IsKeyPressed(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyPressed(key));
    return 1;
}

static int Lua_Input_IsKeyUp(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyUp(key));
    return 1;
}

static int Lua_Input_GetMouseX(lua_State* L) {
    lua_pushnumber(L, GetMouseX());
    return 1;
}

static int Lua_Input_GetMouseY(lua_State* L) {
    lua_pushnumber(L, GetMouseY());
    return 1;
}

static int Lua_Input_SetMouseX(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    SetMousePosition((int)x, GetMouseY());
    return 0;
}

static int Lua_Input_SetMouseY(lua_State* L) {
    float y = (float)luaL_checknumber(L, 1);
    SetMousePosition(GetMouseX(), (int)y);
    return 0;
}

static int Lua_Input_HideCursor(lua_State* L) {
    HideCursor();
    return 0;
}

static int Lua_Input_ShowCursor(lua_State* L) {
    ShowCursor();
    return 0;
}

void RegisterInput(lua_State* L) {
    lua_newtable(L);

    // Keyboard Functions

    lua_pushcfunction(L, Lua_Input_IsKeyDown);
    lua_setfield(L, -2, "IsKeyDown");

    lua_pushcfunction(L, Lua_Input_IsKeyPressed);
    lua_setfield(L, -2, "IsKeyPressed");

    lua_pushcfunction(L, Lua_Input_IsKeyUp);
    lua_setfield(L, -2, "IsKeyUp");

    lua_pushcfunction(L, Lua_Input_GetMouseX);
    lua_setfield(L, -2, "MouseX");

    lua_pushcfunction(L, Lua_Input_GetMouseY);
    lua_setfield(L, -2, "MouseY");

    // Mouse Functions

    lua_pushcfunction(L, Lua_Input_SetMouseX);
    lua_setfield(L, -2, "SetMouseX");

    lua_pushcfunction(L, Lua_Input_SetMouseY);
    lua_setfield(L, -2, "SetMouseY");

    lua_pushcfunction(L, Lua_Input_HideCursor);
    lua_setfield(L, -2, "HideCursor");

    lua_pushcfunction(L, Lua_Input_ShowCursor);
    lua_setfield(L, -2, "ShowCursor");

    lua_setglobal(L, "Input");
}



// Sound

static int Lua_Sound_Load(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Sound* s = new Sound(LoadSound(path));

    Sound** udata = (Sound**)lua_newuserdata(L, sizeof(Sound*));
    *udata = s;

    luaL_getmetatable(L, "SoundMeta");
    lua_setmetatable(L, -2);

    return 1;
}

static int Lua_Sound_Play(lua_State* L) {
    Sound* s = *(Sound**)luaL_checkudata(L, 1, "SoundMeta");
    PlaySound(*s);
    return 0;
}

static int Lua_Sound_Unload(lua_State* L) {
    Sound* s = *(Sound**)luaL_checkudata(L, 1, "SoundMeta");
    UnloadSound(*s);
    delete s;
    return 0;
}

void RegisterSoundMetatable(lua_State* L) {
    luaL_newmetatable(L, "SoundMeta");

    lua_pushcfunction(L, Lua_Sound_Play);
    lua_setfield(L, -2, "Play");

    lua_pushcfunction(L, Lua_Sound_Unload);
    lua_setfield(L, -2, "Unload");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);
}

void RegisterSound(lua_State* L) {
    RegisterSoundMetatable(L);
    lua_register(L, "LoadSound", Lua_Sound_Load);
}


// Register all

void RegisterBindings(lua_State* L) {
    lua_register(L, "InitWindow", Lua_InitWindow);
    lua_register(L, "WindowShouldClose", Lua_WindowShouldClose);
    lua_register(L, "SetExitKey", Lua_SetExitKey);
    lua_register(L, "BeginDrawing", Lua_BeginDrawing);
    lua_register(L, "EndDrawing", Lua_EndDrawing);
    lua_register(L, "ClearBackground", Lua_ClearBackground);
    lua_register(L, "DrawText", Lua_DrawText);

    lua_register(L, "Sleep", Lua_Sleep);
    lua_register(L, "GetDeltaTime", Lua_GetDeltaTime);

    RegisterObjectMetatable(L);
    lua_register(L, "CreateObject", Lua_CreateObject);

    RegisterInput(L);
    RegisterSound(L);
    RegisterTexture(L);
    RegisterMaterial(L);
}