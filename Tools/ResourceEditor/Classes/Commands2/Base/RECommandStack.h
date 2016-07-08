#pragma once

#include "Base/BaseTypes.h"
#include "NgtTools/Commands/CommandStack.h"
#include "Commands2/Base/CommandNotify.h"

#include <core_command_system/i_command_event_listener.hpp>
#include <core_common/signal.hpp>

class RECommandStack : public CommandStack, public CommandNotifyProvider, public wgt::ICommandEventListener
{
public:
    RECommandStack();
    ~RECommandStack() override;

    bool CanRedo() const;
    bool CanUndo() const;

    void Clear();
    void RemoveCommands(DAVA::CommandID_t commandId);

    void Activate();
    void Undo() override;
    void Redo() override;
    void Push(DAVA::Command::Pointer&& command) override;

    bool IsUncleanCommandExists(DAVA::CommandID_t commandId) const;

    void EndMacro() override;

    bool IsClean() const override;
    void SetClean() override;

private:
    //base class CanUndo and CanRedo can be called only by base class and reimplemented functions CanUndo and CanRedo must be called by other classes using public interface
    using CommandStack::CanUndo;
    using CommandStack::CanRedo;

    void commandExecuted(const wgt::CommandInstance& commandInstance, wgt::CommandOperation operation) override;
    void CleanCheck();

    void HistoryIndexChanged(int currentIndex);

    void EnableConections();
    void DisableConnections();
    void DisconnectEvents();

private:
    static const DAVA::int32 EMPTY_INDEX = -1;
    /// SCENE_CHANGED_INDEX we need to store state of command stack when Scene was changed without Command,
    /// that support Undo operation. EMPTY_INDEX is not enough for that, because when we open scene nextCommandIndex and
    /// nextAfterCleanCommandIndex are equal EMPTY_INDEX. If immediately after that user made changes without Command,
    /// nextAfterCleanCommandIndex will not change and scene will not be marked as changed
    const DAVA::int32 SCENE_CHANGED_INDEX = -2;

    class ActiveCommandStack;
    class ActiveStackGuard;

    DAVA::int32 nextCommandIndex = EMPTY_INDEX;
    DAVA::int32 nextAfterCleanCommandIndex = EMPTY_INDEX;
    bool lastCheckCleanState = true;

    DAVA::UnorderedSet<DAVA::CommandID_t> uncleanCommandIds;

    wgt::Connection indexChanged;
};
