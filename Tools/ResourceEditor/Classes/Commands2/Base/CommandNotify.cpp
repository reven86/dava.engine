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

void CommandNotifyProvider::EmitNotify(const RECommand* command, bool redo)
{
    if (nullptr != curNotify)
    {
        curNotify->Notify(command, redo);
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
