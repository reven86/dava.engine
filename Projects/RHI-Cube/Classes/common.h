/*-------------------------------------------------------------------------------------------------
file:   common.h
author: Mosiychuck Dmitry
dsc:
-------------------------------------------------------------------------------------------------*/
#pragma once

#include "platform.h"
//#include "math/math.h"

typedef unsigned int STR_ID;
typedef unsigned int ERR_ID;

enum SYSTEM_ERR_ID
{
    ERR_OK = 0,
    ERR_UNKNOWN,
    ERR_BADOP,
    ERR_BADVAL,
    ERR_RECURSION,
    ERR_ASSERTFAILED,
    ERR_MEMORYFAILURE,
    ERR_WRONGDELETEOPERATOR,
    ERR_WRONGMEMORYADDRESS,
    ERR_FUNCTIONFAILURE,
    ERR_FILESYSTEMERROR,
    ERR_FILENOTFOUND,
    ERR_BADFILE,
    ERR_UNKNOWNWIN32,
    ERR_UNKNOWND3D,
};

#define _string std::string
#define _vector std::vector
#define _stack std::stack
#define _queue std::queue
#define _list std::list
#define _set std::set
#define _map std::map
#define _multimap std::multimap

class CNonCopyble
{
private:
    CNonCopyble(const CNonCopyble& r)
    {
    }
    CNonCopyble& operator=(const CNonCopyble& r)
    {
        return *this;
    }

protected:
    CNonCopyble(void)
    {
    }
    virtual ~CNonCopyble(void)
    {
    }
};

#define IMPLEMENT_MEMORY_TRACKING
#define NEW new
#define DELETE(p) \
    {if (p){delete (p);(p) = 0;}}
#define DELETE_ARRAY(p) \
    {if (p){delete[](p);(p) = 0;}}

#define TRY(f) f;
#define TRY_WIN32(f) f;
#define TRY_D3D(f) f;

#define ASSERT(x) \
    {\
        if (!(x))\
        {\
            __debugbreak; \
        }\
    }
