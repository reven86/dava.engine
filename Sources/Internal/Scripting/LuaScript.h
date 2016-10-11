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
    Create script with default Lua libraries.
    */
    LuaScript();

    /**
    Create script with or without default Lua libraries.
    */
    LuaScript(bool initDefaultLibs);

    /**
    Move script state to another object.
    */
    LuaScript(LuaScript&&);

    /**
    Deleted copy constructor. Lua state is non-copyable.
    */
    LuaScript(const LuaScript&) = delete;

    /**
    Destroy the object and free Lua state.
    */
    ~LuaScript();

    /**
    Deleted assign operator. Lua state is non-copyable.
    */
    LuaScript& operator=(const LuaScript&) = delete;

    /**
    Load script from string, run it and return number of results in the stack.
    Throw LuaException on error.
    */
    int32 ExecString(const String& script);

    /**
    Load script from string, run it and return number of results in the stack.
    Return -1 on error.
    */
    int32 ExecStringSafe(const String& script);

    /**
    Run `fName(...)` function with arguments and return number of results in 
    the stack. 
    Throw LuaException on error.
    */
    template <typename... T>
    int32 ExecFunction(const String& fName, T&&... args);

    /**
    Run `fName(...)` function with arguments and return number of results in
    the stack.
    Return -1 on error.
    */
    template <typename... T>
    int32 ExecFunctionSafe(const String& fName, T&&... args);

    /**
    Return value from top of the stack as Any with specified type and pop it.
    Throw LuaException or error.
    */
    Any PopResult(const Type* preferredType = nullptr);

    /**
    Return value from top of the stack as Any with specified type and pop it.
    Throw LuaException or error.
    */
    template <typename T>
    Any PopResult();

    /**
    Return value from top of the stack as Any with specified type and pop it.
    Return false on error.
    */
    bool PopResultSafe(Any& any, const Type* preferredType = nullptr);

    /**
    Return value from top of the stack as Any with specified type and pop it.
    Return false on error.
    */
    template <typename T>
    bool PopResultSafe(Any& any);

    /**
    Set variable to global table with name `vName`
    */
    void SetGlobalVariable(const String& vName, const Any& value);

private:
    ScriptState* state = nullptr; //!< Internal script state

    /**
    Find function with name `fName` and put at top of the stack.
    */
    void BeginCallFunction(const String& fName);

    /**
    Put any value at top of the stack.
    */
    void PushArg(const Any& any);

    /**
    Call Lua function with `nargs` arguments on top of stack, pop they
    and return number of function results in stack.
    Throw LuaException on error.
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

template <typename T>
inline Any LuaScript::PopResult()
{
    return PopResult(Type::Instance<T>());
}

template <typename T>
inline bool LuaScript::PopResultSafe(Any& any)
{
    return PopResultSafe(any, Type::Instance<T>());
}
}