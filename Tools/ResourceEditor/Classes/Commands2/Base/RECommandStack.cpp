#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

RECommandStack::RECommandStack()
    : CommandStack()
{
    canRedoChanged.Connect(DAVA::MakeFunction(this, &RECommandStack::EmitUndoRedoStateChanged));
    canUndoChanged.Connect(DAVA::MakeFunction(this, &RECommandStack::EmitUndoRedoStateChanged));
}

RECommandStack::~RECommandStack()
{
    DisconnectEvents();
}

void RECommandStack::Clear()
{
    commands.clear();
    cleanIndex = EMPTY_INDEX;
    SetCurrentIndex(EMPTY_INDEX);
}

void RECommandStack::RemoveCommands(DAVA::CommandID_t commandId)
{
    ActiveStackGuard guard(this);
    int commandIndex = 0;
    commandManager->removeCommands([&commandId, &commandIndex, this](const wgt::CommandInstancePtr& instance)
                                   {
                                       DAVA::Command* cmd = instance->getArguments().getBase<DAVA::Command>();
                                       if (cmd->GetID() == DAVA::CMDID_BATCH)
                                       {
                                           RECommandBatch* batch = static_cast<RECommandBatch*>(cmd);

                                           batch->RemoveCommands(commandId);
                                           return batch->Size() == 0;
                                       }

                                       bool needToRemove = cmd->GetID() == commandId;
                                       if (needToRemove == true && commandIndex <= nextAfterCleanCommandIndex)
                                       {
                                           DVASSERT(nextAfterCleanCommandIndex >= 0);
                                           --nextAfterCleanCommandIndex;
                                       }
                                       else
                                       {
                                           ++commandIndex;
                                       }
                                       return needToRemove;
                                   });

    CleanCheck();
}

void RECommandStack::Activate()
{
    ActiveCommandStack::Instance()->SetActive(this);
    EmitUndoRedoStateChanged();
}

void RECommandStack::Undo()
{
    ActiveStackGuard guard(this);
    CommandStack::Undo();
}

void RECommandStack::Redo()
{
    ActiveStackGuard guard(this);
    CommandStack::Redo();
}

void RECommandStack::Exec(DAVA::Command::Pointer&& command)
{
    ActiveStackGuard guard(this);
    if (command != nullptr)
    {
        uncleanCommandIds.insert(command->GetID());
    }
    CommandStack::Exec(std::move(command));
}

bool RECommandStack::IsUncleanCommandExists(DAVA::CommandID_t commandId) const
{
    return uncleanCommandIds.count(commandId) > 0;
}

void RECommandStack::EndBatch()
{
    ActiveStackGuard guard(this);
    CommandStack::EndBatch();
}

bool RECommandStack::IsClean() const
{
    if (nextAfterCleanCommandIndex == SCENE_CHANGED_INDEX)
    {
        return false;
    }

    if (nextAfterCleanCommandIndex == nextCommandIndex)
    {
        return true;
    }

    DVASSERT(nextCommandIndex >= EMPTY_INDEX);
    DVASSERT(nextAfterCleanCommandIndex >= EMPTY_INDEX);

    ActiveStackGuard guard(this);
    DAVA::int32 startCommandIndex = 0;
    DAVA::int32 endCommandIndex = 0;
    if (nextAfterCleanCommandIndex < nextCommandIndex)
    {
        startCommandIndex = nextAfterCleanCommandIndex + 1;
        endCommandIndex = nextCommandIndex;
    }
    else
    {
        startCommandIndex = nextCommandIndex + 1;
        endCommandIndex = nextAfterCleanCommandIndex;
    }

    DVASSERT(endCommandIndex < static_cast<DAVA::int32>(history.size()));
    for (DAVA::int32 i = startCommandIndex; i <= endCommandIndex; ++i)
    {
        DAVA::Command* cmd = commands.at(i).get();
        if (cmd == nullptr || cmd->IsModifying())
            return false;
    }

    return true;
}

void RECommandStack::SetClean()
{
    nextAfterCleanCommandIndex = nextCommandIndex;

    ActiveStackGuard guard(this);
    uncleanCommandIds.clear();
    CommandStackLocal::CommandIdsAccumulator functor(uncleanCommandIds);
    DAVA::int32 historySize = commands.size();
    DVASSERT(nextAfterCleanCommandIndex < historySize);
    for (DAVA::int32 i = DAVA::Max(nextAfterCleanCommandIndex, 0); i < historySize; ++i)
    {
        functor(commands.at(i));
    }

    CleanCheck();
}

void RECommandStack::CleanCheck()
{
    if (lastCheckCleanState != IsClean())
    {
        lastCheckCleanState = IsClean();
        EmitCleanChanged(lastCheckCleanState);
    }
}

void RECommandStack::commandExecuted(const wgt::CommandInstance& commandInstance, wgt::CommandOperation operation)
{
    DAVA::Command* cmd = commandInstance.getArguments().getBase<DAVA::Command>();
    if (cmd != nullptr)
    {
        EmitNotify(cmd, operation != wgt::CommandOperation::UNDO);
    }
}

void RECommandStack::DisconnectEvents()
{
    indexChanged.disconnect();
}

void RECommandStack::EnableConections()
{
    commandManager->registerCommandStatusListener(this);
    indexChanged.enable();
}

void RECommandStack::DisableConnections()
{
    commandManager->deregisterCommandStatusListener(this);
    indexChanged.disable();
}
