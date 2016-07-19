#include "NgtTools/Commands/CommandStack.h"

#include "Command/CommandBatch.h"

#include "NgtTools/Commands/WGTCommand.h"

#include "NgtTools/Common/GlobalContext.h"

#include <core_command_system/i_command_manager.hpp>
#include <core_command_system/i_env_system.hpp>
#include <core_data_model/variant_list.hpp>

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

void CommandStack::Exec(DAVA::Command::Pointer&& command)
{
    DVASSERT(command != nullptr);
    if (command == nullptr)
    {
        return;
    }
    if (rootBatch != nullptr)
    {
        //when the macro started command without undo will cause undefined state of application
        DVASSERT_MSG(command->CanUndo(),
                     DAVA::Format("Command %s, which can not make undo passed to CommandStack within macro %s",
                                  command->GetDescription().c_str(),
                                  batchesStack.top()->GetDescription().c_str())
                     .c_str()
                     );
        if (command->CanUndo())
        {
            batchesStack.top()->AddAndExec(std::move(command));
            return;
        }
    }
    commandManager->queueCommand(wgt::getClassIdentifier<WGTCommand>(), wgt::ObjectHandle(std::move(command)));
}

void CommandStack::BeginBatch(const DAVA::String& name, DAVA::uint32 commandsCount)
{
    //we call BeginMacro first time
    DAVA::Command::Pointer newCommandBatch = DAVA::Command::Create<DAVA::CommandBatch>(name, commandsCount);
    DAVA::CommandBatch* newCommandBatchPtr = static_cast<DAVA::CommandBatch*>(newCommandBatch.get());
    if (rootBatch == nullptr)
    {
        DVASSERT(batchesStack.empty());
        rootBatch = std::move(newCommandBatch);
    }
    //we already create one or more batches
    else
    {
        batchesStack.top()->AddAndExec(std::move(newCommandBatch));
    }
    batchesStack.push(newCommandBatchPtr);
}

void CommandStack::EndBatch()
{
    DVASSERT(rootBatch != nullptr && "CommandStack::EndMacro called without BeginMacro");
    if (batchesStack.size() == 1)
    {
        DVASSERT(rootBatch != nullptr);
        DAVA::CommandBatch* rootBatchPtr = static_cast<DAVA::CommandBatch*>(rootBatch.get());
        if (!rootBatchPtr->IsEmpty())
        {
            commandManager->queueCommand(wgt::getClassIdentifier<WGTCommand>(), wgt::ObjectHandle(std::move(rootBatch)));
        }
    }
    if (!batchesStack.empty())
    {
        batchesStack.pop();
    }
}

bool CommandStack::IsClean() const
{
    return isClean;
}

void CommandStack::SetClean()
{
    int commandIndex = commandManager->commandIndex();
    cleanIndex = commandIndex;
    UpdateCleanState();
}

void CommandStack::Undo()
{
    DVASSERT(CanUndo());
    if (CanUndo())
    {
        commandManager->undo();
    }
}

void CommandStack::Redo()
{
    DVASSERT(commandManager->canRedo());
    if (CanRedo())
    {
        commandManager->redo();
    }
}

bool CommandStack::CanUndo() const
{
    return CanUndoImpl();
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

bool CommandStack::CanUndoImpl() const
{
    return commandManager->canUndo();
}

bool CommandStack::CanRedoImpl() const
{
    return commandManager->canRedo();
}

void CommandStack::OnHistoryIndexChanged(int currentIndex)
{
    UpdateCleanState();
    SetCanUndo(CanUndoImpl());
    SetCanRedo(CanRedoImpl());
}

void CommandStack::UpdateCleanState()
{
    int currentIndex = commandManager->commandIndex();
    int begin = std::min(cleanIndex, currentIndex);
    int end = std::max(cleanIndex, currentIndex);
    DVASSERT(end >= begin);
    bool containsModifiedCommands = false;
    const wgt::VariantList& commandHistory = commandManager->getHistory();
    for (int commandIndex = begin; commandIndex != end && !containsModifiedCommands; ++commandIndex)
    {
        wgt::Variant var = commandHistory[commandIndex + 1]; //wgt command index starts from -1
        if (var.typeIs<wgt::ObjectHandle>())
        {
            wgt::ObjectHandle objectHandle = var.value<wgt::ObjectHandle>();
            wgt::CommandInstance* commandInstance = var.value<wgt::ObjectHandle>().getBase<wgt::CommandInstance>();
            DVASSERT(commandInstance != nullptr);
            DAVA::Command* command = commandInstance->getArguments().getBase<DAVA::Command>();
            containsModifiedCommands |= command->IsModifying();
        }
    }
    SetClean(!containsModifiedCommands);
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
