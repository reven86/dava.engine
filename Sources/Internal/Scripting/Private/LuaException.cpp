#include "Scripting/LuaException.h"

namespace DAVA
{
LuaException::LuaException()
    : std::runtime_error("")
{
}

LuaException::LuaException(int32 code, const String& msg)
    : std::runtime_error(msg)
    , code(code)
{
}
}