#include "Commands2/Base/CommandNotify.h"

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

void CommandNotifyProvider::EmitNotify(const Command2* command, bool redo)
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
