#include "Debug/DVAssert.h"
#include "Document/CommandsBase/CommandStackGroup.h"
#include "Document/CommandsBase/CommandStack.h"

#include "NgtTools/Common/GlobalContext.h"
#include <core_command_system/i_env_system.hpp>

CommandStackGroup::CommandStackGroup()
{
    envManager = NGTLayer::queryInterface<wgt::IEnvManager>();
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
        activeStack->cleanChanged.Disconnect(this);
        activeStack->canRedoChanged.Disconnect(this);
        activeStack->canUndoChanged.Disconnect(this);

        activeStack->DisconnectFromCommandManager();
    }
    activeStack = commandStack;
    if (commandStack != nullptr)
    {
        DVASSERT(stacks.find(commandStack) != stacks.end());
        envManager->selectEnv(activeStack->ID);
        activeStack->ConnectToCommandManager();

        activeStack->cleanChanged.Connect(&cleanChanged, &DAVA::Signal<bool>::Emit);
        activeStack->canRedoChanged.Connect(&canRedoChanged, &DAVA::Signal<bool>::Emit);
        activeStack->canUndoChanged.Connect(&canUndoChanged, &DAVA::Signal<bool>::Emit);
    }
    else
    {
        cleanChanged.Emit(true);
        canRedoChanged.Emit(false);
        canUndoChanged.Emit(false);
    }
}

void CommandStackGroup::Undo()
{
    DVASSERT(activeStack != nullptr);
    if (activeStack != nullptr)
    {
        activeStack->Undo();
    }
}

void CommandStackGroup::Redo()
{
    DVASSERT(activeStack != nullptr);
    if (activeStack != nullptr)
    {
        activeStack->Redo();
    }
}

bool CommandStackGroup::IsClean() const
{
    return activeStack != nullptr ? activeStack->IsClean() : true;
}

bool CommandStackGroup::CanUndo() const
{
    return activeStack != nullptr ? activeStack->CanUndo() : false;
}

bool CommandStackGroup::CanRedo() const
{
    return activeStack != nullptr ? activeStack->CanRedo() : false;
}
