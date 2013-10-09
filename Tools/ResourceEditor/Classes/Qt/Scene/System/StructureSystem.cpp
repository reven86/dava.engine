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



#include "Scene/System/StructureSystem.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityMoveCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/ParticleLayerMoveCommand.h"
#include "Commands2/ParticleLayerRemoveCommand.h"
#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/ParticleForceRemoveCommand.h"

#include "Classes/SceneEditor/SceneValidator.h"

#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"

StructureSystem::StructureSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, lockedSignals(false)
{
    new DAVA::SwitchComponent();
    new DAVA::ActionComponent();
    
}

StructureSystem::~StructureSystem()
{

}

void StructureSystem::Init()
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		// mark solid all entities, that has childs as solid
		for(DAVA::int32 i = 0; i < sceneEditor->GetChildrenCount(); ++i)
		{
			CheckAndMarkSolid(sceneEditor->GetChild(i));
			CheckAndMarkLocked(sceneEditor->GetChild(i));
		}
	}
}

void StructureSystem::Move(const EntityGroup *entityGroup, DAVA::Entity *newParent, DAVA::Entity *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && NULL != entityGroup)
	{
		EntityGroup toMove = *entityGroup;

		if(toMove.Size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Move entities");
		}

		for(size_t i = 0; i < toMove.Size(); ++i)
		{
			sceneEditor->Exec(new EntityMoveCommand(toMove.GetEntity(i), newParent, newBefore));
		}

		if(toMove.Size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::Remove(const EntityGroup *entityGroup)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && NULL != entityGroup)
	{
		EntityGroup toRemove = *entityGroup;

		if(toRemove.Size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Remove entities");
		}

		for(size_t i = 0; i < toRemove.Size(); ++i)
		{
			sceneEditor->Exec(new EntityRemoveCommand(toRemove.GetEntity(i)));
		}

		if(toRemove.Size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::MoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers, DAVA::ParticleEmitter *newEmitter, DAVA::ParticleLayer *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{
			sceneEditor->Exec(new ParticleLayerMoveCommand(layers[i], newEmitter, newBefore));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}


void StructureSystem::RemoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{
			sceneEditor->Exec(new ParticleLayerRemoveCommand(layers[i]));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::MoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &oldLayers, DAVA::ParticleLayer *newLayer)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceMoveCommand(forces[i], oldLayers[i], newLayer));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::RemoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &layers)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			LockSignals();
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceRemoveCommand(forces[i], layers[i]));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
			UnlockSignals();

			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::Reload(const EntityGroup *entityGroup, const DAVA::FilePath &newModelPath, bool saveLightmapSettings)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && entityGroup->Size() > 0)
	{
		DAVA::Vector<DAVA::Entity*> newEntities;
		bool loadSuccess = false;

		// load new models
		for(size_t i = 0; i < entityGroup->Size(); ++i)
		{
			DAVA::FilePath loadModelPath = newModelPath;
			DAVA::Entity *entity = entityGroup->GetEntity(i);

			newEntities.push_back(NULL);

			if(loadModelPath.IsEmpty())
			{
				DAVA::KeyedArchive *entityProperties = entity->GetCustomProperties();
				if(NULL != entityProperties)
				{
					loadModelPath = entityProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
				}
			}

			DAVA::Entity *loadedEntity = Load(loadModelPath);
			if(NULL != loadedEntity)
			{
				loadedEntity->GetCustomProperties()->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, loadModelPath.GetAbsolutePathname());
				newEntities[i] = loadedEntity;

				loadSuccess = true;
			}
		}

		// replace old models with new
		if(loadSuccess)
		{
			LockSignals();
			sceneEditor->BeginBatch("Reload model");

			for(size_t i = 0; i < entityGroup->Size(); ++i)
			{
				DAVA::Entity *origEntity = entityGroup->GetEntity(i);
				DAVA::Entity *newEntity = newEntities[i];

				if(NULL != origEntity && NULL != newEntity && NULL != origEntity->GetParent())
				{
					DAVA::Entity *before = origEntity->GetParent()->GetNextChild(origEntity);

					newEntity->SetLocalTransform(origEntity->GetLocalTransform());
                    
                    if(saveLightmapSettings)
                    {
                        CopyLightmapSettings(origEntity, newEntity);
                    }
					
					sceneEditor->Exec(new EntityMoveCommand(newEntity, origEntity->GetParent(), before));
					sceneEditor->Exec(new EntityRemoveCommand(origEntity));

					newEntity->Release();
				}
			}

			sceneEditor->EndBatch();
			UnlockSignals();

            SceneValidator::Instance()->ValidateSceneAndShowErrors(GetScene());
            
			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::Add(const DAVA::FilePath &newModelPath, const DAVA::Vector3 pos)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		DAVA::Entity *loadedEntity = Load(newModelPath);
		if(NULL != loadedEntity)
		{
            KeyedArchive *customProps = loadedEntity->GetCustomProperties();
            customProps->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, newModelPath.GetAbsolutePathname());
            
			DAVA::Matrix4 transform = loadedEntity->GetLocalTransform();
			transform.SetTranslationVector(pos);
			loadedEntity->SetLocalTransform(transform);

			sceneEditor->Exec(new EntityMoveCommand(loadedEntity, sceneEditor, NULL));
			loadedEntity->Release();

			sceneEditor->UpdateShadowColorFromLandscape();

            SceneValidator::Instance()->ValidateSceneAndShowErrors(GetScene());
            
			SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
		}
	}
}

void StructureSystem::LockSignals()
{
	lockedSignals = true;
}

void StructureSystem::UnlockSignals()
{
	lockedSignals = false;
}

void StructureSystem::Update(DAVA::float32 timeElapsed)
{

}

void StructureSystem::Draw()
{

}

void StructureSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void StructureSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(!lockedSignals)
	{
		if(NULL != command)
		{
			int cmdId = command->GetId();
			if(cmdId == CMDID_PARTICLE_LAYER_REMOVE ||
			   cmdId == CMDID_PARTICLE_LAYER_MOVE ||
			   cmdId == CMDID_PARTICLE_FORCE_REMOVE ||
			   cmdId == CMDID_PARTICLE_FORCE_MOVE)
			{
				SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), NULL);
			}
		}
	}
}

void StructureSystem::AddEntity(DAVA::Entity * entity)
{
	if(!lockedSignals)
	{
		DAVA::Entity *parent = (NULL != entity) ? entity->GetParent() : NULL;
		SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), parent);
	}
}

void StructureSystem::RemoveEntity(DAVA::Entity * entity)
{
	if(!lockedSignals)
	{
		DAVA::Entity *parent = (NULL != entity) ? entity->GetParent() : NULL;
		SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), parent);
	}
}

void StructureSystem::CheckAndMarkSolid(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		if(entity->GetChildrenCount() > 0)
		{
			entity->SetSolid(true);

			for(DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkSolid(entity->GetChild(i));
			}
		}
		else
		{
			entity->SetSolid(false);
		}
	}
}

void StructureSystem::CheckAndMarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		// mark lod childs as locked
		if(NULL != entity->GetComponent(DAVA::Component::LOD_COMPONENT))
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				MarkLocked(entity->GetChild(i));
			}
		}
		else
		{
			for(int i = 0; i < entity->GetChildrenCount(); ++i)
			{
				CheckAndMarkLocked(entity->GetChild(i));
			}
		}
	}
}

void StructureSystem::MarkLocked(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		entity->SetLocked(true);

		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			MarkLocked(entity->GetChild(i));
		}
	}
}

DAVA::Entity* StructureSystem::Load(const DAVA::FilePath& sc2path)
{
	DAVA::Entity* loadedEntity = NULL;
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();

	if(NULL != sceneEditor && sc2path.IsEqualToExtension(".sc2") && sc2path.Exists())
	{
		loadedEntity = sceneEditor->GetRootNode(sc2path);

		if(NULL != loadedEntity)
		{
			Entity *parentForOptimize = new Entity();
			parentForOptimize->AddNode(loadedEntity);

			SceneFileV2 sceneFile;
			sceneFile.OptimizeScene(parentForOptimize);

			if(parentForOptimize->GetChildrenCount())
			{
				loadedEntity = parentForOptimize->GetChild(0);
				loadedEntity->SetSolid(true);
				loadedEntity->Retain();
			}
			else
			{
				loadedEntity->Release();
				loadedEntity = NULL;
			}

			parentForOptimize->Release();
		}

		sceneEditor->ReleaseRootNode(sc2path);
	}

	return loadedEntity;
}

DAVA::Landscape * StructureSystem::FindLanscape() const
{
    Entity *entity = FindLandscapeEntity();
    return GetLandscape(entity);
}

DAVA::Entity * StructureSystem::FindLandscapeEntity() const
{
	return FindLandscapeEntityRecursive(GetScene());
}


DAVA::Entity * StructureSystem::FindLandscapeEntityRecursive( DAVA::Entity *entity ) const
{
	if(GetLandscape(entity))
	{
		return entity;
	}

	DAVA::int32 count = entity->GetChildrenCount();
	for(DAVA::int32 i = 0; i < count; ++i)
	{
		Entity *child = entity->GetChild(i);
		if(FindLandscapeEntityRecursive(child))
		{
			return child;
		}
	}

	return NULL;
}


bool StructureSystem::CopyLightmapSettings(DAVA::Entity *fromEntity, DAVA::Entity *toEntity) const
{
    DAVA::Vector<DAVA::RenderObject *> fromMeshes;
    FindMeshesRecursive(fromEntity, fromMeshes);

    DAVA::Vector<DAVA::RenderObject *> toMeshes;
    FindMeshesRecursive(toEntity, toMeshes);
    
    if(fromMeshes.size() == toMeshes.size())
    {
        DAVA::uint32 meshCount = (DAVA::uint32)fromMeshes.size();
        for(DAVA::uint32 m = 0; m < meshCount; ++m)
        {
            DAVA::uint32 rbFromCount = fromMeshes[m]->GetRenderBatchCount();
            DAVA::uint32 rbToCount = toMeshes[m]->GetRenderBatchCount();
            
            if(rbFromCount != rbToCount)
                return false;
            
            for(DAVA::uint32 rb = 0; rb < rbFromCount; ++rb)
            {
                DAVA::RenderBatch *fromBatch = fromMeshes[m]->GetRenderBatch(rb);
                DAVA::RenderBatch *toBatch = toMeshes[m]->GetRenderBatch(rb);
                
                DAVA::InstanceMaterialState *fromState = fromBatch->GetMaterialInstance();
                DAVA::InstanceMaterialState *toState = toBatch->GetMaterialInstance();
                
                if(fromState && toState)
                {
                    toState->InitFromState(fromState);
                }
                
            }
        }
        
        return true;
    }

    return false;
}

void StructureSystem::FindMeshesRecursive(DAVA::Entity *entity, DAVA::Vector<DAVA::RenderObject *> & objects) const
{
    RenderObject *ro = GetRenderObject(entity);
    if(ro && ro->GetType() == RenderObject::TYPE_MESH)
    {
        objects.push_back(ro);
    }
    
	DAVA::int32 count = entity->GetChildrenCount();
	for(DAVA::int32 i = 0; i < count; ++i)
	{
        FindMeshesRecursive(entity->GetChild(i), objects);
	}
}


