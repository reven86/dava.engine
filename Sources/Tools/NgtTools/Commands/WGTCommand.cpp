#include "Commands/WGTCommand.h"

#include "Debug/DVAssert.h"

#include "Command/Command.h"
#include "Command/CommandBatch.h"

#include "NgtTools/Common/GlobalContext.h"

#include <core_reflection/i_definition_manager.hpp>

const char* WGTCommand::getId() const
{
    return wgt::getClassIdentifier<WGTCommand>();
}

wgt::ObjectHandle WGTCommand::execute(const wgt::ObjectHandle& arguments) const
{
    DAVA::Command* command = arguments.getBase<DAVA::Command>();
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
    DAVA::Command* command = arguments.getBase<DAVA::Command>();
    DVASSERT(command != nullptr);
    return command->CanUndo();
}

bool WGTCommand::undo(const wgt::ObjectHandle& arguments) const
{
    DAVA::Command* command = arguments.getBase<DAVA::Command>();
    DVASSERT(command != nullptr);
    DVASSERT(command->CanUndo());
    command->Undo();
    return true;
}

bool WGTCommand::redo(const wgt::ObjectHandle& arguments) const
{
    DAVA::Command* command = arguments.getBase<DAVA::Command>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

wgt::ObjectHandle WGTCommand::getCommandDescription(const wgt::ObjectHandle& arguments) const
{
    DAVA::String text;
    DAVA::Command* command = arguments.getBase<DAVA::Command>();
    DVASSERT(nullptr != command);
    DAVA::Command* qeCommand = dynamic_cast<DAVA::Command*>(command);
    if (qeCommand != nullptr)
    {
        text = qeCommand->GetText();
    }
    else
    {
        DAVA::CommandBatch* batch = dynamic_cast<DAVA::CommandBatch*>(command);
        DVASSERT(nullptr != batch);
        text = batch->GetText();
    }

    wgt::IDefinitionManager* defManager = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = wgt::GenericObject::create(*defManager);
    handle->set("Name", text);
    return wgt::ObjectHandle(std::move(handle));
}
