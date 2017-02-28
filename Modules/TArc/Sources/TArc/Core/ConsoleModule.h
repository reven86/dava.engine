#pragma once

#include "Debug/DVAssert.h"
#include "Reflection/ReflectionRegistrator.h"

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

    virtual int GetExitCode() const;

    ContextAccessor& GetAccessor();

private:
    friend class Core;
    friend class ConsoleModuleTestExecution;

    void Init(ContextAccessor* accessor);

private:
    ContextAccessor* contextAccessor = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ConsoleModule)
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

inline int ConsoleModule::GetExitCode() const
{
    return 0;
}

} // namespace TArc
} // namespace DAVA
