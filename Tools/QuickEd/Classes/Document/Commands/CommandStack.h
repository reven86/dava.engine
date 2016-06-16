#pragma once

#include "Document/Commands/Command.h"
#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

class ICommandManager;
class CommandBatch;

class CommandStack
{
public:
    CommandStack();

    void Push(QECommand::CommandPtr&& command);
    void BeginMacro(const DAVA::String& name);
    void EndMacro();

    bool IsClean() const;
    void SetCleanIndex(int cleanIndex);

    int GetID() const;

    DAVA::Signal<bool> cleanChanged;

private:
    void OnHistoryIndexChanged(int currentIndex);

    int cleanIndex = -1;
    ICommandManager* commandManager = nullptr;
    CommandBatch* currentBatch = nullptr;
    int ID = 0;
};
