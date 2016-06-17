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

    int GetID() const;

    DAVA::Signal<bool> cleanChanged;

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
};
