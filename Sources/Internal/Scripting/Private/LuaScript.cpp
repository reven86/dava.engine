#include "Scripting/LuaScript.h"
#include "Scripting/LuaException.h"
#include "Debug/DVAssert.h"
#include "LuaBridge.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)
#include "MemoryManager/MemoryProfiler.h"

void* lua_profiler_allocator(void* ud, void* ptr, size_t osize, size_t nsize)
{
    if (0 == nsize)
    {
        DAVA::MemoryManager::Instance()->Deallocate(ptr);
        return nullptr;
    }

    void* newPtr = DAVA::MemoryManager::Instance()->Allocate(nsize, DAVA::ALLOC_POOL_LUA);
    if (osize != 0 && newPtr != nullptr)
    {
        size_t n = std::min(osize, nsize);
        Memcpy(newPtr, ptr, n);
        DAVA::MemoryManager::Instance()->Deallocate(ptr);
    }
    return newPtr;
}
#endif

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
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    state->lua = lua_newstate(&lua_profiler_allocator, nullptr);
#else
    state->lua = luaL_newstate();
#endif
    if (initDefaultLibs)
    {
        luaL_openlibs(state->lua); // Load standard libs
    }
    LuaBridge::RegisterDava(state->lua);
    LuaBridge::RegisterAny(state->lua);
    LuaBridge::RegisterAnyFn(state->lua);
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

int32 LuaScript::ExecString(const String& script)
{
    int32 beginTop = lua_gettop(state->lua); // store current stack size
    int32 res = luaL_loadstring(state->lua, script.c_str()); // stack +1: script chunk
    if (res != 0)
    {
        DAVA_THROW(LuaException, res, LuaBridge::PopString(state->lua)); // stack -1
    }
    res = lua_pcall(state->lua, 0, LUA_MULTRET, 0); // stack -1: run function/chunk on stack top and pop it
    if (res != 0)
    {
        DAVA_THROW(LuaException, res, LuaBridge::PopString(state->lua)); // stack -1
    }

    int32 lastIndex = lua_gettop(state->lua); // store current stack size
    return lastIndex - beginTop; // calculate number of function results
}

int32 LuaScript::ExecStringSafe(const String& script)
{
    try
    {
        return ExecString(script);
    }
    catch (const LuaException& e)
    {
        Logger::Error(Format("LuaException: %s", e.what()).c_str());
        return -1;
    }
}

Any LuaScript::GetResult(int32 index, const Type* preferredType /*= nullptr*/) const
{
    return LuaBridge::LuaToAny(state->lua, index, preferredType);
}

bool LuaScript::GetResultSafe(int32 index, Any& any, const Type* preferredType /*= nullptr*/) const
{
    try
    {
        any = LuaBridge::LuaToAny(state->lua, index, preferredType);
        return true;
    }
    catch (const LuaException& e)
    {
        Logger::Error(Format("LuaException: %s", e.what()).c_str());
        return false;
    }
}

void LuaScript::Pop(int32 n)
{
    n = std::max(0, n);
    int32 size = lua_gettop(state->lua);
    n = std::min(n, size);
    lua_pop(state->lua, n);
}

void LuaScript::SetGlobalVariable(const String& vName, const Any& value)
{
    LuaBridge::AnyToLua(state->lua, value); // stack +1
    lua_setglobal(state->lua, vName.c_str()); // stack -1
}

void LuaScript::DumpStack(std::ostream& os) const
{
    LuaBridge::DumpStack(state->lua, os);
}

void LuaScript::DumpStackToLog(Logger::eLogLevel level) const
{
    std::ostringstream os;
    DumpStack(os);
    Logger* logger = Logger::Instance();
    if (logger)
    {
        logger->Log(level, os.str().c_str());
    }
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
        DAVA_THROW(LuaException, res, LuaBridge::PopString(state->lua)); // stack -1
    }
    int32 lastIndex = lua_gettop(state->lua); // store current stack size
    return lastIndex - beginTop; // calculate number of function results
}
}