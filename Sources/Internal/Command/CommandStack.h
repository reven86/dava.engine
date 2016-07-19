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

    virtual void Exec(Command::Pointer&& command);

    virtual void BeginBatch(const String& name, uint32 commandsCount = 0);
    virtual void EndBatch();

    virtual bool IsClean() const;
    virtual void SetClean();

    virtual void Undo();
    virtual void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    Signal<bool> cleanChanged;
    Signal<bool> canUndoChanged;
    Signal<bool> canRedoChanged;
    Signal<int32> currentIndexChanged;

protected:
    bool CanUndoImpl() const;
    bool CanRedoImpl() const;

private:
    void UpdateCleanState();
    void SetCurrentIndex(int32 currentIndex);

    int32 cleanIndex = -1;
    int32 currentIndex = -1;

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
