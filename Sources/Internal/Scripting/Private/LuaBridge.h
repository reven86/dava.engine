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
Register in Lua new DV namespace with service functions.
Lua stack changes [-0, +0, -]
*/
void RegisterDava(lua_State* L);

/**
Register in Lua metatable for Any userdata type.
Lua stack changes [-0, +0, -]
*/
void RegisterAny(lua_State* L);

/**
Register in Lua metatable for AnyFn userdata type.
Lua stack changes [-0, +0, -]
*/
void RegisterAnyFn(lua_State* L);

/**
Register in Lua metatable for Reflection userdata type.
Lua stack changes [-0, +0, -]
*/
void RegisterReflection(lua_State* L);

/**
Gets Lua variable from stack with specified index and convert it to Any.
Lua stack changes [-0, +0, -]
*/
Any LuaToAny(lua_State* L, int32 index, const Type* preferredType = nullptr);

/**
Put specified value as Lua variable to stack.
Lua stack changes [-0, +1, e]
*/
void AnyToLua(lua_State* L, const Any& value);

/**
Get string from top of stack and pop it.
Lua stack changes [-1, +0, -]
*/
String PopString(lua_State* L);

/**
Dump Lua stack to Debug output.
Lua stack changes [-0, +0, -]
*/
void DumpStack(lua_State* L);
}
}