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



#ifndef __SCENE_GRAPH_COMMANDS_H__
#define __SCENE_GRAPH_COMMANDS_H__

#include "Command.h"

class SceneData;
class DAVA::Entity;
class CommandRemoveRootNodes: public MultiCommand
{
public:	
	CommandRemoveRootNodes();
	virtual ~CommandRemoveRootNodes();

protected:
	virtual void Execute();
	virtual void Cancel();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

	CommandInternalRemoveSceneNode* removeCmd;
};


class CommandLockAtObject: public Command
{
public:
	DAVA_DEPRECATED(CommandLockAtObject());// DEPRECATED: using SceneDataManager(QOBJECT)
    
protected:
    
    virtual void Execute();
};


class CommandRemoveSceneNode: public MultiCommand
{
public:
	CommandRemoveSceneNode();
	virtual ~CommandRemoveSceneNode();

protected:
    virtual void Execute();
	virtual void Cancel();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

	CommandInternalRemoveSceneNode* removeCmd;
};

class CommandInternalRemoveSceneNode: public Command
{
public:
	CommandInternalRemoveSceneNode(DAVA::Entity* node, bool removeSimilar = false);
	virtual ~CommandInternalRemoveSceneNode();
	
protected:
	struct RemoveNodeRec
	{
		DAVA::Entity* node;
		DAVA::Entity* insertBeforeNode;
		DAVA::Entity* nodeParent;

		RemoveNodeRec()
		:	node(NULL)
		,	insertBeforeNode(NULL)
		,	nodeParent(NULL)
		{}
	};

	DAVA::Vector<RemoveNodeRec> nodesForDeletion;
	DAVA::Entity* selectedNode;

    virtual void Execute();
    virtual void Cancel();

	DAVA::int32 GetNodeIndex(const RemoveNodeRec& nodeRec);

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();
};


class CommandDebugFlags: public Command
{
    
public:	
	DAVA_DEPRECATED(CommandDebugFlags());// DEPRECATED: using SceneDataManager(QOBJECT)
    
protected:	
    
    virtual void Execute();
};




#endif // #ifndef __SCENE_GRAPH_COMMANDS_H__