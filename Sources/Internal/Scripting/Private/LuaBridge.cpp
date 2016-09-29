#include "LuaBridge.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{
namespace Lua
{
int32 DV_Logger_Debug(lua_State* state)
{
    luaL_checktype(state, -1, LUA_TSTRING);
    const char* msg = lua_tostring(state, -1);
    Logger::Debug(msg);
    lua_pop(state, 1);
    return 0;
}

int32 DV_Logger_Error(lua_State* state)
{
    luaL_checktype(state, -1, LUA_TSTRING);
    const char* msg = lua_tostring(state, -1);
    Logger::Error(msg);
    lua_pop(state, 1);
    return 0;
}

int32 Dava_register(lua_State* state)
{
    static const struct luaL_reg davalib[] = {
        { "Debug", &DV_Logger_Debug },
        { "Error", &DV_Logger_Error },
        { nullptr, nullptr }
    };

    luaL_register(state, "DV", davalib);
    lua_pop(state, 1);
    return 1;
}

/******************************************************************************/

Any toAny(lua_State* state, int32 index)
{
    Any* pAny = static_cast<Any*>(lua_touserdata(state, index));
    if (!pAny)
    {
        luaL_typerror(state, index, AnyTName);
    }
    return *pAny;
}

Any checkAny(lua_State* state, int32 index)
{
    luaL_checktype(state, index, LUA_TUSERDATA);
    Any* pAny = static_cast<Any*>(luaL_checkudata(state, index, AnyTName));
    if (!pAny)
    {
        luaL_typerror(state, index, AnyTName);
    }
    return *pAny;
}

Any* pushAny(lua_State* state, const Any& refl)
{
    Any* pAny = static_cast<Any*>(lua_newuserdata(state, sizeof(Any)));
    *pAny = refl;
    luaL_getmetatable(state, AnyTName);
    lua_setmetatable(state, -2);
    return pAny;
}

int32 Any_gc(lua_State* state)
{
    // TODO: need?
    return 0;
}

int32 Any_tostring(lua_State* state)
{
    Any any = checkAny(state, 1);
    void* pAny = lua_touserdata(state, 1);
    lua_pushfstring(state, "Any: %s (%p)", any.GetType()->GetName(), pAny);
    return 1;
}

int32 Any_register(lua_State* state)
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

Reflection toReflection(lua_State* state, int32 index)
{
    Reflection* pRef = static_cast<Reflection*>(lua_touserdata(state, index));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

Reflection checkReflection(lua_State* state, int32 index)
{
    luaL_checktype(state, index, LUA_TUSERDATA);
    Reflection* pRef = static_cast<Reflection*>(luaL_checkudata(state, index, ReflectionTName));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

Reflection* pushReflection(lua_State* state, const Reflection& refl)
{
    Reflection* pRef = static_cast<Reflection*>(lua_newuserdata(state, sizeof(Reflection)));
    *pRef = refl;
    luaL_getmetatable(state, ReflectionTName);
    lua_setmetatable(state, -2);
    return pRef;
}

int32 Reflection_gc(lua_State* state)
{
    // TODO: need?
    return 0;
}

int32 Reflection_tostring(lua_State* state)
{
    Reflection refl = checkReflection(state, 1);
    void* pRef = lua_touserdata(state, 1);
    lua_pushfstring(state, "Reflection: %s (%p)", refl.GetValueType()->GetName(), pRef);
    return 1;
}

int32 Reflection_index(lua_State* state)
{
    Reflection self = checkReflection(state, 1);

    Any name;
    int ltype = lua_type(state, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(state, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(state, 2)));
        break;
    default:
        return luaL_error(state, "Wrong key type %d!", ltype);
    }

    Reflection refl = self.GetField(name).ref;
    if (!refl.IsValid())
    {
        lua_pushnil(state);
        return 1;
    }

    if (refl.HasFields() || refl.HasMethods())
    {
        pushReflection(state, refl);
        return 1;
    }

    Any value = refl.GetValue();
    if (value.CanGet<int32>())
    {
        lua_pushinteger(state, value.Get<int32>());
    }
    else if (value.CanGet<int16>())
    {
        lua_pushinteger(state, value.Get<int16>());
    }
    else if (value.CanGet<int8>())
    {
        lua_pushinteger(state, value.Get<int8>());
    }
    else if (value.CanGet<float64>())
    {
        lua_pushnumber(state, value.Get<float64>());
    }
    else if (value.CanGet<float32>())
    {
        lua_pushnumber(state, value.Get<float32>());
    }
    else if (value.CanGet<String>())
    {
        const String& res = value.Get<String>();
        lua_pushlstring(state, res.c_str(), res.length());
    }
    else if (value.CanGet<WideString>())
    {
        const WideString& res = value.Get<WideString>();
        const String& utf = UTF8Utils::EncodeToUTF8(res);
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

int32 Reflection_newindex(lua_State* state)
{
    Reflection self = checkReflection(state, 1);

    Any name;
    int ltype = lua_type(state, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(state, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(state, 2)));
        break;
    default:
        return luaL_error(state, "Wrong key type %d!", ltype);
    }

    Reflection refl = self.GetField(name).ref;
    if (refl.IsValid())
    {
        Any value;
        int ltype = lua_type(state, 3);
        switch (ltype)
        {
        case LUA_TBOOLEAN:
            value.Set(bool(lua_toboolean(state, 3) != 0));
            break;
        case LUA_TNUMBER:
            value.Set(float64(lua_tonumber(state, 3)));
            break;
        case LUA_TSTRING:
            value.Set(String(lua_tolstring(state, 3, nullptr)));
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
        if (value.GetType() == Type::Instance<float64>())
        {
            float64 rawValue = value.Get<float64>();
            if (refl.GetValueType() == Type::Instance<int32>())
            {
                refl.SetValue(Any(static_cast<int32>(rawValue)));
            }
            else if (refl.GetValueType() == Type::Instance<int16>())
            {
                refl.SetValue(Any(static_cast<int16>(rawValue)));
            }
            else if (refl.GetValueType() == Type::Instance<int8>())
            {
                refl.SetValue(Any(static_cast<int8>(rawValue)));
            }
            else if (refl.GetValueType() == Type::Instance<float32>())
            {
                refl.SetValue(Any(static_cast<float32>(rawValue)));
            }
        }
        else if (value.GetType() == Type::Instance<String>() &&
                 refl.GetValueType() == Type::Instance<WideString>())
        {
            const WideString& wstr = UTF8Utils::EncodeToWideString(value.Get<String>());
            refl.SetValue(Any(wstr));
        }
        else
        {
            refl.SetValue(value);
        }

        return 0;
    }

    return 0;
}

int32 Reflection_len(lua_State* state)
{
    Reflection self = checkReflection(state, 1);
    lua_pushinteger(state, self.GetFields().size());
    return 1;
}

int32 Reflection_register(lua_State* state)
{
    static const luaL_reg Reflection_meta[] = {
        { "__gc", &Reflection_gc },
        { "__tostring", &Reflection_tostring },
        { "__index", &Reflection_index },
        { "__newindex", &Reflection_newindex },
        { "__len", &Reflection_len },
        { nullptr, nullptr }
    };

    luaL_newmetatable(state, ReflectionTName);
    luaL_register(state, 0, Reflection_meta);
    lua_pop(state, 1);
    return 1;
}

std::pair<bool, Any> luaToAny(lua_State* state, int32 index)
{
    std::pair<bool, Any> res;
    res.first = true;
    int ltype = lua_type(state, index);
    switch (ltype)
    {
    case LUA_TBOOLEAN:
        res.second.Set(bool(lua_toboolean(state, index) != 0));
        break;
    case LUA_TNUMBER:
        res.second.Set(float64(lua_tonumber(state, index)));
        break;
    case LUA_TSTRING:
        res.second.Set(String(lua_tolstring(state, index, nullptr)));
        break;
    case LUA_TUSERDATA:
    {
        void* ud = nullptr;
        if (ud = luaL_checkudata(state, index, AnyTName))
        {
            res.second = *static_cast<Any*>(ud);
        }
        else if (ud = luaL_checkudata(state, index, ReflectionTName))
        {
            res.second = *static_cast<Reflection*>(ud);
        }
        else
        {
            res.first = false;
        }
    }
    break;
    case LUA_TNIL:
        res.second.Clear();
        break;
    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
        res.first = false;
    }
    return res;
}

void anyToLua(lua_State* state, const Any& value)
{
}
}
}