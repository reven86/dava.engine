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



#include "SceneGraphCommands.h"

#include "DAVAEngine.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"
#include "../EditorScene.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "CommandsManager.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../StringConstants.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"

using namespace DAVA;

CommandRemoveRootNodes::CommandRemoveRootNodes()
    :   MultiCommand(Command::COMMAND_UNDO_REDO, CommandList::ID_COMMAND_REMOVE_ROOT_NODES)
	,	removeCmd(0)
{
	commandName = "Remove Root Nodes";

	SceneData* activeScene = SceneDataManager::Instance()->SceneGetActive();
	Entity *node = activeScene->GetSelectedNode();

	removeCmd = new CommandInternalRemoveSceneNode(node, true);
}

CommandRemoveRootNodes::~CommandRemoveRootNodes()
{
	SafeRelease(removeCmd);
}

void CommandRemoveRootNodes::Execute()
{
	ExecuteInternal(removeCmd);
}

void CommandRemoveRootNodes::Cancel()
{
	CancelInternal(removeCmd);
}

DAVA::Set<DAVA::Entity*> CommandRemoveRootNodes::GetAffectedEntities()
{
	return GetAffectedEntitiesInternal(removeCmd);
}


CommandLockAtObject::CommandLockAtObject()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_LOOK_AT_OBJECT)
{
}


void CommandLockAtObject::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    Entity *node = activeScene->GetSelectedNode();
    if(node)
    {
        activeScene->LockAtSelectedNode();
    }
    else
    {
        SetState(STATE_INVALID);
    }
}



CommandRemoveSceneNode::CommandRemoveSceneNode()
:   MultiCommand(Command::COMMAND_UNDO_REDO, CommandList::ID_COMMAND_REMOVE_SCENE_NODE)
{
	commandName = "Remove Object";

	SceneData* activeScene = SceneDataManager::Instance()->SceneGetActive();
	Entity *node = activeScene->GetSelectedNode();

	removeCmd = new CommandInternalRemoveSceneNode(node, false);
}

CommandRemoveSceneNode::~CommandRemoveSceneNode()
{
	SafeRelease(removeCmd);
}

void CommandRemoveSceneNode::Execute()
{
	ExecuteInternal(removeCmd);
}

void CommandRemoveSceneNode::Cancel()
{
	CancelInternal(removeCmd);
}

Set<Entity*> CommandRemoveSceneNode::GetAffectedEntities()
{
	return GetAffectedEntitiesInternal(removeCmd);
}


CommandInternalRemoveSceneNode::CommandInternalRemoveSceneNode(Entity* node, bool removeSimilar)
:	Command(Command::COMMAND_UNDO_REDO, CommandList::ID_COMMAND_INTERNAL_REMOVE_SCENE_NODE)
{
	commandName = "Remove Object";

	if (removeSimilar)
		commandName += "s";

	if (!node || !node->GetParent())
		return;

	if (removeSimilar)
	{
		String referenceToOwner;
		Entity* nodeParent = node->GetParent();

		KeyedArchive *customProperties = node->GetCustomProperties();
		if(customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
		{
			referenceToOwner = customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
		}

		nodesForDeletion.reserve(nodeParent->GetChildrenCount());

		for (int32 i = 0; i < nodeParent->GetChildrenCount(); ++i)
		{
			Entity* child = nodeParent->GetChild(i);

			customProperties = child->GetCustomProperties();
			if (customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
			{
				if (customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER) == referenceToOwner)
				{
					RemoveNodeRec removeNodeRec;
					removeNodeRec.node = SafeRetain(child);
					removeNodeRec.nodeParent = nodeParent;

					int32 i = GetNodeIndex(removeNodeRec);
					if (i >= 0 && i < nodeParent->GetChildrenCount() - 1)
						removeNodeRec.insertBeforeNode = nodeParent->GetChild(i + 1);

					nodesForDeletion.push_back(removeNodeRec);
				}
			}
		}
	}
	else
	{
		RemoveNodeRec removeNodeRec;
		removeNodeRec.node = SafeRetain(node);
		removeNodeRec.nodeParent = node->GetParent();

		int32 i = GetNodeIndex(removeNodeRec);
		if (i >= 0 && i < removeNodeRec.nodeParent->GetChildrenCount() - 1)
			removeNodeRec.insertBeforeNode = removeNodeRec.nodeParent->GetChild(i + 1);
		nodesForDeletion.push_back(removeNodeRec);
	}

	selectedNode = node;
}

CommandInternalRemoveSceneNode::~CommandInternalRemoveSceneNode()
{
	for (uint32 i = 0; i < nodesForDeletion.size(); ++i)
	{
		SafeRelease(nodesForDeletion[i].node);
	}
	nodesForDeletion.clear();
}

void CommandInternalRemoveSceneNode::Execute()
{
	if (nodesForDeletion.size() == 0)
	{
		SetState(STATE_INVALID);
		return;
	}

	for (uint32 i = 0; i < nodesForDeletion.size(); ++i)
	{
		nodesForDeletion[i].nodeParent->RemoveNode(nodesForDeletion[i].node);
	}

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();
}

void CommandInternalRemoveSceneNode::Cancel()
{
	if (nodesForDeletion.size() == 0)
		return;

	for (Vector<RemoveNodeRec>::reverse_iterator rIt = nodesForDeletion.rbegin(); rIt != nodesForDeletion.rend(); ++rIt)
	{
		if (rIt->insertBeforeNode)
			rIt->nodeParent->InsertBeforeNode(rIt->node, rIt->insertBeforeNode);
		else
			rIt->nodeParent->AddNode(rIt->node);
	}

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();

	if (selectedNode)
	{
		SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		DVASSERT(screen);

		EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
		if (bodyControl && !bodyControl->LandscapeEditorActive())
			activeScene->SelectNode(selectedNode);
	}
}

int32 CommandInternalRemoveSceneNode::GetNodeIndex(const RemoveNodeRec& nodeRec)
{
	if (!nodeRec.node || !nodeRec.nodeParent)
		return -1;

	int32 i = 0;
	for (; i < nodeRec.nodeParent->GetChildrenCount(); ++i)
	{
		if (nodeRec.nodeParent->GetChild(i) == nodeRec.node)
			return i;
	}

	return -1;
}

DAVA::Set<DAVA::Entity*> CommandInternalRemoveSceneNode::GetAffectedEntities()
{
	Set<Entity*> entities;
	for (Vector<RemoveNodeRec>::iterator it = nodesForDeletion.begin(); it != nodesForDeletion.end(); ++it)
	{
		entities.insert(it->node);
	}

	return entities;
}


CommandDebugFlags::CommandDebugFlags()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_DEBUG_FLAGS)
{
}


void CommandDebugFlags::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    Entity *node = activeScene->GetSelectedNode();
    if(node)
    {
        if ((node->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_ALL) == DebugRenderComponent::DEBUG_DRAW_ALL)
        {
            node->SetDebugFlags(0, true);
            if(activeScene->GetSelectedNode() == node)
            {
                node->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS);
            }
        }
        else
        {
            node->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);
        }
    }
    else
    {
        SetState(STATE_INVALID);
    }
}


