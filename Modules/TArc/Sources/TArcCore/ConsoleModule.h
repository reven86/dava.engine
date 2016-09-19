#pragma once

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{

class ContextAccessor;
class ConsoleModule
{
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

private:
    friend class Core;
    void Init(ContextAccessor* accessor);

private:
    ContextAccessor* contextAccessor = 0;
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

} // namespace TArc
} // namespace DAVA
