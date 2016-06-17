#include "Debug/DVAssert.h"
#include "Document/CommandsBase/CommandStackGroup.h"
#include "Document/CommandsBase/CommandStack.h"

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
    if (activeStack != nullptr)
    {
        activeStack->DisconnectFromCommandManager();
    }
    activeStack = commandStack;
    if (commandStack != nullptr)
    {
        DVASSERT(stacks.find(commandStack) != stacks.end());
        envManager->selectEnv(activeStack->ID);
        activeStack->ConnectToCommandManager();
    }
}
