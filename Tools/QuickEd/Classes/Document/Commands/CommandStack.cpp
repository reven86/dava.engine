#include "Document/Commands/CommandStack.h"
#include "Document/Commands/CommandBatch.h"
#include "Document/Commands/WGTCommand.h"
#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_env_system.hpp>

CommandStack::CommandStack()
{
    commandManager = NGTLayer::queryInterface<ICommandManager>();
    DVASSERT(commandManager != nullptr);

    commandManager->signalPostCommandIndexChanged.connect(std::bind(&CommandStack::OnHistoryIndexChanged, this, std::placeholders::_1));

    IEnvManager* envManager = NGTLayer::queryInterface<IEnvManager>();
    DVASSERT(envManager != nullptr);
    ID = envManager->addEnv("");
}

void CommandStack::Push(QECommand::CommandPtr&& command)
{
    if (currentBatch != nullptr)
    {
        currentBatch->AddAndRedo(std::move(command));
    }
    else
    {
        commandManager->queueCommand(getClassIdentifier<WGTCommand>(), ObjectHandle(std::move(command)));
    }
}

void CommandStack::BeginMacro(const DAVA::String& name)
{
    DVASSERT(currentBatch == nullptr && "CommandStack::BeginMacro called without EndMacro");
    currentBatch = new CommandBatch(name);
}

void CommandStack::EndMacro()
{
    DVASSERT(currentBatch != nullptr && "CommandStack::EndMacro called without BeginMacro");
    QECommand::CommandPtr commandPtr(currentBatch);
    commandManager->queueCommand(getClassIdentifier<WGTCommand>(), ObjectHandle(commandPtr));
}

bool CommandStack::IsClean() const
{
    return commandManager->commandIndex() == cleanIndex;
}

void CommandStack::SetCleanIndex(int cleanIndex_)
{
    cleanIndex = cleanIndex_;
    cleanChanged.Emit(IsClean());
}

int CommandStack::GetID() const
{
    return ID;
}

void CommandStack::OnHistoryIndexChanged(int currentIndex)
{
    cleanChanged.Emit(IsClean());
}
