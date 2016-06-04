#pragma once

#include "Base/BaseTypes.h"
#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandBatch.h"

#include <core_command_system/i_command_event_listener.hpp>
#include <core_common/signal.hpp>

namespace wgt
{
class ICommandManager;
}

class CommandStack : public CommandNotifyProvider, public wgt::ICommandEventListener
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
    void commandExecuted(const wgt::CommandInstance& commandInstance, wgt::CommandOperation operation) override;
    void CleanCheck();

    void HistoryIndexChanged(int currentIndex);

    void EnableConections();
    void DisableConnections();
    void DisconnectEvents();

private:
    const DAVA::int32 EMPTY_INDEX = -1;
    /// SCENE_CHANGED_INDEX we need to store state of command stack when Scene was changed without Command,
    /// that support Undo operation. EMPTY_INDEX is not enough for that, because when we open scene nextCommandIndex and
    /// nextAfterCleanCommandIndex are equal EMPTY_INDEX. If immediately after that user made changes without Command,
    /// nextAfterCleanCommandIndex will not change and scene will not be marked as changed
    const DAVA::int32 SCENE_CHANGED_INDEX = -2;

    class ActiveCommandStack;
    class ActiveStackGuard;

    wgt::ICommandManager* commandManager = nullptr;
    std::unique_ptr<CommandBatch> curBatchCommand;

    int enviromentID = 0;
    DAVA::uint32 nestedBatchesCounter = 0;
    DAVA::int32 nextCommandIndex = EMPTY_INDEX;
    DAVA::int32 nextAfterCleanCommandIndex = EMPTY_INDEX;
    bool lastCheckCleanState = true;

    DAVA::UnorderedSet<DAVA::int32> uncleanCommandIds;

    wgt::Connection indexChanged;
};
