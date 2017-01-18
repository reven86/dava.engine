#include "TArc/Testing/ConsoleModuleTestExecution.h"
#include "TArc/Core/ConsoleModule.h"
#include "TArc/Core/ContextAccessor.h"

namespace DAVA
{
namespace TArc
{
void ConsoleModuleTestExecution::ExecuteModule(ConsoleModule* module, ContextAccessor* accessor)
{
    DVASSERT(module != nullptr);

    InitModule(module, accessor);

    while (ProcessModule(module) == false)
    {
        //module loop
    }

    FinalizeModule(module);
}

void ConsoleModuleTestExecution::InitModule(ConsoleModule* module, ContextAccessor* accessor)
{
    if (accessor != nullptr)
    {
        module->Init(accessor);
    }
    module->PostInit();
}

bool ConsoleModuleTestExecution::ProcessModule(ConsoleModule* module)
{
    bool completed = (module->OnFrame() == ConsoleModule::eFrameResult::FINISHED);
    return completed;
}

void ConsoleModuleTestExecution::FinalizeModule(ConsoleModule* module)
{
    module->BeforeDestroyed();
}
}
}
