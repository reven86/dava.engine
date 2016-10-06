#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Scripting/LuaException.h"
#include "Logger/Logger.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
struct ScriptState;

/**
Class for Lua script.
*/
class LuaScript final
{
public:
    /**
    Default constructor.
    */
    LuaScript();

    /**
    Constructor with flag for loading default Lua libs in state.
    */
    LuaScript(bool initDefaultLibs);

    /**
    Default move constructor.
    */
    LuaScript(LuaScript&&);

    /**
    Delete copy constructor. Lua state is non-copyable.
    */
    LuaScript(const LuaScript&) = delete;

    /**
    Destroys the object and free Lua state.
    */
    ~LuaScript();

    /**
    Delete assign operator. Lua state is non-copyable.
    */
    LuaScript& operator=(const LuaScript&) = delete;

    /**
    Load script from string, run it and return number of results.
    Throw LuaException on error.
    Lua stack changes [-0, +nresult, -]
    */
    int32 ExecString(const String& script);

    /**
    Load script from string, run it and return number of results.
    Return -1 on error.
    Lua stack changes [-0, +nresult, -]
    */
    int32 ExecStringSafe(const String& script);

    /**
    Run `fName(...)` function with arguments and return number of
    results. Throw LuaException on error.
    Lua stack changes [-0, +nresult, -]
    */
    template <typename... T>
    int32 ExecFunction(const String& fName, T&&... args);

    /**
    Run `fName(...)` function with arguments and return number of results.
    Return -1 on error.
    Lua stack changes [-0, +nresult, -]
    */
    template <typename... T>
    int32 ExecFunctionSafe(const String& fName, T&&... args);

    /**
    Return value from top of the stack as Any and pop it.
    Throw LuaException or error.
    Lua stack changes [-(top-fromIndex), +0, v].
    */
    Any PopResult();

    /**
    Return value from top of the stack as Any and pop it.
    Return false on error.
    Lua stack changes [-(top-fromIndex), +0, v].
    */
    bool PopResultSafe(Any& any);

    /**
    Set variable to global table with name `vName`
    */
    void SetGlobalVariable(const String& vName, const Any& value);

    //TODO: Create new LuaScript object with lua_State is new thread of current lua_State
    // LuaScript CreateNewThread();

private:
    ScriptState* state = nullptr; //!< Internal script state

    /**
    Find function with name `fName` and put it into stack.
    Lua stack changes [-0, +1, -]
    */
    void BeginCallFunction(const String& fName);

    /**
    Push any value into stack.
    Lua stack changes [-0, +1, -]
    */
    void PushArg(const Any& any);

    /**
    Call Lua function with `nargs` arguments on top of stack, pop they
    and return number of function results in stack.
    Throw LuaException on error.
    Lua stack changes [-(nargs+1), +nresults, v]
    */
    int32 EndCallFunction(int32 nargs);
};

template <typename... T>
inline int32 LuaScript::ExecFunction(const String& fName, T&&... args)
{
    BeginCallFunction(fName);
    const int32 size = sizeof...(args);
    bool vargs[] = { true, (PushArg(Any(std::forward<T>(args))), true)... };
    return EndCallFunction(size);
}

template <typename... T>
inline int32 LuaScript::ExecFunctionSafe(const String& fName, T&&... args)
{
    try
    {
        return ExecFunction(fName, std::forward<T>(args)...);
    }
    catch (const LuaException& e)
    {
        DAVA::Logger::Error(Format("LuaException: %s", e.what()).c_str());
        return -1;
    }
}
}