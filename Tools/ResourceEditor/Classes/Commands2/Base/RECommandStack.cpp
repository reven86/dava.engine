#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

#include "NgtTools/Commands/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_data_model/variant_list.hpp>
#include <core_data_model/i_value_change_notifier.hpp>
#include <core_command_system/command_instance.hpp>
#include <core_command_system/i_env_system.hpp>
#include <core_reflection/reflected_object.hpp>
#include <core_command_system/i_command_manager.hpp>

namespace CommandStackLocal
{
class CommandIdsAccumulator
{
public:
    CommandIdsAccumulator(DAVA::UnorderedSet<DAVA::CommandID_t>& commandIds_)
        : commandIds(commandIds_)
    {
    }

    void operator()(const wgt::CommandInstancePtr& instance)
    {
        DAVA::Command* cmd = instance->getArguments().getBase<DAVA::Command>();
        if (cmd != nullptr)
        {
            this->operator()(cmd);
        }
    }

    void operator()(const DAVA::Command* command)
    {
        DAVA::CommandID_t commandId = command->GetID();
        if (commandId == DAVA::CMDID_BATCH)
        {
            const RECommandBatch* batch = static_cast<const RECommandBatch*>(command);
            for (DAVA::uint32 i = 0; i < batch->Size(); ++i)
            {
                this->operator()(batch->GetCommand(i));
            }
        }
        else
        {
            commandIds.insert(commandId);
        }
    }

private:
    DAVA::UnorderedSet<DAVA::CommandID_t>& commandIds;
};
}

class RECommandStack::ActiveCommandStack : public DAVA::StaticSingleton<ActiveCommandStack>
{
public:
    ActiveCommandStack()
    {
        envManager = NGTLayer::queryInterface<wgt::IEnvManager>();
        DVASSERT(envManager);
    }

    void CommandStackDeleted(RECommandStack* commandStack)
    {
        DVASSERT(commandStack != nullptr);
        if (activeStack == commandStack)
        {
            activeStack->DisableConnections();
            activeStack = nullptr;
        }
    }

    bool IsActive(const RECommandStack* commandStack)
    {
        return activeStack == commandStack;
    }

    void SetActive(RECommandStack* commandStack)
    {
        DVASSERT(activationStack.empty());
        DVASSERT(commandStack != nullptr);
        if (activeStack != nullptr)
        {
            activeStack->DisableConnections();
        }

        SetActiveImpl(commandStack);
        activeStack->EnableConections();
    }

    void PushCommandStack(RECommandStack* commandStack)
    {
        DVASSERT(commandStack != nullptr);

        bool sameStacks = activeStack == commandStack;
        if (activeStack != nullptr && sameStacks == false)
        {
            activeStack->DisableConnections();
        }

        activationStack.push(activeStack);
        SetActiveImpl(commandStack);

        if (sameStacks == false)
        {
            activeStack->EnableConections();
        }
    }

    void PopCommandStack()
    {
        DVASSERT(activeStack);
        DVASSERT(!activationStack.empty());
        RECommandStack* cmdStack = activationStack.top();
        activationStack.pop();
        bool sameStacks = cmdStack == activeStack;
        if (sameStacks == false)
        {
            activeStack->DisableConnections();
        }

        SetActiveImpl(cmdStack);

        if (activeStack != nullptr && sameStacks == false)
        {
            activeStack->EnableConections();
        }
    }

private:
    void SetActiveImpl(RECommandStack* cmdStack)
    {
        activeStack = cmdStack;
        if (activeStack)
        {
            envManager->selectEnv(activeStack->GetID());
        }
    }

private:
    RECommandStack* activeStack = nullptr;
    DAVA::Stack<RECommandStack*> activationStack;
    wgt::IEnvManager* envManager = nullptr;
};

class RECommandStack::ActiveStackGuard
{
public:
    ActiveStackGuard(const RECommandStack* commandStack)
    {
        ActiveCommandStack::Instance()->PushCommandStack(const_cast<RECommandStack*>(commandStack));
    }

    ~ActiveStackGuard()
    {
        ActiveCommandStack::Instance()->PopCommandStack();
    }
};

RECommandStack::RECommandStack()
    : CommandStack()
{
    indexChanged = commandManager->signalPostCommandIndexChanged.connect(std::bind(&RECommandStack::HistoryIndexChanged, this, std::placeholders::_1));
}

RECommandStack::~RECommandStack()
{
    DisconnectEvents();
    ActiveCommandStack::Instance()->CommandStackDeleted(this);
}

bool RECommandStack::CanUndo() const
{
    ActiveStackGuard guard(this);
    return CommandStack::CanUndo();
}

bool RECommandStack::CanRedo() const
{
    ActiveStackGuard guard(this);
    return CommandStack::CanRedo();
}

void RECommandStack::Clear()
{
    ActiveStackGuard guard(this);
    commandManager->removeCommands([this](const wgt::CommandInstancePtr&)
                                   {
                                       nextAfterCleanCommandIndex = DAVA::Max(nextAfterCleanCommandIndex - 1, EMPTY_INDEX);
                                       return true;
                                   });
    CleanCheck();
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

void RECommandStack::Push(DAVA::Command::Pointer&& command)
{
    ActiveStackGuard guard(this);
    if (command != nullptr)
    {
        uncleanCommandIds.insert(command->GetID());
    }
    CommandStack::Push(std::move(command));
}

bool RECommandStack::IsUncleanCommandExists(DAVA::CommandID_t commandId) const
{
    return uncleanCommandIds.count(commandId) > 0;
}

void RECommandStack::EndMacro()
{
    ActiveStackGuard guard(this);
    CommandStack::EndMacro();
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

    const wgt::VariantList& history = commandManager->getHistory();
    DVASSERT(endCommandIndex < static_cast<DAVA::int32>(history.size()));
    for (DAVA::int32 i = startCommandIndex; i <= endCommandIndex; ++i)
    {
        wgt::CommandInstancePtr instance = history[i].value<wgt::CommandInstancePtr>();
        DAVA::Command* cmd = instance->getArguments().getBase<DAVA::Command>();

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
    const wgt::VariantList& history = commandManager->getHistory();
    CommandStackLocal::CommandIdsAccumulator functor(uncleanCommandIds);
    DAVA::int32 historySize = static_cast<DAVA::int32>(history.size());
    DVASSERT(nextAfterCleanCommandIndex < historySize);
    for (DAVA::int32 i = DAVA::Max(nextAfterCleanCommandIndex, 0); i < historySize; ++i)
    {
        functor(history[i].value<wgt::CommandInstancePtr>());
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
        RECommand* reCommand = DAVA::DynamicTypeCheck<RECommand*>(cmd);
        EmitNotify(reCommand, operation != wgt::CommandOperation::UNDO);
    }
}

void RECommandStack::HistoryIndexChanged(int currentIndex)
{
    nextCommandIndex = currentIndex;
    CleanCheck();
    EmitUndoRedoStateChanged();
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
