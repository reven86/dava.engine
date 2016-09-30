#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "FileSystem/FilePath.h"
#include "Scripting/LuaException.h"

namespace DAVA
{
struct ScriptState;

/**
 * \brief Class for Lua script.
 */
class LuaScript
{
public:
    /**
     * \brief Default constructor with flag for loading default Lua libs in state
     */
    LuaScript(bool initDefaultLibs = true);

    /**
     * \brief Default move constructor
     */
    LuaScript(LuaScript&&);

    /**
     * \brief Destroys the object and free Lua state
     */
    ~LuaScript();

    /**
     * \brief Load script from string and run it. Throw LuaException on error.
     */
    void RunString(const String& script);

    /**
     * \brief Load script from file and run it. Throw LuaException on error.
     */
    void RunFile(const FilePath& filepath);

    /**
     * \brief Run main(...) function in loaded script with arguments
     *        and return vector of results. Throw LuaException on error.
     */
    Vector<Any> RunMain(const Vector<Any> args = {});

    /**
     * \brief Run fName(...) function in loaded script with arguments
     *        and return vector of results. Throw LuaException on error.
     */
    Vector<Any> RunFunction(const String& fName, const Vector<Any> args = {});

    /**
     * \brief Load script from string and run it. Return false on error.
     */
    bool RunStringSafe(const String& script);

    /**
     * \brief Load script from file and run it. Return false on error.
     */
    bool RunFileSafe(const FilePath& filepath);

    /**
     * \brief Run main(...) function in loaded script with arguments
     *        and return vector of results. Return false on error.
     */
    bool RunMainSafe(const Vector<Any> args = {}, Vector<Any>* results = nullptr);

    /**
     * \brief Run fName(...) function in loaded script with arguments
     *        and return vector of results. Return false on error.
     */
    bool RunFunctionSafe(const String& fName, const Vector<Any> args = {}, Vector<Any>* results = nullptr);

    //TODO:
    //  LuaScript CreateNewThread(); // Create new LuaScript object with lua_State is new thread of currect lua_State

private:
    // Lua state is non-copyable
    LuaScript(const LuaScript&) = delete;
    LuaScript& operator=(const LuaScript&) = delete;

    // Internal state
    ScriptState* state = nullptr;
};
}