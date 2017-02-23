#include "LuaBridge.h"
#include "Scripting/LuaException.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Utils/StringFormat.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace LuaBridge
{
/*
Some functions have an indicator like this: Lua stack changes [-o, +p, x].
The first field, o, is how many elements the function pops from the stack.
The second field, p, is how many elements the function pushes onto the stack.
(Any function always pushes its results after popping its arguments.)
A field in the form x|y means the function can push (or pop) x or y elements,
depending on the situation; an interrogation mark '?' means that we cannot know
how many elements the function pops/pushes by looking only at its arguments
(e.g., they may depend on what is on the stack).
The third field, x, tells whether the function may throw errors:
'-' means the function never throws any error;
'm' means the function may throw an error only due to not enough memory;
'e' means the function may throw other kinds of errors;
'v' means the function may throw an error on purpose.
*/

/******************************************************************************/

// Lua global table name for Dava service functions.
static const char* DavaNamespace = "DV";

/*
Check and pop string variable from stack and print it to log.
Lua stack changes [-1, +0, v]
*/
int32 DV_Debug(lua_State* L)
{
    luaL_checktype(L, -1, LUA_TSTRING);
    const char* msg = lua_tostring(L, -1);
    lua_pop(L, 1);
    Logger::Debug(msg);
    return 0;
}

/*
Check and pop string variable from stack and print it to log.
Lua stack changes [-1, +0, v]
*/
int32 DV_Error(lua_State* L)
{
    luaL_checktype(L, -1, LUA_TSTRING);
    const char* msg = lua_tostring(L, -1);
    Logger::Error(msg);
    lua_pop(L, 1);
    return 0;
}

void RegisterDava(lua_State* L)
{
    static const struct luaL_reg davalib[] = {
        { "Debug", &DV_Debug },
        { "Error", &DV_Error },
        { nullptr, nullptr }
    };

    luaL_register(L, DavaNamespace, davalib);
    lua_pop(L, 1);
}

/******************************************************************************/

// Any metatable type name
static const char* AnyTName = "AnyT";

/*
Get userdata from stack with index and return it as Any.
Return empty any if can't get.
Lua stack changes [-0, +0, -]
*/
Any* lua_todvany(lua_State* L, int32 index)
{
    return static_cast<Any*>(lua_touserdata(L, index));
}

/*
Check and get userdata from stack with index and return it as Any.
Throw lua_error on incorrect Lua type.
Lua stack changes [-0, +0, v]
*/
Any* lua_checkdvany(lua_State* L, int32 index)
{
    Any* pAny = static_cast<Any*>(luaL_checkudata(L, index, AnyTName));
    DVASSERT(pAny, "Can't get Any ptr");
    return pAny;
}

/*
Push new userdata with Any metatable to top on the stack.
Lua stack changes [-0, +1, -]
*/
void lua_pushdvany(lua_State* L, const Any& any)
{
    void* userdata = lua_newuserdata(L, sizeof(Any));
    DVASSERT(userdata, "Can't create Any ptr");
    new (userdata) Any(any);
    luaL_getmetatable(L, AnyTName);
    lua_setmetatable(L, -2);
}

/*
Meta method for presentation Any as string.
Lua stack changes [-0, +1, -]
*/
int32 Any__tostring(lua_State* L)
{
    Any* pAny = lua_checkdvany(L, 1);
    lua_pushfstring(L, "Any: %s (%p)", pAny->IsEmpty() ? "<empty>" : pAny->GetType()->GetName(), pAny);
    return 1;
}

void RegisterAny(lua_State* L)
{
    static const luaL_reg Any_meta[] = {
        { "__tostring", &Any__tostring }, // Control string representation
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

    luaL_newmetatable(L, AnyTName);
    luaL_register(L, 0, Any_meta);
    lua_pop(L, 1);
}

/******************************************************************************/

// AnyFn metatable type name
static const char* AnyFnTName = "AnyFnT";

/*
Get userdata from stack with specified index and return it as AnyFn.
Return empty any if can't get.
Lua stack changes [-0, +0, -]
*/
AnyFn* lua_todvanyfn(lua_State* L, int32 index)
{
    return static_cast<AnyFn*>(lua_touserdata(L, index));
}

/*
Check and get userdata from stack with specified index and return it as AnyFn.
Throw lua_error on incorrect Lua type.
Lua stack changes [-0, +0, v]
*/
AnyFn* lua_checkdvanyfn(lua_State* L, int32 index)
{
    AnyFn* pAnyFn = static_cast<AnyFn*>(luaL_checkudata(L, index, AnyFnTName));
    DVASSERT(pAnyFn, "Can't get AnyFn ptr");
    return pAnyFn;
}

/*
Push new userdata with AnyFn metatable to top on the stack.
Lua stack changes [-0, +1, -]
*/
void lua_pushdvanyfn(lua_State* L, const AnyFn& any)
{
    void* userdata = lua_newuserdata(L, sizeof(AnyFn));
    DVASSERT(userdata, "Can't create AnyFn ptr");
    new (userdata) AnyFn(any);
    luaL_getmetatable(L, AnyFnTName);
    lua_setmetatable(L, -2);
}

/*
Meta method for presentation AnyFn as string.
Lua stack changes [-0, +1, v]
*/
int32 AnyFn__tostring(lua_State* L)
{
    AnyFn* pAnyFn = lua_checkdvanyfn(L, 1);

    String funcDef = "<non valid>";
    if (pAnyFn->IsValid())
    {
        const auto& params = pAnyFn->GetInvokeParams();
        if (params.retType)
        {
            funcDef += params.retType->GetName();
        }
        funcDef += " Func(";
        bool first = true;
        for (const Type* t : params.argsType)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                funcDef += ", ";
            }
            funcDef += t->GetName();
        }
        funcDef += ")";
    }

    lua_pushfstring(L, "Any: %s (%p)", funcDef.c_str(), pAnyFn);
    return 1;
}

/*
Meta method for invoke AnyFn.
Lua stack changes [-0, +(0|1), v]
*/
int32 AnyFn__call(lua_State* L)
{
    AnyFn* self = lua_checkdvanyfn(L, 1);
    int32 nargs = lua_gettop(L) - 1; // Lower element in stack is AnyFn userdata
    const Vector<const Type*>& argsTypes = self->GetInvokeParams().argsType;

    DVASSERT(nargs >= 0, "Lua stack corrupted!");
    if (nargs != argsTypes.size())
    {
        return luaL_error(L, "Incorrect number of arguments to invoke AnyFn (need %d, found %d)", argsTypes.size(), nargs);
    }

    Any result;
    try
    {
        switch (nargs)
        {
        case 0:
        {
            result = self->Invoke();
            break;
        }
        case 1:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            result = self->Invoke(a1);
            break;
        }
        case 2:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            Any a2 = LuaToAny(L, 3, argsTypes[1]);
            result = self->Invoke(a1, a2);
            break;
        }
        case 3:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            Any a2 = LuaToAny(L, 3, argsTypes[1]);
            Any a3 = LuaToAny(L, 4, argsTypes[2]);
            result = self->Invoke(a1, a2, a3);
            break;
        }
        case 4:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            Any a2 = LuaToAny(L, 3, argsTypes[1]);
            Any a3 = LuaToAny(L, 4, argsTypes[2]);
            Any a4 = LuaToAny(L, 5, argsTypes[3]);
            result = self->Invoke(a1, a2, a3, a4);
            break;
        }
        case 5:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            Any a2 = LuaToAny(L, 3, argsTypes[1]);
            Any a3 = LuaToAny(L, 4, argsTypes[2]);
            Any a4 = LuaToAny(L, 5, argsTypes[3]);
            Any a5 = LuaToAny(L, 6, argsTypes[4]);
            result = self->Invoke(a1, a2, a3, a4, a5);
            break;
        }
        case 6:
        {
            Any a1 = LuaToAny(L, 2, argsTypes[0]);
            Any a2 = LuaToAny(L, 3, argsTypes[1]);
            Any a3 = LuaToAny(L, 4, argsTypes[2]);
            Any a4 = LuaToAny(L, 5, argsTypes[3]);
            Any a5 = LuaToAny(L, 6, argsTypes[4]);
            Any a6 = LuaToAny(L, 7, argsTypes[5]);
            result = self->Invoke(a1, a2, a3, a4, a5, a6);
            break;
        }
        default:
            return luaL_error(L, "Too much arguments (%d) to invoke AnyFn", nargs);
        }
    }
    catch (const LuaException& e)
    {
        return luaL_error(L, e.what());
    }
    AnyToLua(L, result);
    return 1;
}

void RegisterAnyFn(lua_State* L)
{
    static const luaL_reg AnyFn_meta[] = {
        { "__tostring", &AnyFn__tostring },
        { "__call", &AnyFn__call },
        { nullptr, nullptr }
    };

    luaL_newmetatable(L, AnyFnTName);
    luaL_register(L, 0, AnyFn_meta);
    lua_pop(L, 1);
}

/******************************************************************************/

// Reflection metatable type name
static const char* ReflectionTName = "ReflectionT";

/*
Get userdata from stack with specified index and return it as Reflection.
Return empty reflection if can't get.
Lua stack changes [-0, +0, -]
*/
Reflection* lua_todvreflection(lua_State* L, int32 index)
{
    return static_cast<Reflection*>(lua_touserdata(L, index));
}

/*
Check and get userdata from stack with specified index and return it as Reflection.
Throw lua_error on incorrect Lua type.
Lua stack changes [-0, +0, v]
*/
Reflection* lua_checkdvreflection(lua_State* L, int32 index)
{
    Reflection* pRef = static_cast<Reflection*>(luaL_checkudata(L, index, ReflectionTName));
    DVASSERT(pRef, "Can't get Reflection ptr");
    return pRef;
}

/*
Push new userdata with Reflection metatable to top on the stack.
Lua stack changes [-0, +1, -]
*/
void lua_pushdvreflection(lua_State* L, const Reflection& refl)
{
    void* userdata = lua_newuserdata(L, sizeof(Reflection));
    DVASSERT(userdata, "Can't get Reflection ptr");
    new (userdata) Reflection(refl);
    luaL_getmetatable(L, ReflectionTName);
    lua_setmetatable(L, -2);
}

/*
Meta method for presentation Any as string.
Lua stack changes [-0, +1, -]
*/
int32 Reflection__tostring(lua_State* L)
{
    Reflection* pRefl = lua_checkdvreflection(L, 1);
    lua_pushfstring(L, "Reflection: %s (%p)", pRefl->IsValid() ? pRefl->GetValueType()->GetName() : "<non valid>", pRefl);
    return 1;
}

/*
Meta method for getting element from Reflection userdata object.
Lua stack changes [-0, +1, v]
*/
int32 Reflection__index(lua_State* L)
{
    Reflection* self = lua_checkdvreflection(L, 1);

    Any name;
    int ltype = lua_type(L, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(L, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(L, 2)));
        break;
    default:
        return luaL_error(L, "Wrong key type \"%s\"!", lua_typename(L, ltype));
    }

    Reflection refl = self->GetField(name);
    if (refl.IsValid())
    {
        if (refl.HasFields() || refl.HasMethods())
        {
            lua_pushdvreflection(L, refl);
            return 1;
        }

        AnyToLua(L, refl.GetValue());
        return 1;
    }

    AnyFn method = self->GetMethod(name.Get<String>());
    if (method.IsValid())
    {
        lua_pushdvanyfn(L, method);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

/*
Meta method for setting value to Reflection userdata object.
Lua stack changes [-0, +0, v]
*/
int32 Reflection__newindex(lua_State* L)
{
    Reflection* self = lua_checkdvreflection(L, 1);

    Any name;
    int ltype = lua_type(L, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(L, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(L, 2)));
        break;
    default:
        return luaL_error(L, "Wrong key type \"%s\"!", lua_typename(L, ltype));
    }

    Reflection refl = self->GetField(name);
    if (refl.IsValid())
    {
        try
        {
            Any value = LuaToAny(L, 3, refl.GetValueType());
            refl.SetValue(value);
        }
        catch (const LuaException& e)
        {
            return luaL_error(L, e.what());
        }
    }
    return 0;
}

void RegisterReflection(lua_State* L)
{
    static const luaL_reg Reflection_meta[] = {
        { "__tostring", &Reflection__tostring },
        { "__index", &Reflection__index },
        { "__newindex", &Reflection__newindex },
        { nullptr, nullptr }
    };

    luaL_newmetatable(L, ReflectionTName);
    luaL_register(L, 0, Reflection_meta);
    lua_pop(L, 1);
}

/******************************************************************************/

/*
Compare value on top of the stack with metatable named metatableName.
Lua stack changes [-0, +0, v]
*/
bool lua_equalmetatable(lua_State* L, const char* metatableName)
{
    luaL_getmetatable(L, metatableName); // stack +1 (top: metatable metatableName)
    int32 eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 1); // stack -1 (remove metatable metatableName)
    return eq == 1;
}

Any LuaToAny(lua_State* L, int32 index, const Type* preferredType /*= nullptr*/)
{
#define ISTYPE(t) (preferredType == Type::Instance<t>())
#define CASTTYPE(t, ex) (Any(static_cast<t>(ex)))
#define IFCAST(t, luaFn) if ISTYPE(t) { return CASTTYPE(t, luaFn(L, index)); }
#define THROWTYPE DAVA_THROW(LuaException, LUA_ERRRUN, Format("Can cast Lua type (%s) to preferred type (%s)", lua_typename(L, ltype), preferredType->GetName()));

    int ltype = lua_type(L, index);
    switch (ltype)
    {
    case LUA_TNIL:
        if (preferredType)
        {
            THROWTYPE
        }
        else
        {
            return Any();
        }
    case LUA_TBOOLEAN:
        if (preferredType)
        {
            if
                ISTYPE(bool)
                {
                    return CASTTYPE(bool, lua_toboolean(L, index) != 0);
                }
            else
                IFCAST(int8, lua_tointeger)
            else IFCAST(int16, lua_tointeger)
            else IFCAST(int32, lua_tointeger)
            else IFCAST(int64, lua_tonumber)
            else THROWTYPE
        }
        else
        {
            return CASTTYPE(bool, lua_toboolean(L, index) != 0);
        }
    case LUA_TNUMBER:
        if (preferredType)
        {
            IFCAST(float32, lua_tonumber)
            else IFCAST(float64, lua_tonumber)
            else IFCAST(int8, lua_tointeger)
            else IFCAST(int16, lua_tointeger)
            else IFCAST(int32, lua_tointeger)
            else IFCAST(int64, lua_tonumber)
            else IFCAST(char8, lua_tointeger)
            else IFCAST(char16, lua_tointeger)
            else THROWTYPE
        }
        else
        {
            return CASTTYPE(float64, lua_tonumber(L, index));
        }
    case LUA_TSTRING:
        if (preferredType)
        {
            IFCAST(String, lua_tostring)
            else if ISTYPE(WideString)
            {
                const String str(lua_tostring(L, index));
                const WideString wstr(UTF8Utils::EncodeToWideString(str));
                return Any(wstr);
            }
            else THROWTYPE
        }
        else
        {
            return CASTTYPE(String, lua_tostring(L, index));
        }
    case LUA_TUSERDATA:
        if (lua_getmetatable(L, index) != 0) // stack +1 (top: metatable of userdata object)
        {
            if (lua_equalmetatable(L, AnyTName))
            {
                lua_pop(L, 1); // stack -1
                Any* any = lua_todvany(L, index);
                if (preferredType
                    && !ISTYPE(Any)
                    && any->GetType() != preferredType)
                {
                    THROWTYPE;
                }
                return *any;
            }
            else if (lua_equalmetatable(L, AnyFnTName))
            {
                lua_pop(L, 1); // stack -1
                AnyFn* pAnyFn = lua_todvanyfn(L, index);
                if (preferredType && !ISTYPE(AnyFn))
                    THROWTYPE;
                return Any(*pAnyFn);
            }
            else if (lua_equalmetatable(L, ReflectionTName))
            {
                lua_pop(L, 1); // stack -1
                Reflection* pRefl = lua_todvreflection(L, index);
                if (preferredType
                    && !ISTYPE(Reflection)
                    && pRefl->GetValueType() != preferredType)
                {
                    THROWTYPE;
                }
                return Any(*pRefl);
            }
            else
            {
                lua_pop(L, 1); // stack -1
                DAVA_THROW(LuaException, LUA_ERRRUN, "Unknown userdata type!");
            }
        }
        else // stack +0
        {
            DAVA_THROW(LuaException, LUA_ERRRUN, "Unknown userdata type without metatable!");
        }
    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
        DAVA_THROW(LuaException, LUA_ERRRUN, Format("Unsupported Lua type \"%s\"!", lua_typename(L, ltype)));
    }

#undef ISTYPE
#undef CASTTYPE
#undef IFCAST
#undef THROWTYPE
}

void AnyToLua(lua_State* L, const Any& value)
{
#define CANGET(t) (value.CanGet<t>())
#define IFPUSH(t, luaFn) if CANGET(t) { luaFn(L, value.Get<t>()); }

    if (value.IsEmpty())
    {
        lua_pushnil(L); // Push nil if any is empty
    }
    else
        IFPUSH(int8, lua_pushinteger)
    else IFPUSH(int16, lua_pushinteger)
    else IFPUSH(int32, lua_pushinteger)
    else if (CANGET(int64))
    {
        lua_pushnumber(L, static_cast<lua_Number>(value.Get<int64>()));
    }
    else IFPUSH(float32, lua_pushnumber)
    else IFPUSH(float64, lua_pushnumber)
    else IFPUSH(char8, lua_pushinteger)
    else IFPUSH(char16, lua_pushinteger)
    else IFPUSH(bool, lua_pushboolean)
    else IFPUSH(const char*, lua_pushstring)
    else if CANGET(String)
    {
        const String& str = value.Get<String>();
        lua_pushlstring(L, str.c_str(), str.length());
    }
    else if CANGET(WideString)
    {
        const WideString& res = value.Get<WideString>();
        const String& utf = UTF8Utils::EncodeToUTF8(res);
        lua_pushlstring(L, utf.c_str(), res.length());
    }
    else if CANGET(Reflection)
    {
        Reflection ref = value.Get<Reflection>();
        if (ref.IsValid())
        {
            lua_pushdvreflection(L, ref);
        }
        else
        {
            lua_pushnil(L); // Push nil if reflection isn't valid
        }
    }
    else IFPUSH(AnyFn, lua_pushdvanyfn)
    else // unknown type, push as is
    {
        lua_pushdvany(L, value);
    }

#undef CANGET
#undef IFPUSH
}

String PopString(lua_State* L)
{
    String msg;
    if (lua_gettop(L) > 0)
    {
        msg = lua_tostring(L, -1);
        lua_pop(L, 1); // stack -1
    }
    return msg;
}

void DumpStack(lua_State* L, std::ostream& os)
{
    int32 count = lua_gettop(L);
    os << "Lua stack (top: " << count << ")" << (count > 0 ? ":" : "");
    for (int32 i = 1; i <= count; ++i)
    {
        os << std::endl
           << "  " << i << ") <" << luaL_typename(L, i) << ">";
        int ltype = lua_type(L, i);
        switch (ltype)
        {
        case LUA_TBOOLEAN:
            os << " " << (lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            os << " " << lua_tonumber(L, i);
            break;
        case LUA_TSTRING:
            os << " " << lua_tostring(L, i);
            break;
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            os << " " << lua_touserdata(L, i);
            break;
        case LUA_TNIL:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        default:
            break;
        }
    }
}
}
}
