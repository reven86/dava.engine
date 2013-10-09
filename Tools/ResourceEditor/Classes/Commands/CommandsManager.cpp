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



#include "CommandsManager.h"
#include "Command.h"

#include "Scene3D/Scene.h"

#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Commands/CommandSignals.h"

using namespace DAVA;

CommandsManager::CommandsManager()
{
}

CommandsManager::~CommandsManager()
{
	ClearAllQueues();
}

void CommandsManager::ClearAllQueues()
{
	for (QUEUE_MAP::iterator it = queueMap.begin(); it != queueMap.end(); ++it)
	{
		ClearQueue((*it).second);
		SafeDelete((*it).second);
	}
	queueMap.clear();
}

void CommandsManager::ClearQueue(UndoQueue* queue)
{
	DVASSERT(queue);

	queue->commandIndex = -1;
	for_each(queue->commands.begin(),
			 queue->commands.end(),
			 SafeRelease<Command>);
	queue->commands.clear();
}

void CommandsManager::SceneReleased(DAVA::Scene* scene)
{
	QUEUE_MAP::iterator it = queueMap.find(scene);

	if (it != queueMap.end())
	{
		ClearQueue((*it).second);
		SafeDelete((*it).second);
		queueMap.erase(it);
	}
}

void CommandsManager::ClearQueueTail(UndoQueue* queue)
{
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
        DVASSERT(queue->commandIndex <= (UNDO_QUEUE_SIZE - 1)); //Queue cannot be more than UNDO_QUEUE_SIZE
        if(queue->commandIndex == UNDO_QUEUE_SIZE - 1)
        {
            SafeRelease(queue->commands[0]);
            queue->commands.erase(queue->commands.begin());

            --queue->commandIndex;
        }

		int32 newCount = queue->commandIndex + 1;
		for_each(queue->commands.begin() + newCount,
				 queue->commands.end(),
				 SafeRelease<Command>);
		queue->commands.resize(newCount);
	}
}


void CommandsManager::Execute(Command *command, DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

    command->Execute();

	if (command->State() == Command::STATE_VALID)
	{
		EmitCommandExecutedSignal(command, forScene);
	}

    switch (command->Type()) 
    {
        case Command::COMMAND_WITHOUT_UNDO_EFFECT:
            break;
            
        case Command::COMMAND_UNDO_REDO:
            
            if(Command::STATE_VALID == command->State())
            {
                //TODO: VK: if need only UNDO_QUEUE_SIZE commands at queue you may add code here
				ClearQueueTail(queue);
				queue->commands.push_back(SafeRetain(command));
				++queue->commandIndex;
            }
            break;

        case Command::COMMAND_CLEAR_UNDO_QUEUE:
            if(Command::STATE_VALID == command->State())
            {
                ClearQueue(queue);
            }
            break;

        default:
            Logger::Warning("[CommandsManager::Execute] command type (%d) not processed", command->Type());
            break;
    }

	// TODO: mainwindow
	//QtMainWindowHandler::Instance()->UpdateUndoActionsState();
}

void CommandsManager::ExecuteAndRelease(Command* command, DAVA::Scene* forScene)
{
	Execute(command, forScene);
	SafeRelease(command);
}

void CommandsManager::Undo(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= 0) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
		//TODO: need check state?
		Command* command = queue->commands[queue->commandIndex];
		command->Cancel();
		EmitCommandExecutedSignal(command, forScene);

		--queue->commandIndex;

		// TODO: mainwindow
		//QtMainWindowHandler::Instance()->UpdateUndoActionsState();
	}
}

void CommandsManager::Redo(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size() - 1))
	{
		//TODO: need check state?
		++queue->commandIndex;

		Command* command = queue->commands[queue->commandIndex];
		command->Execute();
		EmitCommandExecutedSignal(command, forScene);
	}
}

int32 CommandsManager::GetUndoQueueLength(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	return queue->commandIndex + 1;
}

int32 CommandsManager::GetRedoQueueLength(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	return queue->commands.size() - queue->commandIndex - 1;
}

String CommandsManager::GetUndoCommandName(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= 0) &&
		(queue->commandIndex < (int32)queue->commands.size()))
	{
		return queue->commands[queue->commandIndex]->commandName;
	}

	return "";
}

String CommandsManager::GetRedoCommandName(DAVA::Scene* forScene)
{
	UndoQueue* queue = GetQueueForScene(forScene);
	DVASSERT(queue);

	if ((queue->commandIndex >= -1) &&
		(queue->commandIndex < (int32)queue->commands.size() - 1))
	{
		return queue->commands[queue->commandIndex + 1]->commandName;
	}

	return "";
}

CommandsManager::UndoQueue* CommandsManager::GetQueueForScene(DAVA::Scene *scene)
{
	DVASSERT(scene);

	UndoQueue* res;

	QUEUE_MAP::iterator it = queueMap.find(scene);
	if (it != queueMap.end())
	{
		res = it->second;
	}
	else
	{
		res = new UndoQueue(scene);
		queueMap[scene] = res;
	}

	return res;
}

void CommandsManager::EmitCommandExecutedSignal(Command* command, Scene* scene)
{
	DVASSERT(scene);

	Set<Entity*> entities = command->GetAffectedEntities();
	if (!entities.empty())
	{
		CommandSignals::Instance()->EmitCommandAffectsEntities(scene, command->Id(), entities);
	}
}