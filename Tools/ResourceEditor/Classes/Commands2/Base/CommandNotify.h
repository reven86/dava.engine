#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

class RECommand;

class CommandNotify : public DAVA::BaseObject
{
public:
    virtual void Notify(const RECommand* command, bool redo) = 0;
    virtual void CleanChanged(bool clean);
    virtual void CanUndoChanged(bool canUndo);
    virtual void CanRedoChanged(bool canRedo);
};

class CommandNotifyProvider
{
public:
    virtual ~CommandNotifyProvider();

    void SetNotify(CommandNotify* notify);
    CommandNotify* GetNotify() const;

    void EmitNotify(const RECommand* command, bool redo);
    void EmitCleanChanged(bool clean);
    void CanUndoChanged(bool canUndo);
    void CanRedoChanged(bool canRedo);

protected:
    CommandNotify* curNotify = nullptr;
};

inline CommandNotify* CommandNotifyProvider::GetNotify() const
{
    return curNotify;
}
