#pragma once

#include "Command/Command.h"
#include "Functional/Signal.h"

#include <core_common/signal.hpp>

namespace DAVA
{
class CommandBatch;
}
namespace wgt
{
class ICommandManager;
}

class CommandStack
{
public:
    CommandStack();
    virtual ~CommandStack();

    virtual void Push(DAVA::Command::Pointer&& command);

    virtual void BeginMacro(const DAVA::String& name, DAVA::uint32 commandsCount = 0);
    virtual void EndMacro();

    virtual bool IsClean() const;
    virtual void SetClean();

    virtual void Undo();
    virtual void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    DAVA::int32 GetID() const;

    DAVA::Signal<bool> cleanChanged;
    DAVA::Signal<bool> canUndoChanged;
    DAVA::Signal<bool> canRedoChanged;

    void DisconnectFromCommandManager();
    void ConnectToCommandManager();

protected:
    wgt::ICommandManager* commandManager = nullptr;

private:
    void OnHistoryIndexChanged(int currentIndex);
    void UpdateCleanState();

    DAVA::int32 cleanIndex = -1;
    //enviroinment id given from envManager
    DAVA::int32 ID = 0;
    wgt::Connection indexChanged;
    //we own only first item and than move it to the command manager
    DAVA::Stack<DAVA::CommandBatch*> batchesStack;

    //members to remember stack state and do not emit extra signals
    bool isClean = true;
    bool canUndo = false;
    bool canRedo = false;

    void SetClean(bool isClean);
    void SetCanUndo(bool canUndo);
    void SetCanRedo(bool canRedo);
};
