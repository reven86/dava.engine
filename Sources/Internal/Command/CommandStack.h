#pragma once

#include "Command/Command.h"
#include "Functional/Signal.h"

namespace DAVA
{
class CommandBatch;

class CommandStack
{
public:
    virtual ~CommandStack();

    void Exec(std::unique_ptr<Command>&& command);

    void BeginBatch(const String& name, uint32 commandsCount = 1);
    void EndBatch();

    bool IsClean() const;
    void SetClean();

    void Undo();
    void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    DAVA::String GetUndoText() const;
    DAVA::String GetRedoText() const;

    Signal<bool> cleanChanged;
    Signal<bool> canUndoChanged;
    Signal<bool> canRedoChanged;
    Signal<int32, int32> currentIndexChanged;
    Signal<const DAVA::String&> undoTextChanged;
    Signal<const DAVA::String&> redoTextChanged;

protected:
    virtual std::unique_ptr<CommandBatch> CreateCommmandBatch(const String& name, uint32 commandsCount) const;

    void ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand);

    void UpdateCleanState();
    void SetCurrentIndex(int32 currentIndex);

    static const int32 EMPTY_INDEX = -1;

    int32 cleanIndex = EMPTY_INDEX;
    int32 currentIndex = EMPTY_INDEX;

    Vector<std::unique_ptr<Command>> commands;

    //this stack is created to hold nested batches hierarchy
    Stack<CommandBatch*> batchesStack;
    //root command batch which created when "BeginBatch" is called first time
    std::unique_ptr<Command> rootBatch;

    //members to remember stack state and do not emit extra signals
    bool isClean = true;
    bool canUndo = false;
    bool canRedo = false;

    void SetClean(bool isClean);
    void SetCanUndo(bool canUndo);
    void SetCanRedo(bool canRedo);
};
}
