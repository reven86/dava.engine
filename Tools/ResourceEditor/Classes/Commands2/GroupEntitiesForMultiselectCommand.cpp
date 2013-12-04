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



#include "Commands2/GroupEntitiesForMultiselectCommand.h"
#include "../Qt/Scene/SceneDataManager.h"

GroupEntitiesForMultiselectCommand::GroupEntitiesForMultiselectCommand(const EntityGroup &entities)
	: Command2(CMDID_GROUP_ENTITIES_FOR_MULTISELECT, "Add complex entity with LODs")
{
    sceneEditor = NULL;
    resultEntity = NULL;
    
    if(!IsSelectionValid(entities))
    {
        Logger::Error("Wrong selection.");
    }
    else
    {
        entitiesToGroup = entities;
        Entity* en = entitiesToGroup.GetEntity(0);
        if(NULL != en)
        {
            sceneEditor = dynamic_cast<SceneEditor2 *>(en->GetScene());
        }
    }
}

GroupEntitiesForMultiselectCommand::~GroupEntitiesForMultiselectCommand()
{
	SafeRelease(resultEntity);
	for(Map<Entity*,DAVA::Component*>::iterator it = originalLodComponents.begin();
		it != originalLodComponents.end(); ++it)
	{
		DAVA::Component* lodComponent = it->second;
		SafeRelease(lodComponent);
	}
	originalLodComponents.clear();
}

void GroupEntitiesForMultiselectCommand::Undo()
{
	for(Map<Entity*, Entity*>::iterator itParent = originalChildParentRelations.begin();
		itParent != originalChildParentRelations.end(); ++itParent)
	{
		Entity* parent = itParent->second;
		if(NULL != parent)
		{
			parent->AddNode(itParent->first);
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
	//entities.erase(entities.find(resultEntity));
	if(NULL != resultEntity)
	{
		resultEntity->GetParent()->RemoveNode(resultEntity);
	}
	
	for(Map<Entity*,DAVA::Component*>::iterator it = originalLodComponents.begin();
		it != originalLodComponents.end(); ++it)
	{
		it->first->RemoveComponent(it->second->GetType());
		it->first->AddComponent(it->second);
	}
	originalLodComponents.clear();
}

void GroupEntitiesForMultiselectCommand::Redo()
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
	Entity* parent = solidEntityToAdd->GetParent();
	if(NULL == parent)
	{
		return;
	}
	Entity* complexEntity = new Entity();
	//
	complexEntity->SetName("mergeLodEntity");
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

			GetLodComponentsRecursive(en, originalLodComponents);
		}
	}
	
	LodSystem::MergeChildLods(complexEntity);
	
	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity* en = entitiesToGroup.GetEntity(i);
		UpdateTransformMatrixes(en, originalMatrixes[en]);
	}
	
	complexEntity->SetSolid(true);
	resultEntity = complexEntity;
}

void GroupEntitiesForMultiselectCommand::GetLodComponentsRecursive(Entity* fromEntity, DAVA::Map<DAVA::Entity*, DAVA::Component*>& hostEntitiesAndComponents)
{
	DAVA::Component* lodComponent = fromEntity->GetComponent(DAVA::Component::LOD_COMPONENT);
	if(NULL != lodComponent)
	{
		hostEntitiesAndComponents[fromEntity] = lodComponent->Clone(fromEntity);
	}
	
	int32 count = fromEntity->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		GetLodComponentsRecursive(fromEntity->GetChild(i), hostEntitiesAndComponents);
	}
}

Entity* GroupEntitiesForMultiselectCommand::GetEntity() const
{
	return resultEntity;
}


Entity* GroupEntitiesForMultiselectCommand::GetEntityWithSolidProp(Entity* en)
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

void GroupEntitiesForMultiselectCommand::UpdateTransformMatrixes(Entity* entity, Matrix4& worldMatrix)
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

void GroupEntitiesForMultiselectCommand::MoveEntity(Entity* entity, Vector3& destPoint)
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

bool GroupEntitiesForMultiselectCommand::IsSelectionValid(const EntityGroup &entities)
{
    if(entities.Size() < 2) return false;

    sceneEditor = dynamic_cast<SceneEditor2 *>(entities.GetEntity(0)->GetScene());
    for(size_t ei = 0; ei < entities.Size(); ++ei)
    {
        Entity *e = entities.GetEntity(ei)->GetParent();
        while (e && e != sceneEditor)
        {
            for(size_t ci = 0; ci < entities.Size(); ++ci)
            {
                if(ci == ei) continue;
                
                if(e == entities.GetEntity(ci))
                {
                    sceneEditor = NULL;
                    return false;
                }
            }
            
            e = e->GetParent();
        }
    }
    
    return true;
}
