#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
class Command;
}

class CommandNotify : public DAVA::BaseObject
{
public:
    virtual void Notify(const DAVA::Command* command, bool redo) = 0;
    virtual void CleanChanged(bool clean);
    virtual void UndoRedoStateChanged();
};

class CommandNotifyProvider
{
public:
    virtual ~CommandNotifyProvider();

    void SetNotify(CommandNotify* notify);
    CommandNotify* GetNotify() const;

    void EmitNotify(const DAVA::Command* command, bool redo);
    void EmitCleanChanged(bool clean);
    void EmitUndoRedoStateChanged();

protected:
    CommandNotify* curNotify = nullptr;
};

inline CommandNotify* CommandNotifyProvider::GetNotify() const
{
    return curNotify;
}
