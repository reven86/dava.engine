#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

class CommandStack;

namespace wgt
{
class IEnvManager;
}

class CommandStackGroup : DAVA::TrackedObject
{
public:
    CommandStackGroup();
    void RemoveStack(CommandStack* stackToRemove);
    void AddStack(CommandStack* stackToAdd);

    void SetActiveStack(CommandStack* commandStack);

    void Undo();
    void Redo();

    bool IsClean() const;
    bool CanUndo() const;
    bool CanRedo() const;

    DAVA::Signal<bool> cleanChanged;
    DAVA::Signal<bool> canUndoChanged;
    DAVA::Signal<bool> canRedoChanged;

private:
    CommandStack* activeStack = nullptr;
    DAVA::Set<CommandStack*> stacks;
    wgt::IEnvManager* envManager = nullptr;
};
