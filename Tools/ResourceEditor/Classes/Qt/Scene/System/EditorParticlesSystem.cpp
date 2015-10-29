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


#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"

// framework
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"

// particles-related commands
#include "Commands2/ParticleEditorCommands.h"

EditorParticlesSystem::EditorParticlesSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{
    selectedEffectEntity = NULL;
    selectedEmitter = NULL;
}

void EditorParticlesSystem::SetEmitterSelected(DAVA::Entity *effectEntity, DAVA::ParticleEmitter *emitter)
{
    selectedEffectEntity = effectEntity;
    selectedEmitter = emitter;
}

EditorParticlesSystem::~EditorParticlesSystem()
{

}

void EditorParticlesSystem::DrawDebugInfoForEffect(DAVA::Entity* effectEntity)
{
	SceneCollisionSystem *collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;

    if (collisionSystem)
    {
        if (NULL != effectEntity)
        {
			DAVA::AABBox3 wordBox;
			DAVA::AABBox3 collBox = collisionSystem->GetBoundingBox(effectEntity);
			collBox.GetTransformedBox(effectEntity->GetWorldTransform(), wordBox);	
			// Get sphere radius (size) of debug effect
			DAVA::float32 radius = (collBox.max - collBox.min).Length() / 3;
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(wordBox.GetCenter(), radius, DAVA::Color(0.9f, 0.9f, 0.9f, 0.35f), RenderHelper::DRAW_SOLID_DEPTH);
        }
    }
}

void EditorParticlesSystem::Draw()
{
	// Draw debug information for non-selected entities
	for(size_t i = 0; i < entities.size(); ++i)
	{				
		DrawDebugInfoForEffect(entities[i]);
	}
	
	// Draw debug information for selected entities
	if ((selectedEmitter!=NULL) && (selectedEffectEntity!=NULL))
    {
			
			
		// Draw additional effects according to emitter type
        DAVA::Matrix3 effectMatrix(selectedEffectEntity->GetWorldTransform());
        ParticleEffectComponent * effect = GetEffectComponent(selectedEffectEntity);
        DAVA::Vector3 center =effect->GetSpawnPosition(effect->GetEmitterId(selectedEmitter));
        TransformPerserveLength(center, effectMatrix);
        center+=selectedEffectEntity->GetWorldTransform().GetTranslationVector();

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(center, 0.1f, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        DrawVectorArrow(selectedEffectEntity, selectedEmitter, center);

        switch (selectedEmitter->emitterType)
		{
		case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
		case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
			{
				DrawSizeCircle(selectedEffectEntity, selectedEmitter, center);
			}
			break;
		case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
			{
				DrawSizeCircleShockWave(selectedEffectEntity, selectedEmitter, center);
			}
			break;

		case DAVA::ParticleEmitter::EMITTER_RECT:
			{
				DrawSizeBox(selectedEffectEntity, selectedEmitter, center);
			}
			break;
                
        default: break;
		}
    }
}

void EditorParticlesSystem::DrawSizeCircleShockWave(DAVA::Entity *effectEntity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	//float32 time = emitter->GetTime();
	float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
	float32 emitterRadius = (emitter->radius) ? emitter->radius->GetValue(time) : 0.0f;
    Vector3 emissionVector(0,0,1);
    if (emitter->emissionVector)
        emissionVector = emitter->emissionVector->GetValue(time);

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawCircle(center, emissionVector, emitterRadius, 12, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeCircle(DAVA::Entity *effectEntity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	float32 emitterRadius = 0.0f;
	DAVA::Vector3 emitterVector;	
	float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
							
	if (emitter->radius)
	{
		emitterRadius = emitter->radius->GetValue(time);
	}

	if (emitter->emissionVector)
	{
		DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
		wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));

		emitterVector = emitter->emissionVector->GetValue(time);
		emitterVector = emitterVector * wMat;
	}

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawCircle(center, emitterVector, emitterRadius, 12, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeBox(DAVA::Entity *effectEntity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	// Default value of emitter size
	DAVA::Vector3 emitterSize;

	DAVA::Vector3 p[8];
	float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
	
	if (emitter->size)
	{
		emitterSize = emitter->size->GetValue(time);
	}
	
	DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
	wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));

    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawAABoxTransformed(AABBox3(center - emitterSize / 2, center + emitterSize / 2), wMat, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawVectorArrow(DAVA::Entity *effectEntity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	DAVA::Vector3 emitterVector(0.f, 0.f, 1.f);
	DAVA::float32 arrowBaseSize = 5.0f;
				
	float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
	if (emitter->emissionVector)
	{
		emitterVector = emitter->emissionVector->GetValue(time);
	}
	
	DAVA::float32 scale = 1.0f;
	// Get current scale from HoodSystem
	HoodSystem *hoodSystem = ((SceneEditor2 *) GetScene())->hoodSystem;
	if(hoodSystem != NULL)
	{
		scale = hoodSystem->GetScale();
	}
	
	emitterVector.Normalize();
	
	DAVA::float32 arrowSize = scale;
	emitterVector = (emitterVector * arrowBaseSize * scale) + center;
	
	DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
	wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));
	emitterVector = emitterVector * wMat;

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, emitterVector, arrowSize, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::AddEntity(DAVA::Entity * entity)
{
	entities.push_back(entity);
}

void EditorParticlesSystem::RemoveEntity(DAVA::Entity * entity)
{	
    int size = entities.size();
	for(int i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}
}

void EditorParticlesSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if (!command)
	{
		return;
	}

	// Notify that the Particles-related value is changed.
	SceneEditor2* activeScene = (SceneEditor2 *) GetScene();
	switch (command->GetId())
	{
		case CMDID_PARTICLE_EMITTER_UPDATE:
		{
			const CommandUpdateEmitter* castedCmd = static_cast<const CommandUpdateEmitter*>(command);
			SceneSignals::Instance()->EmitParticleEmitterValueChanged(activeScene,
																	  castedCmd->GetEmitter());
			break;
		}

		case CMDID_PARTICLE_LAYER_UPDATE:
		case CMDID_PARTILCE_LAYER_UPDATE_TIME:
		case CMDID_PARTICLE_LAYER_UPDATE_ENABLED:
		{
			const CommandUpdateParticleLayerBase* castedCmd = static_cast<const CommandUpdateParticleLayerBase*>(command);
			SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene,
																	  castedCmd->GetLayer());
			break;
		}

		case CMDID_PARTICLE_FORCE_UPDATE:
		{
			const CommandUpdateParticleForce* castedCmd = static_cast<const CommandUpdateParticleForce*>(command);
			SceneSignals::Instance()->EmitParticleForceValueChanged(activeScene,
																	castedCmd->GetLayer(),
																	castedCmd->GetForceIndex());
			break;
		}

		case CMDID_PARTICLE_EFFECT_START_STOP:
		{
			const CommandStartStopParticleEffect* castedCmd = static_cast<const CommandStartStopParticleEffect*>(command);
			SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene,
																	 castedCmd->GetEntity(),
																	 castedCmd->GetStarted());
			break;
		}
			
		case CMDID_PARTICLE_EFFECT_RESTART:
		{
			const CommandRestartParticleEffect* castedCmd = static_cast<const CommandRestartParticleEffect*>(command);
			
			// An effect was stopped and then started.
			SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene,
																	 castedCmd->GetEntity(),
																	 false);
			SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene,
																	 castedCmd->GetEntity(),
																	 true);
			break;
		}

		case CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML:
		{
			const CommandLoadParticleEmitterFromYaml* castedCmd = static_cast<const CommandLoadParticleEmitterFromYaml*>(command);
			SceneSignals::Instance()->EmitParticleEmitterLoaded(activeScene, castedCmd->GetEmitter());
			break;
		}

		case CMDID_PARTICLE_EMITTER_SAVE_TO_YAML:
		{
			const CommandSaveParticleEmitterToYaml* castedCmd = static_cast<const CommandSaveParticleEmitterToYaml*>(command);            
			SceneSignals::Instance()->EmitParticleEmitterSaved(activeScene, castedCmd->GetEmitter());
			break;
		}

		case CMDID_PARTICLE_EMITTER_LAYER_ADD:
		{
			const CommandAddParticleEmitterLayer* castedCmd = static_cast<const CommandAddParticleEmitterLayer*>(command);
			SceneSignals::Instance()->EmitParticleLayerAdded(activeScene, castedCmd->GetParentEmitter(), castedCmd->GetCreatedLayer());
			break;
		}
// Return to this code when implementing Layer popup menus.
/*
		case CMDID_REMOVE_PARTICLE_EMITTER_LAYER:
		{
			const CommandRemoveParticleEmitterLayer* castedCmd = static_cast<const CommandRemoveParticleEmitterLayer*>(command);
			SceneSignals::Instance()->EmitParticleLayerRemoved(activeScene, castedCmd->GetEmitter());
			break;
		}
*/
		default:
		{
			break;
		}
	}
}



