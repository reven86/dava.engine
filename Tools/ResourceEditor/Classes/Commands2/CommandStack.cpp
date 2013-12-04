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



#include "Commands2/CommandStack.h"
#include "Commands2/CommandAction.h"

CommandStack::CommandStack()
	: commandListLimit(0)
	, nextCommandIndex(0)
	, cleanCommandIndex(0)
	, lastCheckCleanState(true)
	, curBatchCommand(NULL)
{
	stackCommandsNotify = new CommandStackNotify(this);
}

CommandStack::~CommandStack()
{
	Clear();
	SafeRelease(stackCommandsNotify);
}

bool CommandStack::CanUndo() const
{
	bool ret = false;

	if(commandList.size() > 0 && nextCommandIndex > 0)
	{
		ret = true;
	}

	return ret;
}

bool CommandStack::CanRedo() const
{
	bool ret = false;

	if(nextCommandIndex < commandList.size())
	{
		ret = true;
	}

	return ret;
}

void CommandStack::Clear()
{
	nextCommandIndex = 0;
	cleanCommandIndex = 0;
	
	std::list<Command2 *>::iterator i = commandList.begin();
	std::list<Command2 *>::iterator end = commandList.end();

	for(; i != end; ++i)
	{
		delete *i;
	}

	commandList.clear();

	CleanCheck();
}

void CommandStack::Clear(int commandId)
{
	for(size_t i = 0; i < commandList.size(); i++)
	{
		Command2 *cmd = GetCommandInternal(i);

		if(cmd->GetId() == commandId)
		{
			ClearCommand(i);

			i--; // check command with same index on next step
		}
		else if(cmd->GetId() == CMDID_BATCH)
		{
			CommandBatch *batch = (CommandBatch *) cmd;

			batch->Clear(commandId);
			if(batch->Size() == 0)
			{
				// clear empty batch
				ClearCommand(i);

				i--; // check command with same index on next step
			}
		}
	}
}

void CommandStack::Undo()
{
	if(CanUndo())
	{
		nextCommandIndex--;
		Command2* commandToUndo = GetCommandInternal(nextCommandIndex);

		if(NULL != commandToUndo)
		{
			commandToUndo->Undo();
			EmitNotify(commandToUndo, false);
		}
	}

	CleanCheck();
}

void CommandStack::Redo()
{
	if(CanRedo())
	{
		Command2* commandToRedo = GetCommandInternal(nextCommandIndex);
		nextCommandIndex++;

		if(NULL != commandToRedo)
		{
			commandToRedo->Redo();
			EmitNotify(commandToRedo, true);
		}
	}

	CleanCheck();
}

void CommandStack::Exec(Command2 *command)
{
	if(NULL != command)
	{
		CommandAction* action = dynamic_cast<CommandAction*>(command);
		if (!action)
		{
			if(NULL != curBatchCommand)
			{
				curBatchCommand->AddAndExec(command);
			}
			else
			{
				ExecInternal(command, true);
			}
		}
		else
		{
			action->Redo();
			EmitNotify(command, true);
			delete action;
		}
	}
}

void CommandStack::BeginBatch(const DAVA::String &text)
{
	DVASSERT(NULL == curBatchCommand);

	if(NULL == curBatchCommand)
	{
		curBatchCommand = new CommandBatch();
		curBatchCommand->SetText(text);
		curBatchCommand->SetNotify(stackCommandsNotify);
	}
}

void CommandStack::EndBatch()
{
	if(NULL != curBatchCommand)
	{
		if(curBatchCommand->Size() > 0)
		{
			// all command were already executed in batch
			// so just add them to stack without calling redo
			ExecInternal(curBatchCommand, false);
		}
		else
		{
			delete curBatchCommand;
		}

		curBatchCommand = NULL;
	}
}

bool CommandStack::IsClean() const
{
	return (cleanCommandIndex == nextCommandIndex);
}

void CommandStack::SetClean(bool clean)
{
	if(clean)
	{
		cleanCommandIndex = nextCommandIndex;
	}
	else
	{
		cleanCommandIndex = -1;
	}

	CleanCheck();
}

size_t CommandStack::GetCleanIndex() const
{
	return cleanCommandIndex;
}

size_t CommandStack::GetNextIndex() const
{
	return nextCommandIndex;
}

size_t CommandStack::GetUndoLimit() const
{
	return commandListLimit;
}

void CommandStack::SetUndoLimit(size_t limit)
{
	commandListLimit = limit;
}

size_t CommandStack::GetCount() const
{
	return commandList.size();
}

const Command2* CommandStack::GetCommand(size_t index) const
{
	return GetCommandInternal(index);
}

Command2* CommandStack::GetCommandInternal(size_t index) const
{
	Command2* command = NULL;

	if(index < commandList.size())
	{
		std::list<Command2 *>::const_iterator i = commandList.begin();
		std::advance(i, index);

		if(i != commandList.end())
		{
			command = *i;
		}
	}

	return command;
}

void CommandStack::ExecInternal(Command2 *command, bool runCommand)
{
	ClearRedoCommands();

	commandList.push_back(command);
	nextCommandIndex++;

	if(runCommand)
	{
		command->SetNotify(stackCommandsNotify);
		command->Redo();
	}

	EmitNotify(command, true);
	ClearLimitedCommands();

	CleanCheck();
}

void CommandStack::ClearRedoCommands()
{
	if(CanRedo())
	{
		std::list<Command2 *>::iterator i = commandList.begin();
		std::advance(i, nextCommandIndex);
		while(i != commandList.end())
		{
			delete *i;
			i = commandList.erase(i);
		}
	}
}

void CommandStack::ClearLimitedCommands()
{
	while(commandListLimit > 0 && commandList.size() > commandListLimit)
	{
		ClearCommand(0);
	}
}

void CommandStack::ClearCommand(size_t index)
{
	const Command2 *command = GetCommand(index);
	if(NULL != command)
	{
		commandList.remove((Command2 *) command);

		if(nextCommandIndex > 0)
		{
			nextCommandIndex--;
		}

		if(cleanCommandIndex > 0 && index < cleanCommandIndex)
		{
			cleanCommandIndex--;
		}

		delete command;
	}

	CleanCheck();
}

void CommandStack::CleanCheck()
{
	if(lastCheckCleanState != IsClean())
	{
		lastCheckCleanState = IsClean();
		EmitCleanChanged(lastCheckCleanState);
	}
}

void CommandStack::CommandExecuted(const Command2 *command, bool redo)
{
	EmitNotify(command, redo);
}

CommandStackNotify::CommandStackNotify(CommandStack *_stack)
	: stack(_stack)
{ }

void CommandStackNotify::Notify(const Command2 *command, bool redo)
{
	if(NULL != stack)
	{
		stack->CommandExecuted(command, redo);
	}
}
