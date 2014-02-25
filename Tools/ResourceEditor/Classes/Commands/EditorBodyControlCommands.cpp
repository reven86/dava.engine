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



#include "EditorBodyControlCommands.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "CommandsManager.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"

#include "Scene/SceneEditor2.h"
#include "Scene/System/CollisionSystem.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"

CommandEntityModification::CommandEntityModification(Command::eCommandType type, CommandList::eCommandId id)
:	Command(type, id)
{
}

DAVA::Set<DAVA::Entity*> CommandEntityModification::GetAffectedEntities()
{
	return entities;
}

CommandGroupEntitiesForMultiselect::CommandGroupEntitiesForMultiselect(const EntityGroup* entities)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_UNITE_ENTITIES_FOR_MULTISELECT)
{
	commandName = "Unite entities for multiselect";
	this->entitiesToGroup = (*entities);
	this->resultEntity = NULL;
	Entity* en = entitiesToGroup.GetEntity(0);
	sceneEditor = NULL;
	if(NULL != en)
	{
		sceneEditor = dynamic_cast<SceneEditor2 *>(en->GetScene());
	}
}

void CommandGroupEntitiesForMultiselect::Execute()
{
	if(entitiesToGroup.Size() < 2)
	{
		return;
	}

	Vector3 originalModifEntitiesCenter = entitiesToGroup.GetCommonBbox().GetCenter();
	Entity* solidEntityToAdd = GetEntityWithSolidProp(entitiesToGroup.GetEntity(0));
	if(NULL == solidEntityToAdd)
	{
		return;
	}
	Entity* parent = solidEntityToAdd->GetParent();//check
	if(NULL == parent)
	{
		return;
	}
	Entity* complexEntity = new Entity();
	//complexEntity->SetName("textComplexEntity");
	//complexEntity->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);

	parent->AddNode(complexEntity);
	MoveEntity(complexEntity, originalModifEntitiesCenter);

	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity *en = entitiesToGroup.GetEntity(i);
		if(NULL != en )
		{
			solidEntityToAdd = GetEntityWithSolidProp(en);
			originalMatrixes[en] = en->GetWorldTransform();
			originalChildParentRelations[solidEntityToAdd] = solidEntityToAdd->GetParent();
			complexEntity->AddNode(solidEntityToAdd);
		}
	}
	
	LodSystem::MergeChildLods(complexEntity);

	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity* en = entitiesToGroup.GetEntity(i);
		UpdateTransformMatrixes(en, originalMatrixes[en]);
	}

	resultEntity = complexEntity;

	entities.insert(resultEntity);
	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity *en = entitiesToGroup.GetEntity(i);
		entities.insert(en);
	}
}

void CommandGroupEntitiesForMultiselect::Cancel()
{
	for(Map<Entity*, Entity*>::iterator itParent = originalChildParentRelations.begin();
		itParent != originalChildParentRelations.end(); ++itParent)
	{
		Entity* parent = (*itParent).second;
		if(NULL != parent)
		{
			parent->AddNode((*itParent).first);
		}
	}
	originalChildParentRelations.clear();
	for(Map<Entity*, Matrix4>::iterator itPosition = originalMatrixes.begin();
		itPosition != originalMatrixes.end(); ++itPosition)
	{
		Entity* entity = (*itPosition).first;
		if(NULL != entity)
		{
			UpdateTransformMatrixes(entity, itPosition->second);
		}
	}
	originalMatrixes.clear();
	entities.erase(entities.find(resultEntity));
	if(NULL != resultEntity)
	{
		resultEntity->GetParent()->RemoveNode(resultEntity);
		SafeRelease(resultEntity);
	}
}

Entity* CommandGroupEntitiesForMultiselect::GetEntityWithSolidProp(Entity* en)
{
	Entity* solidEntity = en;
	while (NULL != solidEntity)
	{
		KeyedArchive *customProperties = solidEntity->GetCustomProperties();
		if(customProperties && customProperties->IsKeyExists(String(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME)))
		{
			break;
		}
		solidEntity = solidEntity->GetParent();
	};
	return solidEntity;
}

void CommandGroupEntitiesForMultiselect::UpdateTransformMatrixes(Entity* entity, Matrix4& worldMatrix)
{
	if(NULL == entity)
	{
		return;
	}

	DAVA::Matrix4 newTransform = worldMatrix;

	// to move the entity directly to absolute coordinates all paretn matrixes must be multiplied
	// and be taken into consideration beacause they are procesed  in TransformSystem (HierahicFindUpdatableTransform)
	if(entity->GetParent() != NULL)
	{
		Entity* parent = entity->GetParent();
		//Matrix4 parentMatrix = entity->GetParent()->GetWorldTransform();
		Matrix4 parentMatrix = Matrix4::IDENTITY;

		//calculate paretn matrixes through entire parent tree
		while (parent && !(parent->GetLocalTransform() == Matrix4::IDENTITY && parent->GetWorldTransform() == Matrix4::IDENTITY))
		{
			Matrix4 tempMatrix = parentMatrix * parent->GetLocalTransform();
			parentMatrix = tempMatrix;
			parent = parent->GetParent();
		};
		
		// newTransform should be devided by parentMatrix, because it would be multiplied in HierahicFindUpdatableTransform
		// matrix operation : A / B = ( B ^ (-1) ) * A
		Matrix4 inversedParetnMatrix;//(B ^ (-1))

		bool canBeInversed = parentMatrix.GetInverse(inversedParetnMatrix);
		if(!canBeInversed)
		{
			return;
		}

		Matrix4 absoluteNewTransform =  newTransform * inversedParetnMatrix ; //( B ^ (-1) ) * A
		newTransform = absoluteNewTransform;
	}
	entity->SetLocalTransform(newTransform);
}

void CommandGroupEntitiesForMultiselect::MoveEntity(Entity* entity, Vector3& destPoint)
{
	if(NULL == sceneEditor || NULL == entity)
	{
		return;
	}
	DAVA::AABBox3 currentItemBB;
	sceneEditor->collisionSystem->GetBoundingBox(entity).GetTransformedBox(entity->GetWorldTransform(), currentItemBB);

	Vector3 centrOfEntity = currentItemBB.GetCenter();
	DAVA::Vector3 moveOffset = destPoint - centrOfEntity;
	DAVA::Matrix4 moveModification;
	moveModification.CreateTranslation(moveOffset);
	
	DAVA::Matrix4 newTransform = entity->GetWorldTransform() * moveModification;

	UpdateTransformMatrixes(entity,newTransform);
}


CommandTransformObject::CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_TRANSFORM_OBJECT)
{
	commandName = "Transform Object";

	undoTransform = originalTransform;
	redoTransform = finalTransform;

	entities.insert(node);
}

void CommandTransformObject::Execute()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(redoTransform);
		UpdateCollision();
	}
	else
		SetState(STATE_INVALID);
}

void CommandTransformObject::Cancel()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(undoTransform);
		UpdateCollision();
	}
}

void CommandTransformObject::UpdateCollision()
{
	Entity* node = *entities.begin();

	SceneEditor2 *sep = dynamic_cast<SceneEditor2 *>(node->GetScene());
	if(NULL != sep && NULL != sep->collisionSystem)
	{
		// make sure that worldtransform is up to date
		sep->transformSystem->Process(.001f);

		// update bullet object
		sep->collisionSystem->UpdateCollisionObject(node);
	}
}


CommandCloneObject::CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_CLONE_OBJECT)
,	collisionWorld(collisionWorld)
{
	commandName = "Clone Object";

	originalNode = node;
	this->bodyControl = bodyControl;
}

CommandCloneObject::~CommandCloneObject()
{
	if (!entities.empty())
	{
		Entity* clonedNode = *entities.begin();
		SafeRelease(clonedNode);
	}
}

void CommandCloneObject::Execute()
{
	if (originalNode && bodyControl)
	{
		Entity* clonedNode = 0;

		if (entities.empty())
		{
			clonedNode = originalNode->Clone();
			if (!clonedNode)
			{
				SetState(STATE_INVALID);
				return;
			}

			//if (collisionWorld)
			//	UpdateCollision(clonedNode);

			entities.insert(clonedNode);
		}
		else
		{
			clonedNode = *entities.begin();
		}

		originalNode->GetParent()->AddNode(clonedNode);
		bodyControl->SelectNode(clonedNode);
	}
	else
		SetState(STATE_INVALID);
}

void CommandCloneObject::Cancel()
{
	Entity* clonedNode = GetClonedNode();

	if (originalNode && clonedNode)
	{
		clonedNode->GetParent()->RemoveNode(clonedNode);

		// rebuild scene graph after removing node
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->RebuildSceneGraph();

		if (bodyControl)
			bodyControl->SelectNode(originalNode);
	}
}

void CommandCloneObject::UpdateCollision(DAVA::Entity *node)
{
	DVASSERT(node && collisionWorld);
	
	BulletComponent* bc = dynamic_cast<BulletComponent*>(node->GetComponent(Component::BULLET_COMPONENT));
	if (bc && !bc->GetBulletObject())
	{
		bc->SetBulletObject(ScopedPtr<BulletObject>(new BulletObject(node->GetScene(),
																	 collisionWorld,
																	 node,
																	 node->GetWorldTransform())));
	}
	
	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		UpdateCollision(node->GetChild(i));
	}
}

Entity* CommandCloneObject::GetClonedNode()
{
	Entity* clonedNode = 0;
	if (!entities.empty())
	{
		clonedNode = *entities.begin();
	}

	return clonedNode;
}


CommandCloneAndTransform::CommandCloneAndTransform(DAVA::Entity* originalNode,
												   const DAVA::Matrix4& finalTransform,
												   EditorBodyControl* bodyControl,
												   btCollisionWorld* collisionWorld)
:	MultiCommand(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_CLONE_AND_TRANSFORM)
,	clonedNode(0)
,	cloneCmd(0)
,	transformCmd(0)
{
	commandName = "Clone Object";

	this->originalNode = originalNode;
	this->bodyControl = bodyControl;
	this->collisionWorld = collisionWorld;
	this->transform = finalTransform;
}

CommandCloneAndTransform::~CommandCloneAndTransform()
{
	SafeRelease(transformCmd);
	SafeRelease(cloneCmd);
}

void CommandCloneAndTransform::Execute()
{
	if (!cloneCmd)
	{
		cloneCmd = new CommandCloneObject(originalNode, bodyControl, collisionWorld);
	}
	ExecuteInternal(cloneCmd);

	if (GetInternalCommandState(cloneCmd) != STATE_VALID)
	{
		SetState(STATE_INVALID);
		return;
	}

	if(!transformCmd)
	{
		transformCmd = new CommandTransformObject(cloneCmd->GetClonedNode(), originalNode->GetLocalTransform(), transform);
	}

	// Need to apply transform only once when creating command
	ExecuteInternal(transformCmd);

	if (GetInternalCommandState(transformCmd) != STATE_VALID)
	{
		SetState(STATE_INVALID);
		return;
	}
}

void CommandCloneAndTransform::Cancel()
{
	// No need to undo transform command, removed node will appear at the same place where it was removed
	if (cloneCmd)
	{
		CancelInternal(cloneCmd);
	}
}


CommandPlaceOnLandscape::CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_PLACE_ON_LANDSCAPE)
,	bodyControl(bodyControl)
{
	commandName = "Place On Landscape";

	redoTransform.Identity();

	if (node)
		undoTransform = node->GetLocalTransform();

	entities.insert(node);
}

void CommandPlaceOnLandscape::Execute()
{
	Entity* node = *entities.begin();

	if (node && bodyControl)
	{
		redoTransform = node->GetLocalTransform() * bodyControl->GetLandscapeOffset(node->GetWorldTransform());
		node->SetLocalTransform(redoTransform);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceOnLandscape::Cancel()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(undoTransform);
	}
}


CommandRestoreOriginalTransform::CommandRestoreOriginalTransform(DAVA::Entity* node)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_RESTORE_ORIGINAL_TRANSFORM)
{
	commandName = "Restore Original Transform";

	if (node)
	{
		StoreCurrentTransform(node);
	}

	entities.insert(node);
}

void CommandRestoreOriginalTransform::Execute()
{
	Entity* node = *entities.begin();

	if (node)
	{
		// DF-1326 fix - Restore local matrix to identity
		node->SetLocalTransform(Matrix4::IDENTITY);
	}
	else
		SetState(STATE_INVALID);
}

void CommandRestoreOriginalTransform::Cancel()
{
	Entity* node = *entities.begin();

	if (node)
	{
		RestoreTransform(node);
	}
}

void CommandRestoreOriginalTransform::StoreCurrentTransform(DAVA::Entity *node)
{
	if (node)
	{
		undoTransforms[node] = node->GetLocalTransform();

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
			StoreCurrentTransform(node->GetChild(i));
	}
}

void CommandRestoreOriginalTransform::RestoreTransform(DAVA::Entity *node)
{
	if (node)
	{
		Map<Entity*, Matrix4>::iterator it = undoTransforms.find(node);
		if (it != undoTransforms.end())
		{
			node->SetLocalTransform((*it).second);
		}

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
		{
			RestoreTransform(node->GetChild(i));
		}
	}
}
