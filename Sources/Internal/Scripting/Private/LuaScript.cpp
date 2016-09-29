#include "Scripting/LuaScript.h"
#include "Debug/DVAssert.h"
#include "LuaBridge.h"

namespace DAVA
{
struct ScriptState
{
    lua_State* lua = nullptr;
};

static const String mainFuctionName = "main";

LuaScript::LuaScript()
{
    state = new ScriptState;
    state->lua = luaL_newstate();

    int32 top = lua_gettop(state->lua);

    luaL_openlibs(state->lua);

    top = lua_gettop(state->lua);

    Lua::Dava_register(state->lua);

    top = lua_gettop(state->lua);

    //Lua::Any_register(state->lua);

    top = lua_gettop(state->lua);

    Lua::Reflection_register(state->lua);

    top = lua_gettop(state->lua);
}

LuaScript::~LuaScript()
{
    lua_close(state->lua);
    delete state;
}

LuaScript::LoadResult LuaScript::RunString(const String& script)
{
    LoadResult res;

    int32 top = lua_gettop(state->lua);

    res.code = luaL_loadstring(state->lua, script.c_str());

    top = lua_gettop(state->lua);

    if (res.code != 0)
    {
        DVASSERT_MSG(false, "Can't load script");
        res.error = Format("Can't load string. Lua script error (%d): %s", res.code, lua_tostring(state->lua, -1));
        lua_pop(state->lua, -1);
        Logger::Error(res.error.c_str());
        return res;
    }

    top = lua_gettop(state->lua);

    res.code = lua_pcall(state->lua, 0, 0, 0);

    top = lua_gettop(state->lua);

    if (res.code != 0)
    {
        DVASSERT_MSG(false, "Can't execute script");
        res.error = Format("Can't execute script. Lua script error (%d): %s", res.code, lua_tostring(state->lua, -1));
        lua_pop(state->lua, -1);
        Logger::Error(res.error.c_str());
        return res;
    }

    res.loaded = true;
    return res;
}

LuaScript::LoadResult LuaScript::RunFile(const FilePath& filepath)
{
    LoadResult res;
    res.code = luaL_loadfile(state->lua, filepath.GetAbsolutePathname().c_str());
    if (res.code != 0)
    {
        DVASSERT_MSG(false, "Can't load file");
        res.error = Format("Can't load file. Lua script error (%d): %s", res.code, lua_tostring(state->lua, -1));
        lua_pop(state->lua, -1);
        Logger::Error(res.error.c_str());
    }

    res.loaded = true;
    return res;
}

LuaScript::RunResult LuaScript::RunMain(Any args[] /* = {} */)
{
    LuaScript::RunResult res;

    int32 beginTop = lua_gettop(state->lua);

    lua_getglobal(state->lua, mainFuctionName.c_str()); // stack +1: main() function

    int32 top = lua_gettop(state->lua);

    int32 count = sizeof(args) / sizeof(Any);
    for (int32 i = 0; i < count; ++i)
    {
        Lua::anyToLua(state->lua, args[i]); // stack +count: function args
    }

    res.code = lua_pcall(state->lua, count, 1, 0); // stack -(count+1), +1: return value or error message

    top = lua_gettop(state->lua);

    if (res.code != 0)
    {
        DVASSERT_MSG(false, "Can't execute main()");
        res.error = Format("Can't execute main(). Lua script error (%d): %s", res.code, lua_tostring(state->lua, -1));
        lua_pop(state->lua, -1);
        Logger::Error(res.error.c_str());
        return res;
    }

    std::pair<bool, Any> toRes = Lua::luaToAny(state->lua, -1);
    lua_pop(state->lua, -1);
    if (!toRes.first)
    {
        res.code = -1;
        res.error = Format("Can't execute main(). Wrong return type!");
        Logger::Error(res.error.c_str());
        return res;
    }

    top = lua_gettop(state->lua);

    res.value = toRes.second;
    return res;
}
}