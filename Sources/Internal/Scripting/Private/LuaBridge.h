#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Reflection/Reflection.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace DAVA
{
namespace Lua
{
int32 Dava_register(lua_State* state);

static const char* AnyTName = "Any";
Any toAny(lua_State* state, int32 index);
Any checkAny(lua_State* state, int32 index);
Any* pushAny(lua_State* state, const Any& refl);
int32 Any_register(lua_State* state);

static const char* ReflectionTName = "Reflection";
Reflection toReflection(lua_State* state, int32 index);
Reflection checkReflection(lua_State* state, int32 index);
Reflection* pushReflection(lua_State* state, const Reflection& refl);
int32 Reflection_register(lua_State* state);

std::pair<bool, Any> luaToAny(lua_State* state, int32 index);
void anyToLua(lua_State* state, const Any& value);
}
}