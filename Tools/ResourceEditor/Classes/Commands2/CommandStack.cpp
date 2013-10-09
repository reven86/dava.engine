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

void CommandStack::Undo()
{
	if(CanUndo())
	{
		nextCommandIndex--;
		Command2* commandToUndo = GetCommand(nextCommandIndex);

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
		Command2* commandToRedo = GetCommand(nextCommandIndex);
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

size_t CommandStack::GetUndoLimit() const
{
	return commandListLimit;
}

void CommandStack::SetUndoLimit(size_t limit)
{
	commandListLimit = limit;
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
		std::list<Command2 *>::iterator next;

		std::advance(i, nextCommandIndex);
		while(i != commandList.end())
		{
			next = i;
			next++;

			delete *i;
			commandList.erase(i);

			i = next;
		}
	}
}

void CommandStack::ClearLimitedCommands()
{
	while(commandListLimit > 0 && commandList.size() > commandListLimit)
	{
		Command2* command = GetCommand(0);

		// this is single command
		if(NULL != command)
		{
			delete command;
		}

		commandList.pop_front();

		nextCommandIndex--;
		cleanCommandIndex--;
	}
}

void CommandStack::CleanCheck()
{
	if(lastCheckCleanState != IsClean())
	{
		lastCheckCleanState = IsClean();
		EmitCleanChanged(lastCheckCleanState);
	}
}

Command2* CommandStack::GetCommand(size_t index)
{
	Command2* command = NULL;

	if(index < commandList.size())
	{
		std::list<Command2 *>::iterator i = commandList.begin();
		std::advance(i, index);

		if(i != commandList.end())
		{
			command = *i;
		}
	}

	return command;
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
