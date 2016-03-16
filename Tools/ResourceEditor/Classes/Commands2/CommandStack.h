/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __COMMAND_STACK_H__
#define __COMMAND_STACK_H__

#include "Base/BaseTypes.h"
#include "Commands2/Command2.h"
#include "Commands2/CommandBatch.h"

class CommandStackNotify;

class CommandStack : public CommandNotifyProvider
{
public:
    CommandStack();
    ~CommandStack();

    bool CanRedo() const;
    bool CanUndo() const;

    void Clear();
    void Clear(int commandId);

    void Undo();
    void Redo();
    void Exec(Command2* command);

    void BeginBatch(const DAVA::String& text);
    void EndBatch();
    bool IsBatchStarted() const;

    bool IsClean() const;
    void SetClean(bool clean);

    size_t GetCleanIndex() const;
    size_t GetNextIndex() const;

    size_t GetUndoLimit() const;
    void SetUndoLimit(size_t limit);

    size_t GetCount() const;
    const Command2* GetCommand(size_t index) const;

private:
    friend class CommandStackNotify;

    void ExecInternal(Command2* command, bool runCommand);
    Command2* GetCommandInternal(size_t index) const;

    void ClearRedoCommands();
    void ClearLimitedCommands();
    void ClearCommand(size_t index);

    void CleanCheck();
    void CommandExecuted(const Command2* command, bool redo);

private:
    const size_t INVALID_CLEAN_INDEX = static_cast<size_t>(-1);

    DAVA::List<Command2*> commandList;
    DAVA::ScopedPtr<CommandStackNotify> stackCommandsNotify;
    CommandBatch* curBatchCommand = nullptr;
    size_t commandListLimit = 0;
    size_t nextCommandIndex = 0;
    size_t cleanCommandIndex = 0;
    DAVA::uint32 nestedBatchesCounter = 0;
    bool lastCheckCleanState = true;
};

class CommandStackNotify : public CommandNotify
{
public:
    CommandStackNotify(CommandStack* _stack);
    void Notify(const Command2* command, bool redo) override;

private:
    CommandStack* stack = nullptr;
};

#endif // __COMMAND_STACK_H__
