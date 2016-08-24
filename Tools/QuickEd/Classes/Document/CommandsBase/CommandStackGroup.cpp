#include "Debug/DVAssert.h"
#include "Document/CommandsBase/CommandStackGroup.h"
#include "Command/CommandStack.h"

CommandStackGroup::CommandStackGroup()
{
}

void CommandStackGroup::RemoveStack(DAVA::CommandStack* stackToRemove)
{
    if (activeStack == stackToRemove)
    {
        SetActiveStack(nullptr);
    }
    stacks.erase(stackToRemove);
}

void CommandStackGroup::AddStack(DAVA::CommandStack* stackToAdd)
{
    stacks.insert(stackToAdd);
}

void CommandStackGroup::SetActiveStack(DAVA::CommandStack* commandStack)
{
    if (activeStack != nullptr)
    {
        activeStack->cleanChanged.Disconnect(this);
        activeStack->canRedoChanged.Disconnect(this);
        activeStack->canUndoChanged.Disconnect(this);
        activeStack->undoTextChanged.Disconnect(this);
        activeStack->redoTextChanged.Disconnect(this);
    }
    activeStack = commandStack;
    if (commandStack != nullptr)
    {
        DVASSERT(stacks.find(commandStack) != stacks.end());
        activeStack->cleanChanged.Connect(&cleanChanged, &DAVA::Signal<bool>::Emit);
        activeStack->canRedoChanged.Connect(&canRedoChanged, &DAVA::Signal<bool>::Emit);
        activeStack->canUndoChanged.Connect(&canUndoChanged, &DAVA::Signal<bool>::Emit);
        activeStack->undoTextChanged.Connect(&undoTextChanged, &DAVA::Signal<const DAVA::String&>::Emit);
        activeStack->redoTextChanged.Connect(&redoTextChanged, &DAVA::Signal<const DAVA::String&>::Emit);
    }
    cleanChanged.Emit(IsClean());
    canRedoChanged.Emit(CanRedo());
    canUndoChanged.Emit(CanUndo());
    undoTextChanged.Emit(GetUndoText());
    redoTextChanged.Emit(GetRedoText());
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

DAVA::String CommandStackGroup::GetUndoText() const
{
    return activeStack != nullptr ? activeStack->GetUndoText() : DAVA::String();
}

DAVA::String CommandStackGroup::GetRedoText() const
{
    return activeStack != nullptr ? activeStack->GetRedoText() : DAVA::String();
}
