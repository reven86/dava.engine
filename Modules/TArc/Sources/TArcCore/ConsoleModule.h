#pragma once

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class ConsoleModule
{
public:
    int GetExitCode() const;

protected:
    enum class eFrameResult
    {
        CONTINUE,
        FINISHED
    };
    virtual void PostInit() = 0;
    virtual eFrameResult OnFrame() = 0;
    virtual void BeforeDestroyed() = 0;

    ContextAccessor& GetAccessor();

    void SetExitCode(int);

private:
    friend class Core;
    void Init(ContextAccessor* accessor);

private:
    ContextAccessor* contextAccessor = 0;
    int exitCode = 0;
};

inline void ConsoleModule::Init(ContextAccessor* accessor)
{
    DVASSERT(contextAccessor == nullptr);
    contextAccessor = accessor;
}

inline ContextAccessor& ConsoleModule::GetAccessor()
{
    DVASSERT(contextAccessor != nullptr);
    return *contextAccessor;
}

inline void ConsoleModule::SetExitCode(int code)
{
    exitCode = code;
}

inline int ConsoleModule::GetExitCode() const
{
    return exitCode;
}

} // namespace TArc
} // namespace DAVA
