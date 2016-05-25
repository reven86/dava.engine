#ifndef __COMMAND_STACK_H__
#define __COMMAND_STACK_H__

#include "Base/BaseTypes.h"
#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandBatch.h"

class CommandStack : public CommandNotifyProvider, public CommandNotify
{
public:
    CommandStack();
    ~CommandStack() override;

    bool CanRedo() const;
    bool CanUndo() const;

    void Clear();
    void RemoveCommands(DAVA::int32 commandId);

    void Undo();
    void Redo();
    void Exec(Command2::Pointer&& command);

    void BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount);
    void EndBatch();

    bool IsClean() const;
    void SetClean(bool clean);

    DAVA::int32 GetCleanIndex() const;
    DAVA::int32 GetNextIndex() const;

    DAVA::int32 GetUndoLimit() const;
    void SetUndoLimit(DAVA::int32 limit);

    DAVA::uint32 GetCount() const;
    const Command2* GetCommand(DAVA::int32 index) const;

private:
    //CommandNotify
    void Notify(const Command2* command, bool redo) override;

    using CommandsContainer = DAVA::List<Command2::Pointer>;

    void ExecInternal(Command2::Pointer&& command, bool runCommand);
    Command2* GetCommandInternal(DAVA::int32 index) const;

    void ClearRedoCommands();
    void ClearLimitedCommands();

    void CleanCheck();
    void CommandExecuted(const Command2* command, bool redo);

private:
    const DAVA::int32 INVALID_CLEAN_INDEX = static_cast<DAVA::int32>(-1);

    CommandsContainer commandList;
    std::unique_ptr<CommandBatch> curBatchCommand;

    DAVA::uint32 nestedBatchesCounter = 0;
    DAVA::int32 commandListLimit = 0;
    DAVA::int32 nextCommandIndex = 0;
    DAVA::int32 nextAfterCleanCommandIndex = 0;
    bool lastCheckCleanState = true;
};


#endif // __COMMAND_STACK_H__
