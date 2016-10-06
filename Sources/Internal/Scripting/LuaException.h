#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
Class for Lua errors handling as exception
*/
class LuaException : public std::runtime_error //TODO: use DAVA exception as base
{
public:
    /**
    Default constructor
    */
    LuaException();

    /**
    Detailed constructor
    */
    LuaException(int32 code, const String& msg);

    /**
    Return stored error code
    */
    int32 error_code() const;

private:
    int32 code = -1;
};

inline int32 LuaException::error_code() const
{
    return code;
}
}