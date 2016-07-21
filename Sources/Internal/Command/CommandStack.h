#pragma once

#include "Command/Command.h"
#include "Functional/Signal.h"

namespace DAVA
{
class CommandBatch;

class CommandStack
{
public:
    CommandStack();
    virtual ~CommandStack();

    void Exec(Command::Pointer&& command);

    void BeginBatch(const String& name, uint32 commandsCount = 0);
    void EndBatch();

    bool IsClean() const;
    void SetClean();

    void Undo();
    void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    Signal<bool> cleanChanged;
    Signal<bool> canUndoChanged;
    Signal<bool> canRedoChanged;
    Signal<int32> currentIndexChanged;

protected:
    void UpdateCleanState();
    void SetCurrentIndex(int32 currentIndex);

    static const DAVA::int32 EMPTY_INDEX = -1;

    int32 cleanIndex = EMPTY_INDEX;
    int32 currentIndex = EMPTY_INDEX;

    Vector<Command::Pointer> commands;

    //this stack is created to hold nested batches hierarchy
    DAVA::Stack<DAVA::CommandBatch*> batchesStack;
    //root command batch which created when "BeginBatch" is called first time
    DAVA::Command::Pointer rootBatch;

    //members to remember stack state and do not emit extra signals
    bool isClean = true;
    bool canUndo = false;
    bool canRedo = false;

    void SetClean(bool isClean);
    void SetCanUndo(bool canUndo);
    void SetCanRedo(bool canRedo);
};
}
