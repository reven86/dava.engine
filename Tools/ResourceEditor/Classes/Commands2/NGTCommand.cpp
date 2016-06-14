#include "NGTCommand.h"
#include "Commands2/Base/Command2.h"

#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

#include <core_reflection/i_definition_manager.hpp>

const char* NGTCommand::getId() const
{
    return wgt::getClassIdentifier<NGTCommand>();
}

wgt::ObjectHandle NGTCommand::execute(const wgt::ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Execute();
    return wgt::CommandErrorCode::COMMAND_NO_ERROR;
}

wgt::CommandThreadAffinity NGTCommand::threadAffinity() const
{
    return wgt::CommandThreadAffinity::UI_THREAD;
}

bool NGTCommand::customUndo() const
{
    return true;
}

bool NGTCommand::canUndo(const wgt::ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    return command->CanUndo();
}

bool NGTCommand::undo(const wgt::ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    DVASSERT(command->CanUndo() == true);
    command->Undo();
    return true;
}

bool NGTCommand::redo(const wgt::ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

wgt::ObjectHandle NGTCommand::getCommandDescription(const wgt::ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    wgt::IDefinitionManager* defManager = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = wgt::GenericObject::create(*defManager);
    handle->set("Name", command->GetText());
    return wgt::ObjectHandle(std::move(handle));
}
