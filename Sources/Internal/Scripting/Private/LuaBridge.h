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
 * \brief Register in lua_State new DV namespace with service functions
 */
void RegisterDava(lua_State* L);

/**
 * \brief Register in lua_State metatable for Any userdata type
 */
void RegisterAny(lua_State* L);

/**
* \brief Register in lua_State metatable for Reflection userdata type
*/
void RegisterReflection(lua_State* L);

/**
 * \brief Gets Lua variable from stack with index and convert it to Any
 */
Any luaToAny(lua_State* L, int32 index);

/**
 * \brief Push Any as Lua variable to stack
 */
void anyToLua(lua_State* L, const Any& value);
}
}