#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace DAVA
{
namespace LuaBridge
{
/**
 * \brief Register in lua_State new DV namespace with service functions.
 *        Lua stack changes [-0, +0, -]
 */
void RegisterDava(lua_State* L);

/**
 * \brief Register in lua_State metatable for Any userdata type.
 *        Lua stack changes [-0, +0, -]
 */
void RegisterAny(lua_State* L);

/**
 * \brief Register in lua_State metatable for Reflection userdata type.
 *        Lua stack changes [-0, +0, -]
 */
void RegisterReflection(lua_State* L);

/**
 * \brief Gets Lua variable from stack with index and convert it to Any.
 *        Lua stack changes [-0, +0, -]
 */
Any luaToAny(lua_State* L, int32 index);

/**
 * \brief Push Any as Lua variable to stack.
 *        Lua stack changes [-0, +1, e]
 */
void anyToLua(lua_State* L, const Any& value);

/**
 * \brief Get string from top of stack and pop it.
 *        Lua stack changes [-1, +0, -]
 */
String PopString(lua_State* L);
}
}