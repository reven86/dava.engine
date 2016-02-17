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
#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandBatch.h"

class CommandStack : public CommandNotifyProvider, public CommandNotify
{
public:
    CommandStack();
    ~CommandStack() override;

    bool CanRedo() const;
    bool CanUndo() const;

    void Clear();
    void RemoveCommands(DAVA::int32 commandId);

    void Undo();
    void Redo();
    void Exec(std::unique_ptr<Command2>&& command);

    void BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount);
    void EndBatch();

    bool IsClean() const;
    void SetClean(bool clean);

    DAVA::int32 GetCleanIndex() const;
    DAVA::int32 GetNextIndex() const;

    DAVA::int32 GetUndoLimit() const;
    void SetUndoLimit(DAVA::int32 limit);

    DAVA::uint32 GetCount() const;
    const Command2* GetCommand(DAVA::int32 index) const;

private:
    //CommandNotify
    void Notify(const Command2* command, bool redo) override;

    using CommandsContainer = DAVA::List<std::unique_ptr<Command2>>;

    void ExecInternal(std::unique_ptr<Command2>&& command, bool runCommand);
    Command2* GetCommandInternal(DAVA::int32 index) const;

    void ClearRedoCommands();
    void ClearLimitedCommands();

    void CleanCheck();
    void CommandExecuted(const Command2* command, bool redo);

private:
    CommandsContainer commandList;
    std::unique_ptr<CommandBatch> curBatchCommand;

    DAVA::uint32 nestedBatchesCounter = 0;
    DAVA::int32 commandListLimit = 0;
    DAVA::int32 nextCommandIndex = 0;
    DAVA::int32 cleanCommandIndex = 0;
    bool lastCheckCleanState = true;
};


#endif // __COMMAND_STACK_H__
