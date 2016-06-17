#include "Document/CommandsBase/WGTCommand.h"
#include "Document/CommandsBase/Command.h"
#include "Document/CommandsBase/CommandBatch.h"

#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

const char* WGTCommand::getId() const
{
    return wgt::getClassIdentifier<WGTCommand>();
}

wgt::ObjectHandle WGTCommand::execute(const wgt::ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Execute();
    return wgt::CommandErrorCode::COMMAND_NO_ERROR;
}

wgt::CommandThreadAffinity WGTCommand::threadAffinity() const
{
    return wgt::CommandThreadAffinity::UI_THREAD;
}

bool WGTCommand::customUndo() const
{
    return true;
}

bool WGTCommand::canUndo(const wgt::ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    return true;
}

bool WGTCommand::undo(const wgt::ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Undo();
    return true;
}

bool WGTCommand::redo(const wgt::ObjectHandle& arguments) const
{
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

wgt::ObjectHandle WGTCommand::getCommandDescription(const wgt::ObjectHandle& arguments) const
{
    DAVA::String text;
    DAVA::ICommand* command = arguments.getBase<DAVA::ICommand>();
    DVASSERT(nullptr != command);
    ::Command* qeCommand = dynamic_cast<::Command*>(command);
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

    wgt::IDefinitionManager* defManager = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = wgt::GenericObject::create(*defManager);
    handle->set("Name", text);
    return wgt::ObjectHandle(std::move(handle));
}
