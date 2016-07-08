#include "Commands2/Base/CommandNotify.h"

void CommandNotify::CleanChanged(bool /*clean*/)
{
}

void CommandNotify::UndoRedoStateChanged()
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

void CommandNotifyProvider::EmitNotify(const DAVA::Command* command, bool redo)
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

void CommandNotifyProvider::EmitUndoRedoStateChanged()
{
    if (nullptr != curNotify)
    {
        curNotify->UndoRedoStateChanged();
    }
}
