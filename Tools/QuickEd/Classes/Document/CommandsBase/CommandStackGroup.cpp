#include "Debug/DVAssert.h"
#include "Document/CommandsBase/CommandStackGroup.h"
#include "Command/CommandStack.h"

using namespace DAVA;

CommandStackGroup::CommandStackGroup()
{
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
        activeStack->undoCommandChanged.Disconnect(this);
        activeStack->redoCommandChanged.Disconnect(this);
    }
    activeStack = commandStack;
    if (commandStack != nullptr)
    {
        DVASSERT(stacks.find(commandStack) != stacks.end());
        activeStack->cleanChanged.Connect(&cleanChanged, &Signal<bool>::Emit);
        activeStack->canRedoChanged.Connect(&canRedoChanged, &Signal<bool>::Emit);
        activeStack->canUndoChanged.Connect(&canUndoChanged, &Signal<bool>::Emit);
        activeStack->undoCommandChanged.Connect(&undoCommandChanged, &Signal<const Command*>::Emit);
        activeStack->redoCommandChanged.Connect(&redoCommandChanged, &Signal<const Command*>::Emit);
    }
    cleanChanged.Emit(IsClean());
    canRedoChanged.Emit(CanRedo());
    canUndoChanged.Emit(CanUndo());
    undoCommandChanged.Emit(GetUndoCommand());
    redoCommandChanged.Emit(GetRedoCommand());
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

const Command* CommandStackGroup::GetUndoCommand() const
{
    return activeStack != nullptr ? activeStack->GetUndoCommand() : nullptr;
}

const Command* CommandStackGroup::GetRedoCommand() const
{
    return activeStack != nullptr ? activeStack->GetRedoCommand() : nullptr;
}
