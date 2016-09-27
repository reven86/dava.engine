#include "Scripting/Script.h"
#include "Debug/DVAssert.h"
#include "lua_bridge.h"

namespace DAVA
{
struct ScriptState
{
    lua_State* lua = nullptr;
};

static const String mainFuctionName = "main";

Script::Script()
{
    state = new ScriptState;
    state->lua = luaL_newstate();
    luaL_openlibs(state->lua);

    lua::Dava_register(state->lua);
    lua::Any_register(state->lua);
    lua::Reflection_register(state->lua);
}

Script::~Script()
{
    lua_close(state->lua);
    delete state;
}

bool Script::LoadString(const String& script)
{
    int res = luaL_loadstring(state->lua, script.c_str());
    DVASSERT_MSG(res == 0, "Can't load script");
    if (res != 0)
    {
        Logger::Error("LUA Error: %s", lua_tostring(state->lua, -1));
        return false;
    }
    res = lua_pcall(state->lua, 0, LUA_MULTRET, 0);
    DVASSERT_MSG(res == 0, "Can't execute script");
    if (res != 0)
    {
        Logger::Error("LUA Error: %s", lua_tostring(state->lua, -1));
        return false;
    }
    return true;
}

bool Script::LoadFile(const FilePath& filepath)
{
    int res = luaL_loadfile(state->lua, filepath.GetAbsolutePathname().c_str());
    if (res != 0)
    {
        DVASSERT_MSG(false, "Can't load file");
        Logger::Error("Can't load file. Lua script error (%d): %s", res, lua_tostring(state->lua, -1));
        return false;
    }
    res = lua_pcall(state->lua, 0, 0, 0);
    if (res != 0)
    {
        DVASSERT_MSG(false, "Can't execute script");
        Logger::Error("Can't execute script. Lua script error (%d): %s", res, lua_tostring(state->lua, -1));
        return false;
    }
    return true;
}

bool Script::Run(const DAVA::Reflection& context)
{
    lua_getglobal(state->lua, mainFuctionName.c_str());
    lua::pushReflection(state->lua, context);
    DAVA::int32 res = lua_pcall(state->lua, 1, 0, 0);
    if (res != 0)
    {
        DVASSERT_MSG(false, "Can't execute main()");
        Logger::Error("Can't execute main(). Lua script error (%d): %s", res, lua_tostring(state->lua, -1));
        return false;
    }
    return true;
}
}