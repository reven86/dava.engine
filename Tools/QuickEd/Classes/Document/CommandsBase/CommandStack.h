#pragma once

#include "Document/CommandsBase/Command.h"
#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

#include <core_common/signal.hpp>

namespace wgt
{
class ICommandManager;
}
class CommandBatch;

class CommandStack
{
public:
    CommandStack();
    ~CommandStack();
    void Push(Command::CommandPtr&& command);
    void BeginMacro(const DAVA::String& name);
    void EndMacro();

    bool IsClean() const;
    void SetClean();

    void Undo();
    void Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    int GetID() const;

    DAVA::Signal<bool> cleanChanged;
    DAVA::Signal<bool> canUndoChanged;
    DAVA::Signal<bool> canRedoChanged;

private:
    friend class CommandStackGroup;
    void DisconnectFromCommandManager();
    void ConnectToCommandManager();

    void OnHistoryIndexChanged(int currentIndex);

    int cleanIndex = -1;
    wgt::ICommandManager* commandManager = nullptr;
    int ID = 0;
    wgt::Connection indexChanged;
    DAVA::Stack<CommandBatch*> batches;

    //members to remember stack state and do not emit extra signals
    bool isClean = true;
    bool canUndo = false;
    bool canRedo = false;

    void SetClean(bool isClean);
    void SetCanUndo(bool canUndo);
    void SetCanRedo(bool canRedo);
};
