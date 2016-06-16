#include "Document/Commands/CommandStackGroup.h"
#include "Debug/DVAssert.h"

#include "NgtTools/Common/GlobalContext.h"
#include <core_command_system/i_env_system.hpp>

CommandStackGroup::CommandStackGroup()
{
    envManager = NGTLayer::queryInterface<IEnvManager>();
    DVASSERT(envManager);
}

void CommandStackGroup::RemoveStack(CommandStack* stackToRemove)
{
    if (activeStack == stackToRemove)
    {
        SetActiveStack(nullptr);
    }
    stacks.erase(stackToRemove);
}

void CommandStackGroup::AddStack(CommandStack* stackToAdd)
{
    stacks.insert(stackToAdd);
}

void CommandStackGroup::SetActiveStack(CommandStack* commandStack)
{
    if (stacks.find(commandStack) == stacks.end())
    {
        DVASSERT(false && "Can not find stack to set it active");
        return;
    }
    activeStack = commandStack;
}
