#ifndef __LOGENGINE_EXCEPTION_H__
#define __LOGENGINE_EXCEPTION_H__

#include "Base/BaseTypes.h"

namespace Log
{
class Exception
{
public:
    virtual ~Exception();
    virtual const String& GetMessage()
    {
        return message;
    };

protected:
    Exception(const String& _message)
        : message(_message)
    {
    }
    String message;
};

// fatal exceptions
class FindFileException : public Exception
{
public:
    FindFileException()
        : Exception("[*** FatalException] failed to find file\n")
    {
    }
    ~FindFileException(){};

private:
};

// fatal exceptions
class MemoryAllocateException : public Exception
{
public:
    MemoryAllocateException()
        : Exception("[*** MemoryAllocateException] failed to allocate memory\n")
    {
    }
    ~MemoryAllocateException(){};

private:
};
};

#endif // __LOGENGINE_EXCEPTION_H__
