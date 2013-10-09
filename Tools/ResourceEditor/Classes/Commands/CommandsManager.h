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



#ifndef __COMMANDS_MANAGER_H__
#define __COMMANDS_MANAGER_H__

#include "DAVAEngine.h"

class Command;
class DAVA::Scene;

class CommandsManager: public DAVA::Singleton<CommandsManager>
{
    static const DAVA::int32 UNDO_QUEUE_SIZE = 10;
    
public:	
	CommandsManager();
	virtual ~CommandsManager();
	
    void Execute(Command *command, DAVA::Scene* forScene);
	void ExecuteAndRelease(Command* command, DAVA::Scene* forScene);
    void Undo(DAVA::Scene* forScene);
    void Redo(DAVA::Scene* forScene);

	DAVA::int32 GetUndoQueueLength(DAVA::Scene* forScene);
	DAVA::int32 GetRedoQueueLength(DAVA::Scene* forScene);

	DAVA::String GetUndoCommandName(DAVA::Scene* forScene);
	DAVA::String GetRedoCommandName(DAVA::Scene* forScene);

	void SceneReleased(DAVA::Scene* scene);

private:
	struct UndoQueue
	{
		DAVA::Vector<Command*> commands;
		DAVA::int32 commandIndex;
		DAVA::Scene* activeScene;

		UndoQueue(DAVA::Scene* scene)
		:	commandIndex(-1)
		,	activeScene(scene)
		{ commands.reserve(UNDO_QUEUE_SIZE); };
	};

	typedef DAVA::Map<DAVA::Scene*, UndoQueue*> QUEUE_MAP;

    void ClearQueue(UndoQueue* queue);
    void ClearQueueTail(UndoQueue* queue);
	void ClearAllQueues();
	UndoQueue* GetQueueForScene(DAVA::Scene* scene);

	void EmitCommandExecutedSignal(Command* command, DAVA::Scene* scene);

	QUEUE_MAP queueMap;
};



#endif // __COMMANDS_MANAGER_H__