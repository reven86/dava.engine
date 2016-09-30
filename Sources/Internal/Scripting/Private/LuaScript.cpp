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

static const String mainFuctionName = "main";

LuaScript::LuaScript(bool initDefaultLibs /*= true*/)
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

void LuaScript::RunString(const String& script)
{
    int32 res = luaL_loadstring(state->lua, script.c_str()); // stack +1: script chunk
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
    res = lua_pcall(state->lua, 0, 0, 0); // stack -1: run function/chunk on stack top and pop it
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
}

void LuaScript::RunFile(const FilePath& filepath)
{
    int32 res = luaL_loadfile(state->lua, filepath.GetAbsolutePathname().c_str()); // stack +1: script chunk
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
    res = lua_pcall(state->lua, 0, 0, 0); // stack -1: run function/chunk on stack top and pop it
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }
}

Vector<Any> LuaScript::RunMain(const Vector<Any> args)
{
    return RunFunction(mainFuctionName);
}

Vector<Any> LuaScript::RunFunction(const String& fName, const Vector<Any> args)
{
    int32 beginTop = lua_gettop(state->lua); // store current stack size
    lua_getglobal(state->lua, fName.c_str()); // stack +1: main() function
    int32 count = int32(args.size());
    for (int32 i = 0; i < count; ++i)
    {
        LuaBridge::anyToLua(state->lua, args[i]); // stack +1 (*count): function arg
    }

    int32 res = lua_pcall(state->lua, count, LUA_MULTRET, 0); // stack -(count+1), +nresults: return value or error message
    if (res != 0)
    {
        throw LuaException(res, LuaBridge::PopString(state->lua)); // stack -1
    }

    int32 endTop = lua_gettop(state->lua); // store current stack size
    int32 nresults = endTop - beginTop; // calculate number of function results
    Vector<Any> results;
    try
    {
        for (int32 index = beginTop + 1; index <= endTop; ++index)
        {
            Any result = LuaBridge::luaToAny(state->lua, index);
            results.push_back(std::move(result));
        }
        lua_pop(state->lua, nresults); // stack -nresults
    }
    catch (const LuaException& e)
    {
        lua_pop(state->lua, nresults); // stack -nresults
        throw e;
    }
    return results;
}

bool LuaScript::RunStringSafe(const String& script)
{
    try
    {
        RunString(script);
        return true;
    }
    catch (const LuaException& e)
    {
        Logger::Error(e.what());
        return false;
    }
}

bool LuaScript::RunFileSafe(const FilePath& filepath)
{
    try
    {
        RunFile(filepath);
        return true;
    }
    catch (const LuaException& e)
    {
        Logger::Error(e.what());
        return false;
    }
}

bool LuaScript::RunMainSafe(const Vector<Any> args, Vector<Any>* results)
{
    return RunFunctionSafe(mainFuctionName, args, results);
}

bool LuaScript::RunFunctionSafe(const String& fName, const Vector<Any> args, Vector<Any>* results)
{
    try
    {
        if (results)
        {
            *results = RunFunction(fName, args);
        }
        else
        {
            RunFunction(fName, args);
        }
        return true;
    }
    catch (const LuaException& e)
    {
        Logger::Error(e.what());
        return false;
    }
}
}