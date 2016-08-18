#include "Commands2/Base/CommandNotify.h"

void CommandNotify::CleanChanged(bool /*clean*/)
{
}

void CommandNotify::CanUndoChanged(bool /*canUndo*/)
{
}

void CommandNotify::CanRedoChanged(bool /*canRedo*/)
{
}

void CommandNotify::UndoTextChanged(const DAVA::String& undoText)
{
}

void CommandNotify::RedoTextChanged(const DAVA::String& redoText)
{
}

CommandNotifyProvider::~CommandNotifyProvider()
{
    SafeRelease(curNotify);
}

void CommandNotifyProvider::SetNotify(CommandNotify* notify)
{
    if (curNotify != notify)
    {
        SafeRelease(curNotify);
        curNotify = SafeRetain(notify);
    }
}

void CommandNotifyProvider::EmitNotify(const RECommandNotificationObject& commandNotification)
{
    if (nullptr != curNotify)
    {
        curNotify->Notify(commandNotification);
    }
}

void CommandNotifyProvider::EmitCleanChanged(bool clean)
{
    if (nullptr != curNotify)
    {
        curNotify->CleanChanged(clean);
    }
}

void CommandNotifyProvider::CanUndoChanged(bool canUndo)
{
    if (nullptr != curNotify)
    {
        curNotify->CanUndoChanged(canUndo);
    }
}

void CommandNotifyProvider::CanRedoChanged(bool canRedo)
{
    if (nullptr != curNotify)
    {
        curNotify->CanRedoChanged(canRedo);
    }
}

void CommandNotifyProvider::UndoTextChanged(const DAVA::String& undoText)
{
    if (nullptr != curNotify)
    {
        curNotify->UndoTextChanged(undoText);
    }
}

void CommandNotifyProvider::RedoTextChanged(const DAVA::String& redoText)
{
    if (nullptr != curNotify)
    {
        curNotify->RedoTextChanged(redoText);
    }
}
