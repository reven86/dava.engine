#include "Document/CommandsBase/WGTCommand.h"
#include "Document/CommandsBase/Command.h"
#include "Document/CommandsBase/CommandBatch.h"

#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

const char* WGTCommand::getId() const
{
    return getClassIdentifier<WGTCommand>();
}

ObjectHandle WGTCommand::execute(const ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Execute();
    return CommandErrorCode::COMMAND_NO_ERROR;
}

CommandThreadAffinity WGTCommand::threadAffinity() const
{
    return CommandThreadAffinity::UI_THREAD;
}

bool WGTCommand::customUndo() const
{
    return true;
}

bool WGTCommand::canUndo(const ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    return true;
}

bool WGTCommand::undo(const ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Undo();
    return true;
}

bool WGTCommand::redo(const ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

ObjectHandle WGTCommand::getCommandDescription(const ObjectHandle& arguments) const
{
    DAVA::String text;
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(nullptr != command);
    QECommand* qeCommand = dynamic_cast<QECommand*>(command);
    if (qeCommand != nullptr)
    {
        text = qeCommand->GetText();
    }
    else
    {
        CommandBatch* batch = dynamic_cast<CommandBatch*>(command);
        DVASSERT(nullptr != batch);
        text = batch->GetText();
    }

    IDefinitionManager* defManager = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = GenericObject::create(*defManager);
    handle->set("Name", text);
    return ObjectHandle(std::move(handle));
}
