#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class DemoObj
{
    DAVA_DECLARE_TYPE_INITIALIZER;

public:
    int a = 99;
    String b = "DEMO_TEST";
};

DAVA_TYPE_INITIALIZER(DemoObj)
{
    DAVA::ReflectionRegistrator<DemoObj>::Begin()
    .Field("a", &DemoObj::a)
    .Field("b", &DemoObj::b)
    .End();
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static DAVA::int32 lua_logger_debug(lua_State* state)
{
    const char* msg = lua_tostring(state, -1);
    DAVA::Logger::Debug(msg);
    lua_pop(state, 1);
    return 0;
}

static const struct luaL_reg davalib[] = {
    { "Debug", &lua_logger_debug },
    { nullptr, nullptr }
};

static DAVA::int32 Dava_register(lua_State* state)
{
    luaL_openlib(state, "DV", davalib, 0);
    return 1;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static const char* ReflectionTName = "DAVA::Reflection";

static DAVA::Reflection toReflection(lua_State* state, DAVA::int32 index)
{
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(lua_touserdata(state, index));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

static DAVA::Reflection checkReflection(lua_State* state, DAVA::int32 index)
{
    luaL_checktype(state, index, LUA_TUSERDATA);
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(luaL_checkudata(state, index, ReflectionTName));
    if (!pRef)
    {
        luaL_typerror(state, index, ReflectionTName);
    }
    return *pRef;
}

static DAVA::Reflection* pushReflection(lua_State* state, const DAVA::Reflection& refl)
{
    DAVA::Reflection* pRef = static_cast<DAVA::Reflection*>(lua_newuserdata(state, sizeof(DAVA::Reflection)));
    *pRef = refl;
    luaL_getmetatable(state, ReflectionTName);
    lua_setmetatable(state, -2);
    return pRef;
}

static DAVA::int32 Reflection_ref(lua_State* state)
{
    DAVA::Reflection refl = checkReflection(state, 1);
    String name = luaL_checkstring(state, 2);

    const DAVA::ReflectedObject& obj = refl.GetValueObject();
    const DAVA::StructureWrapper* wr = refl.GetStructure();

    DAVA::Ref::Field field = wr->GetField(obj, name);
    pushReflection(state, field.valueRef);
    return 1;
}

static DAVA::int32 Reflection_value(lua_State* state)
{
    DAVA::Reflection refl = checkReflection(state, 1);

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
    else if (value.CanGet<bool>())
    {
        lua_pushboolean(state, value.Get<bool>());
    }
    else // unknown type
    {
        //         Any* res = (Any*)lua_newuserdata(state, sizeof(value));
        //         *res = std::move(value);
        luaL_error(state, "Complex type");
    }
    return 1;
}

static DAVA::int32 Reflection_set(lua_State* state)
{
    DAVA::Reflection refl = checkReflection(state, 1);

    Any value;
    int ltype = lua_type(state, 2);
    switch (ltype)
    {
    case LUA_TBOOLEAN:
        value.Set(lua_toboolean(state, 2));
        break;
    case LUA_TNUMBER:
        value.Set(float64(lua_tonumber(state, 2)));
        break;
    case LUA_TSTRING:
        value.Set(String(lua_tolstring(state, 2, nullptr)));
        break;
    case LUA_TUSERDATA:
    //         DAVA::Any* any = static_cast<DAVA::Any*>(lua_touserdata(state, 2));
    //         value = *any;
    //         break;
    case LUA_TNIL:
    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
        luaL_error(state, "Wrong input type!");
        break;
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
    else
    {
        refl.SetValue(value);
    }

    return 0;
}

static const luaL_reg Reflection_methods[] = {
    { "ref", &Reflection_ref },
    { "value", &Reflection_value },
    { "set", &Reflection_set },
    { nullptr, nullptr }
};

static DAVA::int32 Reflection_gc(lua_State* state)
{
    // TODO: need?
    return 0;
}

static DAVA::int32 Reflection_tostring(lua_State* state)
{
    DAVA::Reflection refl = checkReflection(state, 1);
    void* pRef = lua_touserdata(state, 1);
    lua_pushfstring(state, "Reflection: %s (%p)", refl.GetValueType()->GetName(), pRef);
    return 1;
}

static const luaL_reg Reflection_meta[] = {
    { "__gc", &Reflection_gc },
    { "__tostring", &Reflection_tostring },
    { nullptr, nullptr }
};

static int32 Reflection_register(lua_State* state)
{
    /* create methods table, add it to the globals */
    luaL_openlib(state, ReflectionTName, Reflection_methods, 0);
    /* create metatable for Image, add it to the Lua registry */
    luaL_newmetatable(state, ReflectionTName);
    /* fill metatable */
    luaL_openlib(state, 0, Reflection_meta, 0);
    lua_pushliteral(state, "__index");
    /* dup methods table*/
    lua_pushvalue(state, -3);
    /* metatable.__index = methods */
    lua_rawset(state, -3);
    lua_pushliteral(state, "__metatable");
    /* dup methods table*/
    lua_pushvalue(state, -3);
    /* hide metatable: metatable.__metatable = methods */
    lua_rawset(state, -3);
    /* drop metatable */
    lua_pop(state, 1);
    /* return methods on the stack */
    return 1;
}

// static const char* AnyTName = "DAVA::Any";
//
// static DAVA::Any toAny(lua_State* state, DAVA::int32 index)
// {
//     DAVA::Any* pAny = static_cast<DAVA::Any*>(lua_touserdata(state, index));
//     if (!pAny)
//     {
//         luaL_typerror(state, index, AnyTName);
//     }
//     return *pAny;
// }
//
// static DAVA::Any checkAny(lua_State* state, DAVA::int32 index)
// {
//     luaL_checktype(state, index, LUA_TUSERDATA);
//     DAVA::Any* pAny = static_cast<DAVA::Any*>(luaL_checkudata(state, index, AnyTName));
//     if (!pAny)
//     {
//         luaL_typerror(state, index, AnyTName);
//     }
//     return *pAny;
// }
//
// static DAVA::Any* pushAny(lua_State* state, const DAVA::Any& refl)
// {
//     DAVA::Any* pAny = static_cast<DAVA::Any*>(lua_newuserdata(state, sizeof(DAVA::Any)));
//     *pAny = refl;
//     luaL_getmetatable(state, AnyTName);
//     lua_setmetatable(state, -2);
//     return pAny;
// }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

class Script
{
public:
    Script()
    {
        state = luaL_newstate();
        luaL_openlibs(state);

        Dava_register(state);
        Reflection_register(state);
    }

    ~Script()
    {
        lua_close(state);
    }

    bool LoadString(const String& script)
    {
        int res = luaL_loadstring(state, script.c_str());
        DVASSERT_MSG(res == 0, "Can't load script");
        if (res != 0)
        {
            Logger::Error("LUA Error: %s", lua_tostring(state, -1));
            return false;
        }
        res = lua_pcall(state, 0, LUA_MULTRET, 0);
        DVASSERT_MSG(res == 0, "Can't execute script");
        if (res != 0)
        {
            Logger::Error("LUA Error: %s", lua_tostring(state, -1));
            return false;
        }
        return true;
    }

    bool LoadFile(const String& filepath)
    {
        int res = luaL_loadfile(state, filepath.c_str());
        DVASSERT_MSG(res == 0, "Can't load file");
        if (res != 0)
        {
            Logger::Error("LUA Error: %s", lua_tostring(state, -1));
            return false;
        }
        res = lua_pcall(state, 0, LUA_MULTRET, 0);
        DVASSERT_MSG(res == 0, "Can't execute script");
        if (res != 0)
        {
            Logger::Error("LUA Error: %s", lua_tostring(state, -1));
            return false;
        }
        return true;
    }

    bool Run(const DAVA::Reflection& context)
    {
        lua_getglobal(state, "main");
        pushReflection(state, context);
        int res = lua_pcall(state, 1, 1, 0);

        if (res != 0)
        {
            DVASSERT_MSG(false, "Can't execute main()");
            Logger::Error("LUA Error: %s", lua_tostring(state, -1));
        }

        return res == 0;
    }

private:
    lua_State* state = nullptr;
};

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

using namespace DAVA;

ScriptingTest::ScriptingTest()
    : BaseScreen("ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    static const String demo_script =
    R"script(

DV.Debug("LUA: static code")

function main(context)
    aRef = context:ref("a")
    DV.Debug("-->> 1: " .. aRef:value())
    aRef:set(956)
    DV.Debug("-->> 2: " .. aRef:value())
    DV.Debug("-->> 3: " .. context:ref("b"):value())
end

)script";

    DemoObj obj;
    DAVA::Reflection objRef = DAVA::Reflection::Reflect(&obj);

    Script s;
    s.LoadString(demo_script);
    s.Run(objRef);
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
