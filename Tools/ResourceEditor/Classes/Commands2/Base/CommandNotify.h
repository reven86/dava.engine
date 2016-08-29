#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

class RECommandNotificationObject;
class CommandNotify : public DAVA::BaseObject
{
public:
    virtual void Notify(const RECommandNotificationObject& commandNotification) = 0;
    virtual void CleanChanged(bool clean);
    virtual void CanUndoChanged(bool canUndo);
    virtual void CanRedoChanged(bool canRedo);
    virtual void UndoTextChanged(const DAVA::String& undoText);
    virtual void RedoTextChanged(const DAVA::String& redoText);
};

class CommandNotifyProvider
{
public:
    virtual ~CommandNotifyProvider();

    void SetNotify(CommandNotify* notify);
    CommandNotify* GetNotify() const;

    void EmitNotify(const RECommandNotificationObject& commandNotification);
    void EmitCleanChanged(bool clean);
    void CanUndoChanged(bool canUndo);
    void CanRedoChanged(bool canRedo);
    void UndoTextChanged(const DAVA::String& undoText);
    void RedoTextChanged(const DAVA::String& redoText);

protected:
    CommandNotify* curNotify = nullptr;
};

inline CommandNotify* CommandNotifyProvider::GetNotify() const
{
    return curNotify;
}
