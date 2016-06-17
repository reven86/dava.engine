#include "Document/CommandsBase/CommandStack.h"
#include "Document/CommandsBase/CommandBatch.h"
#include "Document/CommandsBase/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_env_system.hpp>

CommandStack::CommandStack()
{
    commandManager = NGTLayer::queryInterface<ICommandManager>();
    DVASSERT(commandManager != nullptr);

    indexChanged = commandManager->signalPostCommandIndexChanged.connect(std::bind(&CommandStack::OnHistoryIndexChanged, this, std::placeholders::_1));

    IEnvManager* envManager = NGTLayer::queryInterface<IEnvManager>();
    DVASSERT(envManager != nullptr);
    ID = envManager->addEnv("");
}

CommandStack::~CommandStack()
{
    IEnvManager* envManager = NGTLayer::queryInterface<IEnvManager>();
    DVASSERT(envManager != nullptr);
    envManager->removeEnv(ID);

    DVASSERT(!indexChanged.enabled());
    indexChanged.disconnect();
}

void CommandStack::Push(QECommand::CommandPtr&& command)
{
    if (!batches.empty())
    {
        batches.top()->AddAndRedo(std::move(command));
    }
    else
    {
        commandManager->queueCommand(getClassIdentifier<WGTCommand>(), ObjectHandle(std::move(command)));
    }
}

void CommandStack::BeginMacro(const DAVA::String& name)
{
    batches.push(new CommandBatch(name));
}

void CommandStack::EndMacro()
{
    DVASSERT(!batches.empty() && "CommandStack::EndMacro called without BeginMacro");
    CommandBatch* batch = batches.top();
    batches.pop();
    if (batches.empty())
    {
        QECommand::CommandPtr commandPtr(batch);
        commandManager->queueCommand(getClassIdentifier<WGTCommand>(), ObjectHandle(std::move(commandPtr)));
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

int CommandStack::GetID() const
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

void CommandStack::OnHistoryIndexChanged(int currentIndex)
{
    cleanChanged.Emit(IsClean());
}
