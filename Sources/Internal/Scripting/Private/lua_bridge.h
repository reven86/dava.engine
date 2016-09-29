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

DAVA::int32 Dava_register(lua_State* state);

static const char* AnyTName = "DAVA::Any";
DAVA::Any toAny(lua_State* state, DAVA::int32 index);
DAVA::Any checkAny(lua_State* state, DAVA::int32 index);
DAVA::Any* pushAny(lua_State* state, const DAVA::Any& refl);
DAVA::int32 Any_register(lua_State* state);

static const char* ReflectionTName = "DAVA::Reflection";
DAVA::Reflection toReflection(lua_State* state, DAVA::int32 index);
DAVA::Reflection checkReflection(lua_State* state, DAVA::int32 index);
DAVA::Reflection* pushReflection(lua_State* state, const DAVA::Reflection& refl);
DAVA::int32 Reflection_register(lua_State* state);
}
}