#pragma once

#include "Base/BaseTypes.h"
#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandBatch.h"

#include <core_command_system/i_command_event_listener.hpp>
#include <core_common/signal.hpp>

class ICommandManager;

class CommandStack : public CommandNotifyProvider, public ICommandEventListener
{
public:
    CommandStack();
    ~CommandStack() override;

    bool CanRedo() const;
    bool CanUndo() const;

    void Clear();
    void RemoveCommands(DAVA::int32 commandId);

    void Activate();
    void Undo();
    void Redo();
    void Exec(Command2::Pointer&& command);

    bool IsUncleanCommandExists(DAVA::int32 commandId) const;

    void BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount);
    void EndBatch();

    bool IsClean() const;
    void SetClean(bool clean);

private:
    void commandExecuted(const CommandInstance& commandInstance, CommandOperation operation) override;
    void CleanCheck();

    void HistoryIndexChanged(int currentIndex);

    void EnableConections();
    void DisableConnections();
    void DisconnectEvents();

private:
    const DAVA::int32 EMPTY_INDEX = -1;

    class ActiveCommandStack;
    class ActiveStackGuard;

    ICommandManager* commandManager = nullptr;
    std::unique_ptr<CommandBatch> curBatchCommand;

    int enviromentID = 0;
    DAVA::uint32 nestedBatchesCounter = 0;
    DAVA::int32 nextCommandIndex = EMPTY_INDEX;
    DAVA::int32 nextAfterCleanCommandIndex = EMPTY_INDEX;
    bool lastCheckCleanState = true;

    DAVA::UnorderedSet<DAVA::int32> uncleanCommandIds;

    Connection indexChanged;
};
