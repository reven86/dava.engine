#include "NGTCommand.h"
#include "Commands2/Base/Command2.h"

#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

const char* NGTCommand::getId() const
{
    return getClassIdentifier<NGTCommand>();
}

ObjectHandle NGTCommand::execute(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Execute();
    return CommandErrorCode::COMMAND_NO_ERROR;
}

CommandThreadAffinity NGTCommand::threadAffinity() const
{
    return CommandThreadAffinity::UI_THREAD;
}

bool NGTCommand::customUndo() const
{
    return true;
}

bool NGTCommand::canUndo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    return command->CanUndo();
}

bool NGTCommand::undo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    DVASSERT(command->CanUndo() == true);
    command->Undo();
    return true;
}

bool NGTCommand::redo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

ObjectHandle NGTCommand::getCommandDescription(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    IDefinitionManager* defManager = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = GenericObject::create(*defManager);
    handle->set("Name", command->GetText());
    return ObjectHandle(std::move(handle));
}
