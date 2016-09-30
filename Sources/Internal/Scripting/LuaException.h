#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
 * \brief Class for Lua errors handling as exception
 */
class LuaException : public std::runtime_error //TODO: use DAVA exception as base
{
public:
    /**
     * \brief Default constructor
     */
    LuaException();

    /**
     * \brief Detailed constructor
     */
    LuaException(int32 code, const String& msg);

    /**
     * \brief Return stored error code
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