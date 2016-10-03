#include "Scripting/LuaScript.h"
#include "Scripting/LuaException.h"
#include "Debug/DVAssert.h"
#include "LuaBridge.h"

namespace DAVA
{
struct ScriptState
{
    lua_State* lua = nullptr;
};

LuaScript::LuaScript()
    : LuaScript(true)
{
}

LuaScript::LuaScript(bool initDefaultLibs)
{
    state = new ScriptState;
    state->lua = luaL_newstate();
    if (initDefaultLibs)
    {
        luaL_openlibs(state->lua); // Load standard libs
    }
    LuaBridge::RegisterDava(state->lua);
    LuaBridge::RegisterAny(state->lua);
    LuaBridge::RegisterReflection(state->lua);
}

LuaScript::LuaScript(LuaScript&& obj)
{
    std::swap(state, obj.state);
}

LuaScript::~LuaScript()
{
    if (state)
    {
        lua_close(state->lua);
        delete state;
    }
}

int32 LuaScript::ExecuteString(const String& script)
{
    int32 beginTop = lua_gettop(state->lua); // store current stack size
    int32 res = luaL_loadstring(state->lua, script.c_str()); // stack +1: script chunk
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
    res = lua_pcall(state->lua, 0, LUA_MULTRET, 0); // stack -1: run function/chunk on stack top and pop it
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }

    int32 lastIndex = lua_gettop(state->lua); // store current stack size
    return lastIndex - beginTop; // calculate number of function results
}

int32 LuaScript::ExecuteStringSafe(const String& script)
{
    try
    {
        return ExecuteString(script);
    }
    catch (const LuaException& e)
    {
        Logger::Error(Format("LuaException: %s", e.what()).c_str());
        return -1;
    }
}

Any LuaScript::PopAny()
{
    Any res = LuaBridge::LuaToAny(state->lua, -1);
    lua_pop(state->lua, 1);
    return res;
}

bool LuaScript::PopAnySafe(Any& any)
{
    bool res = true;
    try
    {
        any = LuaBridge::LuaToAny(state->lua, -1);
    }
    catch (const LuaException& e)
    {
        Logger::Error(Format("LuaException: %s", e.what()).c_str());
        res = false;
    }
    lua_pop(state->lua, 1);
    return true;
}

void LuaScript::SetGlobalVariable(const String& vName, const Any& value)
{
    LuaBridge::AnyToLua(state->lua, value); // stack +1
    lua_setglobal(state->lua, vName.c_str()); // stack -1
}

void LuaScript::BeginCallFunction(const String& fName)
{
    lua_getglobal(state->lua, fName.c_str()); // stack +1: main() function
}

void LuaScript::PushArg(const Any& any)
{
    LuaBridge::AnyToLua(state->lua, any); // stack +1: function arg
}

int32 LuaScript::EndCallFunction(int32 nargs)
{
    int32 beginTop = lua_gettop(state->lua) - nargs - 1; // store current stack size without number of args and function
    DVASSERT_MSG(beginTop >= 0, "Lua stack corrupted!");
    int32 res = lua_pcall(state->lua, nargs, LUA_MULTRET, 0); // stack -(nargs+1), +nresults: return value or error message
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
    int32 lastIndex = lua_gettop(state->lua); // store current stack size
    return lastIndex - beginTop; // calculate number of function results
}
}