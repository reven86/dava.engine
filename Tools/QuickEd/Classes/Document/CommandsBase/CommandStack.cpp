#include "Document/CommandsBase/CommandStack.h"
#include "Document/CommandsBase/CommandBatch.h"
#include "Document/CommandsBase/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_env_system.hpp>

CommandStack::CommandStack()
{
    commandManager = NGTLayer::queryInterface<wgt::ICommandManager>();
    DVASSERT(commandManager != nullptr);

    indexChanged = commandManager->signalPostCommandIndexChanged.connect(std::bind(&CommandStack::OnHistoryIndexChanged, this, std::placeholders::_1));

    wgt::IEnvManager* envManager = NGTLayer::queryInterface<wgt::IEnvManager>();
    DVASSERT(envManager != nullptr);
    ID = envManager->addEnv("");
}

CommandStack::~CommandStack()
{
    wgt::IEnvManager* envManager = NGTLayer::queryInterface<wgt::IEnvManager>();
    DVASSERT(envManager != nullptr);
    envManager->removeEnv(ID);

    indexChanged.disconnect();
}

void CommandStack::Push(Command::CommandPtr&& command)
{
    if (!batches.empty())
    {
        batches.top()->AddAndRedo(std::move(command));
    }
    else
    {
        commandManager->queueCommand(wgt::getClassIdentifier<WGTCommand>(), wgt::ObjectHandle(std::move(command)));
    }
}

void CommandStack::BeginMacro(const DAVA::String& name)
{
    batches.push(std::make_unique<CommandBatch>(name));
}

void CommandStack::EndMacro()
{
    DVASSERT(!batches.empty() && "CommandStack::EndMacro called without BeginMacro");
    std::unique_ptr<CommandBatch> batch(std::move(batches.top()));
    batches.pop();
    if (batches.empty())
    {
        if (!batch->IsEmpty())
        {
            commandManager->queueCommand(wgt::getClassIdentifier<WGTCommand>(), wgt::ObjectHandle(std::move(batch)));
        }
    }
}

bool CommandStack::IsClean() const
{
    return commandManager->commandIndex() == cleanIndex;
}

void CommandStack::SetClean()
{
    cleanIndex = commandManager->commandIndex();
    cleanChanged.Emit(true);
}

void CommandStack::Undo()
{
    DVASSERT(commandManager->canUndo());
    commandManager->undo();
}

void CommandStack::Redo()
{
    DVASSERT(commandManager->canRedo());
    commandManager->redo();
}

bool CommandStack::CanUndo() const
{
    return commandManager->canUndo();
}

bool CommandStack::CanRedo() const
{
    return commandManager->canRedo();
}

DAVA::int32 CommandStack::GetID() const
{
    return ID;
}

void CommandStack::DisconnectFromCommandManager()
{
    indexChanged.disable();
}

void CommandStack::ConnectToCommandManager()
{
    indexChanged.enable();
}

void CommandStack::OnHistoryIndexChanged(int /*currentIndex*/)
{
    SetClean(IsClean());
    SetCanUndo(CanUndo());
    SetCanRedo(CanRedo());
}

void CommandStack::SetClean(bool isClean_)
{
    if (isClean != isClean_)
    {
        isClean = isClean_;
        cleanChanged.Emit(isClean);
    }
}

void CommandStack::SetCanUndo(bool canUndo_)
{
    if (canUndo != canUndo_)
    {
        canUndo = canUndo_;
        canUndoChanged.Emit(canUndo);
    }
}

void CommandStack::SetCanRedo(bool canRedo_)
{
    if (canRedo != canRedo_)
    {
        canRedo = canRedo_;
        canRedoChanged.Emit(canRedo);
    }
}
