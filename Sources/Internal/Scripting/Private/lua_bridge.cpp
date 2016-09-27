#include "lua_bridge.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

namespace lua
{
DAVA::int32 lua_logger_debug(lua_State* state)
{
    luaL_checktype(state, -1, LUA_TSTRING);
    const char* msg = lua_tostring(state, -1);
    DAVA::Logger::Debug(msg);
    lua_pop(state, 1);
    return 0;
}

DAVA::int32 Dava_register(lua_State* state)
{
    static const struct luaL_reg davalib[] = {
        { "Debug", &lua_logger_debug },
        { nullptr, nullptr }
    };

    luaL_openlib(state, "DV", davalib, 0);
    return 1;
}

/******************************************************************************/

DAVA::Any toAny(lua_State* state, DAVA::int32 index)
{
    DAVA::Any* pAny = static_cast<DAVA::Any*>(lua_touserdata(state, index));
    if (!pAny)
    {
        luaL_typerror(state, index, AnyTName);
    }
    return *pAny;
}

DAVA::Any checkAny(lua_State* state, DAVA::int32 index)
{
    luaL_checktype(state, index, LUA_TUSERDATA);
    DAVA::Any* pAny = static_cast<DAVA::Any*>(luaL_checkudata(state, index, AnyTName));
    if (!pAny)
    {
        luaL_typerror(state, index, AnyTName);
    }
    return *pAny;
}

DAVA::Any* pushAny(lua_State* state, const DAVA::Any& refl)
{
    DAVA::Any* pAny = static_cast<DAVA::Any*>(lua_newuserdata(state, sizeof(DAVA::Any)));
    *pAny = refl;
    luaL_getmetatable(state, AnyTName);
    lua_setmetatable(state, -2);
    return pAny;
}

DAVA::int32 Any_gc(lua_State* state)
{
    // TODO: need?
    return 0;
}

DAVA::int32 Any_tostring(lua_State* state)
{
    DAVA::Any any = checkAny(state, 1);
    void* pAny = lua_touserdata(state, 1);
    lua_pushfstring(state, "Any: %s (%p)", any.GetType()->GetName(), pAny);
    return 1;
}

DAVA::int32 Any_register(lua_State* state)
{
    static const luaL_reg Any_methods[] = {
        { nullptr, nullptr }
    };

    static const luaL_reg Any_meta[] = {
        { "__gc", &Any_gc }, // Userdata finalize code
        { "__tostring", &Any_tostring }, // Control string representation
        // { "__call",  }, // Treat a table like a function
        // { "__len",  }, // Control table length
        // { "__mode",  }, // Control weak references
        // { "__unm",  }, // Unary minus
        // { "__add",  }, // Addition
        // { "__sub",  }, // Subtraction
        // { "__mul",  }, // Multiplication
        // { "__div",  }, // Division
        // { "__mod",  }, // Modulo
        // { "__pow",  }, // Involution
        // { "__concat",  }, // Concatenation
        // { "__eq",  }, // Check for equality
        // { "__lt",  }, // Check for less-than
        // { "__le",  }, // Check for less-than-or-equal
        { nullptr, nullptr }
    };

    luaL_openlib(state, AnyTName, Any_methods, 0);
    luaL_newmetatable(state, AnyTName);
    luaL_openlib(state, 0, Any_meta, 0);
    lua_pushliteral(state, "__index"); // Hide index. __index = methods
    lua_pushvalue(state, -3);
    lua_rawset(state, -3);
    lua_pushliteral(state, "__metatable"); // Hide metatable. __metatable = methods
    lua_pushvalue(state, -3);
    lua_rawset(state, -3);
    lua_pop(state, 1);
    return 1;
}

/******************************************************************************/

DAVA::Reflection toReflection(lua_State* state, DAVA::int32 index)
{
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(lua_touserdata(state, index));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

DAVA::Reflection checkReflection(lua_State* state, DAVA::int32 index)
{
    luaL_checktype(state, index, LUA_TUSERDATA);
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(luaL_checkudata(state, index, ReflectionTName));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

DAVA::Reflection* pushReflection(lua_State* state, const DAVA::Reflection& refl)
{
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(lua_newuserdata(state, sizeof(DAVA::Reflection)));
    *pRef = refl;
    luaL_getmetatable(state, ReflectionTName);
    lua_setmetatable(state, -2);
    return pRef;
}

DAVA::int32 Reflection_gc(lua_State* state)
{
    // TODO: need?
    return 0;
}

DAVA::int32 Reflection_tostring(lua_State* state)
{
    DAVA::Reflection refl = checkReflection(state, 1);
    void* pRef = lua_touserdata(state, 1);
    lua_pushfstring(state, "Reflection: %s (%p)", refl.GetValueType()->GetName(), pRef);
    return 1;
}

DAVA::int32 Reflection_index(lua_State* state)
{
    DAVA::Reflection self = checkReflection(state, 1);
    DAVA::String name = luaL_checkstring(state, 2);

    DAVA::Reflection refl = self.GetField(name).ref;
    if (!refl.IsValid())
    {
        return 0;
    }

    if (refl.HasFields() || refl.HasMethods())
    {
        pushReflection(state, refl);
        return 1;
    }

    DAVA::Any value = refl.GetValue();
    if (value.CanGet<DAVA::int32>())
    {
        lua_pushinteger(state, value.Get<DAVA::int32>());
    }
    else if (value.CanGet<DAVA::int16>())
    {
        lua_pushinteger(state, value.Get<DAVA::int16>());
    }
    else if (value.CanGet<DAVA::int8>())
    {
        lua_pushinteger(state, value.Get<DAVA::int8>());
    }
    else if (value.CanGet<DAVA::float64>())
    {
        lua_pushnumber(state, value.Get<DAVA::float64>());
    }
    else if (value.CanGet<DAVA::float32>())
    {
        lua_pushnumber(state, value.Get<DAVA::float32>());
    }
    else if (value.CanGet<DAVA::String>())
    {
        const DAVA::String& res = value.Get<DAVA::String>();
        lua_pushlstring(state, res.c_str(), res.length());
    }
    else if (value.CanGet<DAVA::WideString>())
    {
        const DAVA::WideString& res = value.Get<DAVA::WideString>();
        const DAVA::String& utf = DAVA::UTF8Utils::EncodeToUTF8(res);
        lua_pushlstring(state, utf.c_str(), res.length());
    }
    else if (value.CanGet<bool>())
    {
        lua_pushboolean(state, value.Get<bool>());
    }
    else // unknown type
    {
        pushAny(state, value);
    }
    return 1;
}

DAVA::int32 Reflection_newindex(lua_State* state)
{
    DAVA::Reflection self = checkReflection(state, 1);
    DAVA::String name = luaL_checkstring(state, 2);

    DAVA::Reflection refl = self.GetField(name).ref;
    if (refl.IsValid())
    {
        DAVA::Any value;
        int ltype = lua_type(state, 3);
        switch (ltype)
        {
        case LUA_TBOOLEAN:
            value.Set(bool(lua_toboolean(state, 3) != 0));
            break;
        case LUA_TNUMBER:
            value.Set(DAVA::float64(lua_tonumber(state, 3)));
            break;
        case LUA_TSTRING:
            value.Set(DAVA::String(lua_tolstring(state, 3, nullptr)));
            break;
        case LUA_TUSERDATA:
            value = checkAny(state, 3);
            break;
        case LUA_TNIL:
        case LUA_TLIGHTUSERDATA:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        default:
            return luaL_error(state, "Wrong input type!");
        }

        // Cast-HACK
        if (value.GetType() == DAVA::Type::Instance<DAVA::float64>())
        {
            DAVA::float64 rawValue = value.Get<DAVA::float64>();
            if (refl.GetValueType() == DAVA::Type::Instance<DAVA::int32>())
            {
                refl.SetValue(DAVA::Any(static_cast<DAVA::int32>(rawValue)));
            }
            else if (refl.GetValueType() == DAVA::Type::Instance<DAVA::int16>())
            {
                refl.SetValue(DAVA::Any(static_cast<DAVA::int16>(rawValue)));
            }
            else if (refl.GetValueType() == DAVA::Type::Instance<DAVA::int8>())
            {
                refl.SetValue(DAVA::Any(static_cast<DAVA::int8>(rawValue)));
            }
            else if (refl.GetValueType() == DAVA::Type::Instance<DAVA::float32>())
            {
                refl.SetValue(DAVA::Any(static_cast<DAVA::float32>(rawValue)));
            }
        }
        else if (value.GetType() == DAVA::Type::Instance<DAVA::String>() &&
                 refl.GetValueType() == DAVA::Type::Instance<DAVA::WideString>())
        {
            const DAVA::WideString& wstr = DAVA::UTF8Utils::EncodeToWideString(value.Get<DAVA::String>());
            refl.SetValue(DAVA::Any(wstr));
        }
        else
        {
            refl.SetValue(value);
        }

        return 0;
    }

    return 0;
}

DAVA::int32 Reflection_register(lua_State* state)
{
    static const luaL_reg Reflection_meta[] = {
        { "__gc", &Reflection_gc },
        { "__tostring", &Reflection_tostring },
        { "__index", &Reflection_index },
        { "__newindex", &Reflection_newindex },
        { nullptr, nullptr }
    };

    luaL_newmetatable(state, ReflectionTName);
    luaL_register(state, 0, Reflection_meta);
    return 1;
}
}