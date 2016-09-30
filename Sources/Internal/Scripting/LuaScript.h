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
class LuaScript final
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
     * \brief Load script from string, run it and return vector of results.
     *        Throw LuaException on error.
     */
    Vector<Any> RunString(const String& script);

    /**
     * \brief Load script from file, run it and return vector of results.
     *        Throw LuaException on error.
     */
    Vector<Any> RunFile(const FilePath& filepath);

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
     * \brief Load script from string, run it and return vector of results.
     *        Return false on error.
     */
    bool RunStringSafe(const String& script, Vector<Any>* results = nullptr);

    /**
     * \brief Load script from file, run it and return vector of results. Return false on error.
     */
    bool RunFileSafe(const FilePath& filepath, Vector<Any>* results = nullptr);

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

    /**
     * \brief Set value to global table with name vName
     */
    void SetGlobalValue(const String& vName, const Any& value);

    //TODO:
    //  LuaScript CreateNewThread(); // Create new LuaScript object with lua_State is new thread of currect lua_State

private:
    // Lua state is non-copyable
    LuaScript(const LuaScript&) = delete;
    LuaScript& operator=(const LuaScript&) = delete;

    /**
     * \brief Pop (top - fromIndex) elements from stack and return they as
     *        vector of Any.
     *        Lua stack changes [-(top-fromIndex), +0, e]
     */
    Vector<Any> PopElementsFromStackToAny(int32 fromIndex);

    // Internal state
    ScriptState* state = nullptr;
};
}