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

struct lua_State* state;

static DAVA::int32 Dava_register(lua_State* state);

static const char* AnyTName = "DAVA::Any";
static DAVA::Any toAny(lua_State* state, DAVA::int32 index);
static DAVA::Any checkAny(lua_State* state, DAVA::int32 index);
static DAVA::Any* pushAny(lua_State* state, const DAVA::Any& refl);
static DAVA::int32 Any_register(lua_State* state);

static const char* ReflectionTName = "DAVA::Reflection";
static DAVA::Reflection toReflection(lua_State* state, DAVA::int32 index);
static DAVA::Reflection checkReflection(lua_State* state, DAVA::int32 index);
static DAVA::Reflection* pushReflection(lua_State* state, const DAVA::Reflection& refl);
static DAVA::int32 Reflection_register(lua_State* state);
