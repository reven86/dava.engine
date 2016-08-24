#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

namespace DAVA
{
class CommandStack;
}
class CommandStackGroup : DAVA::TrackedObject
{
public:
    CommandStackGroup();
    void RemoveStack(DAVA::CommandStack* stackToRemove);
    void AddStack(DAVA::CommandStack* stackToAdd);

    void SetActiveStack(DAVA::CommandStack* commandStack);

    void Undo();
    void Redo();

    bool IsClean() const;
    bool CanUndo() const;
    bool CanRedo() const;

    DAVA::String GetUndoText() const;
    DAVA::String GetRedoText() const;

    DAVA::Signal<bool> cleanChanged;
    DAVA::Signal<bool> canUndoChanged;
    DAVA::Signal<bool> canRedoChanged;

    DAVA::Signal<const DAVA::String&> undoTextChanged;
    DAVA::Signal<const DAVA::String&> redoTextChanged;

private:
    DAVA::CommandStack* activeStack = nullptr;
    DAVA::Set<DAVA::CommandStack*> stacks;
};
