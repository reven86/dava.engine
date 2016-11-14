#include "CommandLine/Private/CommandLineModuleTestExecute.h"
#include "CommandLine/CommandLineModule.h"

void CommandLineModuleTestExecute::ExecuteModule(CommandLineModule* module)
{
    DVASSERT(module != nullptr);

    InitModule(module);

    while (ProcessModule(module) == false)
    {
        //module loop
    }

    FinalizeModule(module);
}

void CommandLineModuleTestExecute::InitModule(CommandLineModule* module)
{
    module->PostInit();
}

bool CommandLineModuleTestExecute::ProcessModule(CommandLineModule* module)
{
    bool completed = (module->OnFrame() == CommandLineModule::eFrameResult::FINISHED);
    return completed;
}

void CommandLineModuleTestExecute::FinalizeModule(CommandLineModule* module)
{
    module->BeforeDestroyed();
}
