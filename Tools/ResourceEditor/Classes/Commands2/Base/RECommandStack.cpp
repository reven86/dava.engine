#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

#include "Commands2/Base/RECommandNotificationObject.h"

RECommandStack::RECommandStack()
    : DAVA::CommandStack()
{
    canRedoChanged.Connect(this, &RECommandStack::CanRedoChanged);
    canUndoChanged.Connect(this, &RECommandStack::CanUndoChanged);
    undoTextChanged.Connect(this, &RECommandStack::UndoTextChanged);
    redoTextChanged.Connect(this, &RECommandStack::RedoTextChanged);
    cleanChanged.Connect(this, &RECommandStack::EmitCleanChanged);
    currentIndexChanged.Connect(this, &RECommandStack::CurrentIndexChanged);
}

RECommandStack::~RECommandStack() = default;

void RECommandStack::Clear()
{
    commands.clear();
    cleanIndex = EMPTY_INDEX;
    SetCurrentIndex(EMPTY_INDEX);
}

void RECommandStack::SetClean(bool clean)
{
    if (clean)
    {
        //call public "SetClean" implementation
        CommandStack::SetClean();
    }
    else
    {
        //call private "SetClean" implementation, which only set "clean" flag to false
        CommandStack::SetClean(false);
    }
}

void RECommandStack::RemoveCommands(DAVA::uint32 commandId)
{
    for (DAVA::int32 index = static_cast<DAVA::int32>(commands.size() - 1); index >= 0; --index)
    {
        DAVA::Command* commandPtr = commands.at(index).get();
        if (DAVA::IsCommandBatch(commandPtr))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(commandPtr);
            batch->RemoveCommands(commandId);
            if (batch->IsEmpty())
            {
                RemoveCommand(index);
            }
        }
        else
        {
            const RECommand* reCommand = static_cast<const RECommand*>(commandPtr);
            if (reCommand->GetID() == commandId)
            {
                RemoveCommand(index);
            }
        }
    }
}

void RECommandStack::Activate()
{
    canUndoChanged.Emit(CanUndo());
    canRedoChanged.Emit(CanRedo());
}

bool RECommandStack::IsUncleanCommandExists(DAVA::uint32 commandId) const
{
    DAVA::uint32 size = static_cast<DAVA::uint32>(commands.size());
    for (DAVA::uint32 index = std::max(cleanIndex, 0); index < size; ++index)
    {
        const DAVA::Command* commandPtr = commands.at(index).get();
        if (IsCommandBatch(commandPtr) == false)
        {
            const RECommand* reCommandPtr = static_cast<const RECommand*>(commandPtr);
            if (reCommandPtr->MatchCommandID(commandId))
            {
                return true;
            }
        }
    }
    return false;
}

DAVA::CommandBatch* RECommandStack::CreateCommmandBatch(const DAVA::String& name, DAVA::uint32 commandsCount) const
{
    return new RECommandBatch(name, commandsCount);
}

void RECommandStack::RemoveCommand(DAVA::uint32 index)
{
    DVASSERT(index < commands.size());
    if (cleanIndex > static_cast<DAVA::int32>(index))
    {
        cleanIndex--;
    }
    commands.erase(commands.begin() + index);
    if (currentIndex > static_cast<DAVA::int32>(index))
    {
        SetCurrentIndex(currentIndex - 1);
    }
}

void RECommandStack::CurrentIndexChanged(DAVA::int32 newIndex, DAVA::int32 oldIndex)
{
    if ((newIndex >= 0 || oldIndex >= 0) && (newIndex < static_cast<DAVA::int32>(commands.size())))
    {
        DAVA::int32 commandIndex = DAVA::Max(newIndex, oldIndex);
        DAVA::Command* cmd = commands[commandIndex].get();

        RECommandNotificationObject notification;
        if (DAVA::IsCommandBatch(cmd))
        {
            notification.batch = static_cast<RECommandBatch*>(cmd);
        }
        else
        {
            notification.command = static_cast<RECommand*>(cmd);
        }
        notification.redo = (newIndex > oldIndex);
        EmitNotify(notification);
    }
    else if (currentIndex != EMPTY_INDEX)
    {
        DVASSERT_MSG(false, DAVA::Format("Commands changed to wrong index(%d)", newIndex).c_str());
    }
}

void RECommandStack::ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand)
{
    if (IsCommandAction(command.get()))
    {
        //get ownership of the given command;
        std::unique_ptr<Command> commandCopy(std::move(command));
        commandCopy->Redo();
        if (!commandCopy->IsClean())
        {
            RECommandStack::SetClean(false);
        }
    }
    else
    {
        CommandStack::ExecInternal(std::move(command), isSingleCommand);
    }
}
