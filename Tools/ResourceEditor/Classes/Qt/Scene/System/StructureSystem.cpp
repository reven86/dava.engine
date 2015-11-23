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
#include "Scene/System/CameraSystem.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityParentChangeCommand.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/ParticleEmitterMoveCommands.h"
#include "Commands2/ParticleLayerMoveCommand.h"
#include "Commands2/ParticleLayerRemoveCommand.h"
#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/ParticleForceRemoveCommand.h"

#include "Deprecated/SceneValidator.h"

StructureSystem::StructureSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, structureChanged(false)
{
}

StructureSystem::~StructureSystem()
{

}

bool StructureSystem::Init(const DAVA::FilePath & path)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if(NULL == sceneEditor)
    {
        return false;
    }

    return (SceneFileV2::ERROR_NO_ERROR == sceneEditor->LoadScene(path));
}

void StructureSystem::Move(const EntityGroup &entityGroup, DAVA::Entity *newParent, DAVA::Entity *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor && entityGroup.Size() > 0)
	{
		if(entityGroup.Size() > 1)
		{
			sceneEditor->BeginBatch("Move entities");
		}

		for(size_t i = 0; i < entityGroup.Size(); ++i)
		{
			sceneEditor->Exec(new EntityParentChangeCommand(entityGroup.GetEntity(i), newParent, newBefore));
		}

		if(entityGroup.Size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::Remove(const EntityGroup &entityGroup)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(nullptr != sceneEditor && entityGroup.Size() > 0)
	{
        sceneEditor->BeginBatch("Remove entities");

        for (size_t i = 0; i < entityGroup.Size(); ++i)
        {
            DAVA::Entity *entity = entityGroup.GetEntity(i);
            if (entity->GetNotRemovable() == false)
            {
                for (auto delegate : delegates)
                {
                    delegate->WillRemove(entity);
                }

                sceneEditor->Exec(new EntityRemoveCommand(entity));

                for (auto delegate : delegates)
                {
                    delegate->DidRemoved(entity);
                }
            }
        }

        sceneEditor->EndBatch();

		EmitChanged();
	}
}

void StructureSystem::MoveEmitter(const DAVA::Vector<DAVA::ParticleEmitter *> &emitters, const DAVA::Vector<DAVA::ParticleEffectComponent *>& oldEffects, DAVA::ParticleEffectComponent *newEffect, int dropAfter)
{
    SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
    if(NULL != sceneEditor)
    {
        if(emitters.size() > 1)
        {
            sceneEditor->BeginBatch("Move particle emitter");
        }

        for(size_t i = 0; i < emitters.size(); ++i)
        {		
            sceneEditor->Exec(new ParticleEmitterMoveCommand(oldEffects[i], emitters[i], newEffect, dropAfter++));
        }

        if(emitters.size() > 1)
        {
            sceneEditor->EndBatch();
        }

        EmitChanged();
    }
}

void StructureSystem::MoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers, const DAVA::Vector<DAVA::ParticleEmitter *>& oldEmitters, DAVA::ParticleEmitter *newEmitter, DAVA::ParticleLayer *newBefore)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{		
			sceneEditor->Exec(new ParticleLayerMoveCommand(oldEmitters[i], layers[i], newEmitter, newBefore));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}


void StructureSystem::RemoveLayer(const DAVA::Vector<DAVA::ParticleLayer *> &layers, const DAVA::Vector<DAVA::ParticleEmitter *>& oldEmitters)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(layers.size() > 1)
		{
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < layers.size(); ++i)
		{
			
			sceneEditor->Exec(new ParticleLayerRemoveCommand(oldEmitters[i], layers[i]));
		}

		if(layers.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::MoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &oldLayers, DAVA::ParticleLayer *newLayer)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			sceneEditor->BeginBatch("Move particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceMoveCommand(forces[i], oldLayers[i], newLayer));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::RemoveForce(const DAVA::Vector<DAVA::ParticleForce *> &forces, const DAVA::Vector<DAVA::ParticleLayer *> &layers)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		if(forces.size() > 1)
		{
			sceneEditor->BeginBatch("Remove particle layers");
		}

		for(size_t i = 0; i < forces.size(); ++i)
		{
			sceneEditor->Exec(new ParticleForceRemoveCommand(forces[i], layers[i]));
		}

		if(forces.size() > 1)
		{
			sceneEditor->EndBatch();
		}

		EmitChanged();
	}
}

void StructureSystem::ReloadEntities(const EntityGroup& entityGroup, bool saveLightmapSettings)
{
	if(entityGroup.Size() > 0)
	{
		DAVA::Set<DAVA::FilePath> refsToReload;

		for(int i = 0; i < (int)entityGroup.Size(); ++i)
		{
            DAVA::KeyedArchive * props = GetCustomPropertiesArchieve(entityGroup.GetEntity(i));
			if(NULL != props)
			{
				DAVA::FilePath pathToReload(props->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER));
				if(!pathToReload.IsEmpty())
				{
					refsToReload.insert(pathToReload);
				}
			}
		}

		DAVA::Set<DAVA::FilePath>::iterator it = refsToReload.begin();
		for(; it != refsToReload.end(); ++it)
		{
			ReloadRefs(*it, saveLightmapSettings);
		}
	}
}

void StructureSystem::ReloadRefs(const DAVA::FilePath &modelPath, bool saveLightmapSettings)
{
	if(!modelPath.IsEmpty())
	{
		DAVA::Set<DAVA::Entity *> entitiesToReload;
		SearchEntityByRef(GetScene(), modelPath, entitiesToReload);
		ReloadInternal(entitiesToReload, modelPath, saveLightmapSettings);
	}
}

void StructureSystem::ReloadEntitiesAs(const EntityGroup& entityGroup, const DAVA::FilePath &newModelPath, bool saveLightmapSettings)
{
	if(entityGroup.Size() > 0)
	{
		DAVA::Set<DAVA::Entity *> entitiesToReload;

		for (int i = 0; i < (int)entityGroup.Size(); i++)
		{
			entitiesToReload.insert(entityGroup.GetEntity(i));
		}

		ReloadInternal(entitiesToReload, newModelPath, saveLightmapSettings);
	}
}

void StructureSystem::ReloadInternal(DAVA::Set<DAVA::Entity *> &entitiesToReload, const DAVA::FilePath &newModelPath, bool saveLightmapSettings)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(NULL != sceneEditor)
	{
		// also we should reload all entities, that already has reference to the same newModelPath
		SearchEntityByRef(GetScene(), newModelPath, entitiesToReload);

		if(entitiesToReload.size() > 0)
		{
			// try to load new model
			DAVA::Entity *loadedEntity = LoadInternal(newModelPath, true);

			if(NULL != loadedEntity)
			{
				DAVA::Set<DAVA::Entity *>::iterator it = entitiesToReload.begin();
				DAVA::Set<DAVA::Entity *>::iterator end = entitiesToReload.end();

				sceneEditor->BeginBatch("Reload model");

				for(; it != end; ++it)
				{
					DAVA::Entity *newEntityInstance = loadedEntity->Clone();
					DAVA::Entity *origEntity = *it;

					if(NULL != origEntity && NULL != newEntityInstance && NULL != origEntity->GetParent())
					{
						DAVA::Entity *before = origEntity->GetParent()->GetNextChild(origEntity);

						newEntityInstance->SetLocalTransform(origEntity->GetLocalTransform());
                        newEntityInstance->SetID(origEntity->GetID());
                        newEntityInstance->SetSceneID(origEntity->GetSceneID());

						if(saveLightmapSettings)
						{
							CopyLightmapSettings(origEntity, newEntityInstance);
						}

						sceneEditor->Exec(new EntityParentChangeCommand(newEntityInstance, origEntity->GetParent(), before));
						sceneEditor->Exec(new EntityRemoveCommand(origEntity));

                        newEntityInstance->Release();
					}
				}

				sceneEditor->EndBatch();
				loadedEntity->Release();
			}
		}
	}
}

void StructureSystem::Add(const DAVA::FilePath &newModelPath, const DAVA::Vector3 pos)
{
	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
	if(nullptr != sceneEditor)
	{
        ScopedPtr<Entity> loadedEntity(Load(newModelPath));
        if (static_cast<DAVA::Entity *>(loadedEntity) != nullptr)
		{
			DAVA::Vector3 entityPos = pos;

			KeyedArchive *customProps = GetOrCreateCustomProperties(loadedEntity)->GetArchive();
            customProps->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, newModelPath.GetAbsolutePathname());

			if(entityPos.IsZero() && FindLandscape(loadedEntity) == nullptr)
			{
				SceneCameraSystem *cameraSystem = sceneEditor->cameraSystem;

				DAVA::Vector3 camDirection = cameraSystem->GetCameraDirection();
				DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();

				DAVA::AABBox3 commonBBox = loadedEntity->GetWTMaximumBoundingBoxSlow();
				DAVA::float32 bboxSize = 5.0f;
                
                if(!commonBBox.IsEmpty())
                {
                    bboxSize += (commonBBox.max - commonBBox.min).Length();
                }

				camDirection.Normalize();
				
				entityPos = camPosition + camDirection * bboxSize;
			}

			DAVA::Matrix4 transform = loadedEntity->GetLocalTransform();
			transform.SetTranslationVector(entityPos);
			loadedEntity->SetLocalTransform(transform);
            
            if (GetPathComponent(loadedEntity))
            {
                sceneEditor->pathSystem->AddPath(loadedEntity);
            }
            else
            {
                sceneEditor->Exec(new EntityAddCommand(loadedEntity, sceneEditor));
            }

			// TODO: move this code to some another place (into command itself or into ProcessCommand function)
			// 
			// Перенести в Load и завалидейтить только подгруженную Entity
			// -->
			SceneValidator::Instance()->ValidateSceneAndShowErrors(sceneEditor, sceneEditor->GetScenePath());
			// <--
            
			EmitChanged();
		}
	}
}

void StructureSystem::EmitChanged()
{
	// mark that structure was changed. real signal will be emited on next update() call
	// this should done be to increase performance - on Change emit on multiple scene structure operations
	structureChanged = true;
}

void StructureSystem::AddDelegate(StructureSystemDelegate *delegate)
{
    delegates.push_back(delegate);
}

void StructureSystem::RemoveDelegate(StructureSystemDelegate *delegate)
{
    delegates.remove(delegate);
}

void StructureSystem::Process(DAVA::float32 timeElapsed)
{
	if(structureChanged)
	{
		SceneSignals::Instance()->EmitStructureChanged((SceneEditor2 *) GetScene(), nullptr);
		structureChanged = false;
	}
}

void StructureSystem::Draw()
{

}

void StructureSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(NULL != command)
	{
        switch(command->GetId())
        {
            case CMDID_PARTICLE_LAYER_REMOVE:
            case CMDID_PARTICLE_LAYER_MOVE:
            case CMDID_PARTICLE_FORCE_REMOVE:
            case CMDID_PARTICLE_FORCE_MOVE:
                EmitChanged();
                break;

            default:
                break;
        }
        
        auto autoSelectionEnabled = SettingsManager::GetValue(Settings::Scene_AutoselectNewEntities).AsBool();
        if(autoSelectionEnabled)
        {
            ProcessAutoSelection(command, redo);
        }
    }
}

void StructureSystem::ProcessAutoSelection(const Command2 *command, bool redo) const
{
    auto commandId = command->GetId();

    auto sceneEditor = static_cast<SceneEditor2 *>(GetScene());
    auto selectionSystem = sceneEditor->selectionSystem;
    
    if(CMDID_BATCH == commandId)
    {
        auto batch = static_cast<const CommandBatch *>(command);
        
        auto contain = batch->ContainsCommand(CMDID_ENTITY_ADD) || batch->ContainsCommand(CMDID_ENTITY_REMOVE);
        if(contain)
        {
            selectionSystem->Clear();

            auto count = batch->Size();
            for(auto i = 0; i < count; ++i)
            {
                auto cmd = batch->GetCommand(i);
                auto cmdID = cmd->GetId();
                
                auto needAddEntity = ((CMDID_ENTITY_ADD == cmdID && redo) || (CMDID_ENTITY_REMOVE == cmdID && !redo));
                if(needAddEntity)
                {
                    selectionSystem->AddSelection(cmd->GetEntity());
                }
            }
        }
    }
    else if(CMDID_ENTITY_ADD == commandId || CMDID_ENTITY_REMOVE == commandId)
    {
        selectionSystem->Clear();
        
        auto needAddEntity = ((CMDID_ENTITY_ADD == commandId && redo) || (CMDID_ENTITY_REMOVE == commandId && !redo));
        if(needAddEntity)
        {
            selectionSystem->AddSelection(command->GetEntity());
        }
    }
}


void StructureSystem::AddEntity(DAVA::Entity * entity)
{
	EmitChanged();
}

void StructureSystem::RemoveEntity(DAVA::Entity * entity)
{
	EmitChanged();
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

DAVA::Entity* StructureSystem::Load(const DAVA::FilePath& sc2path)
{
	return LoadInternal(sc2path, false);
}

DAVA::Entity* StructureSystem::LoadInternal(const DAVA::FilePath& sc2path, bool clearCache)
{
	DAVA::Entity* loadedEntity = nullptr;

	SceneEditor2* sceneEditor = (SceneEditor2*) GetScene();
    if (nullptr != sceneEditor && sc2path.IsEqualToExtension(".sc2") && FileSystem::Instance()->Exists(sc2path))
    {
        if(clearCache)
        {
            // if there is already entity for such file, we should release it
            // to be sure that latest version will be loaded 
            sceneEditor->cache.Clear(sc2path);
        }

        loadedEntity = sceneEditor->cache.GetClone(sc2path);
        if(nullptr != loadedEntity)
        {
            // this is for backward compatibility.
            // sceneFileV2 will remove empty nodes only
            // if there is parent for such nodes. 
            {
                SceneFileV2* tmpSceneFile = new SceneFileV2();
                Entity* tmpParent = new Entity();
                Entity* tmpEntity = loadedEntity;

                tmpParent->AddNode(tmpEntity);
                tmpSceneFile->RemoveEmptyHierarchy(tmpEntity);

                loadedEntity = SafeRetain(tmpParent->GetChild(0));

                SafeRelease(tmpEntity);
                SafeRelease(tmpParent);
                SafeRelease(tmpSceneFile);
            }

            KeyedArchive *props = GetOrCreateCustomProperties(loadedEntity)->GetArchive();
            props->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, sc2path.GetAbsolutePathname());
            
            CheckAndMarkSolid(loadedEntity);

			SceneValidator::ExtractEmptyRenderObjectsAndShowErrors(loadedEntity);
        }
	}
    else
    {
        DAVA::Logger::Error("Wrong extension or no such file: %s", sc2path.GetAbsolutePathname().c_str());
    }

	return loadedEntity;
}

void StructureSystem::CopyLightmapSettings(DAVA::NMaterial *fromState, DAVA::NMaterial *toState) const
{
    if (fromState->HasLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP))
    {
        Texture* lightmap = fromState->GetLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP);
        if (toState->HasLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP))
            toState->SetTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, lightmap);
        else
            toState->AddTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, lightmap);
    }

    if (fromState->HasLocalProperty(NMaterialParamName::PARAM_UV_SCALE))
    {
        const float* data = fromState->GetLocalPropValue(NMaterialParamName::PARAM_UV_SCALE);
        if (toState->HasLocalProperty(NMaterialParamName::PARAM_UV_SCALE))
            toState->SetPropertyValue(NMaterialParamName::PARAM_UV_SCALE, data);
        else
            toState->AddProperty(NMaterialParamName::PARAM_UV_SCALE, data, rhi::ShaderProp::TYPE_FLOAT2);
    }

    if (fromState->HasLocalProperty(NMaterialParamName::PARAM_UV_OFFSET))
    {
        const float* data = fromState->GetLocalPropValue(NMaterialParamName::PARAM_UV_OFFSET);
        if (toState->HasLocalProperty(NMaterialParamName::PARAM_UV_OFFSET))
            toState->SetPropertyValue(NMaterialParamName::PARAM_UV_OFFSET, data);
        else
            toState->AddProperty(NMaterialParamName::PARAM_UV_OFFSET, data, rhi::ShaderProp::TYPE_FLOAT2);
    }
}

struct BatchInfo
{
	BatchInfo() : switchIndex(-1), lodIndex(-1), batch(NULL) {}

	DAVA::int32 switchIndex;
	DAVA::int32 lodIndex;

	DAVA::RenderBatch *batch;
};

struct SortBatches
{
	bool operator()(const BatchInfo & b1, const BatchInfo & b2)
	{
		if(b1.switchIndex == b2.switchIndex)
		{
			return b1.lodIndex < b2.lodIndex;
		}

		return b1.switchIndex < b2.switchIndex;
	}
};

void CreateBatchesInfo(DAVA::RenderObject *object, DAVA::Vector<BatchInfo> & batches)
{
	if(!object) return;

	DAVA::uint32 batchesCount = object->GetRenderBatchCount();
	for(DAVA::uint32 i = 0; i < batchesCount; ++i)
	{
		BatchInfo info;
		info.batch = object->GetRenderBatch(i, info.lodIndex, info.switchIndex);
		batches.push_back(info);
	}

	std::sort(batches.begin(), batches.end(), SortBatches());
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
			DAVA::Vector<BatchInfo> fromBatches;
			CreateBatchesInfo(fromMeshes[m], fromBatches);

			DAVA::Vector<BatchInfo> toBatches;
			CreateBatchesInfo(toMeshes[m], toBatches);

			DAVA::uint32 rbFromCount = fromMeshes[m]->GetRenderBatchCount();
			DAVA::uint32 rbToCount = toMeshes[m]->GetRenderBatchCount();

			for(DAVA::uint32 from = 0, to = 0; from < rbFromCount && to < rbToCount; )
			{
				BatchInfo & fromBatch = fromBatches[from];
				BatchInfo & toBatch = toBatches[to];

				if(fromBatch.switchIndex == toBatch.switchIndex)
				{
					if(fromBatch.lodIndex <= toBatch.lodIndex)
					{
						for(DAVA::uint32 usedToIndex = to; usedToIndex < rbToCount; ++usedToIndex)
						{
							BatchInfo & usedToBatch = toBatches[usedToIndex];

                            if((fromBatch.switchIndex != usedToBatch.switchIndex))
                                break;

							DAVA::PolygonGroup *fromPG = fromBatch.batch->GetPolygonGroup();
							DAVA::PolygonGroup *toPG = usedToBatch.batch->GetPolygonGroup();

							DAVA::uint32 fromSize = fromPG->GetVertexCount() * fromPG->vertexStride;
							DAVA::uint32 toSize = toPG->GetVertexCount() * toPG->vertexStride;
							if((fromSize == toSize) && (0 == Memcmp(fromPG->meshData, toPG->meshData, fromSize)))
							{
								CopyLightmapSettings(fromBatch.batch->GetMaterial(), usedToBatch.batch->GetMaterial());
							}
						}

						++from;
					}
					else if(fromBatch.lodIndex < toBatch.lodIndex)
					{
						++from;
					}
					else
					{
						++to;
					}
				}
				else if(fromBatch.switchIndex < toBatch.switchIndex)
				{
					++from;
				}
				else
				{
					++to;
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

void StructureSystem::SearchEntityByRef(DAVA::Entity *parent, const DAVA::FilePath &refToOwner, DAVA::Set<DAVA::Entity *> &result)
{
	if(NULL != parent)
	{
		for(int i = 0; i < parent->GetChildrenCount(); ++i)
		{
			DAVA::Entity *entity = parent->GetChild(i);
			DAVA::KeyedArchive *arch = GetCustomPropertiesArchieve(entity);
            
            if(arch)
            {
                // if this entity has searched reference - add it to the set
                if(DAVA::FilePath(arch->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, "")) == refToOwner)
                {
                    result.insert(entity);
                    continue;
                }
            }

            // else continue searching in child entities
            SearchEntityByRef(entity, refToOwner, result);
		}
	}
}
