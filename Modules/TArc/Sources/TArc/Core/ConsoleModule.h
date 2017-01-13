#pragma once

#include "Debug/DVAssert.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class ConsoleModule : public ReflectionBase
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

    DAVA_VIRTUAL_REFLECTION(ConsoleModule)
    {
    }
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
