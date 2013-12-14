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
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"

// particles-related commands
#include "Commands2/ParticleEditorCommands.h"

EditorParticlesSystem::EditorParticlesSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{
	DAVA::RenderManager* rm = DAVA::RenderManager::Instance();
	
	renderState = rm->Derive3DRenderState(DAVA::RenderStateData::STATE_BLEND |
										  DAVA::RenderStateData::STATE_COLORMASK_ALL |
										  DAVA::RenderStateData::STATE_DEPTH_TEST);
}

EditorParticlesSystem::~EditorParticlesSystem()
{

}

void EditorParticlesSystem::Update(DAVA::float32 timeElapsed)
{

}

void EditorParticlesSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void EditorParticlesSystem::DrawDebugInfoForEmitter(DAVA::Entity* parentEntity)
{
	SceneCollisionSystem *collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	
	if (collisionSystem)
	{		
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.9f, 0.9f, 0.9f, 0.35f));		
		for(int i = 0; i < parentEntity->GetChildrenCount(); ++i)
		{
			DAVA::Entity *entity = parentEntity->GetChild(i);
			if(NULL != entity)
			{
				DAVA::AABBox3 wordBox;
				DAVA::AABBox3 collBox = collisionSystem->GetBoundingBox(entity);
				collBox.GetTransformedBox(entity->GetWorldTransform(), wordBox);	
				// Get sphere radius (size) of debug effect
				DAVA::float32 radius = (collBox.max - collBox.min).Length() / 3;
				DAVA::RenderHelper::Instance()->FillDodecahedron(wordBox.GetCenter(), radius);
			}
		}
		DAVA::RenderManager::Instance()->ResetColor();
	}
}

void EditorParticlesSystem::Draw()
{
	//int oldState = DAVA::RenderManager::Instance()->GetState();
	//DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	//DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	//DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_TEST);
	//DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);
	
	DAVA::RenderManager::Instance()->SetRenderState(renderState);
	DAVA::RenderManager::Instance()->FlushState();
	
	// Draw debug information for non-selected entities
	for(size_t i = 0; i < entities.size(); ++i)
	{				
		DrawDebugInfoForEmitter(entities[i]);
	}
	
	// Draw debug information for selected entities
	SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	if(selectionSystem != NULL)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f));
		
		for (size_t i = 0; i < selectionSystem->GetSelectionCount(); i++)
		{
			DAVA::Entity *entity = selectionSystem->GetSelectionEntity(i);
			
			DAVA::ParticleEmitter *emitter = GetEmitter(entity);
			// Draw additional effects according to emitter type
			if (emitter)
			{
				// Get center of entity object
				DAVA::AABBox3 selectionBox = selectionSystem->GetSelectionAABox(i);
				DAVA::Vector3 center = selectionBox.GetCenter();
				// Always draw emission vector arrow for emitter
				DrawVectorArrow(entity, emitter, center);

				switch (emitter->emitterType)
				{
				case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
				case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
					{
						DrawSizeCircle(entity, emitter, center);
					}
					break;
				case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
					{
						DrawSizeCircleShockWave(emitter, center);
					}
					break;

				case DAVA::ParticleEmitter::EMITTER_RECT:
					{
						DrawSizeBox(entity, emitter, center);
					}
					break;

				case DAVA::ParticleEmitter::EMITTER_POINT:
					{
						DAVA::RenderHelper::Instance()->FillDodecahedron(center, 0.05f);
					}
					break;
				}
			}
		}
		DAVA::RenderManager::Instance()->ResetColor();
	}
	
	//DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	//DAVA::RenderManager::Instance()->SetState(oldState);
}

void EditorParticlesSystem::DrawSizeCircleShockWave(DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	float32 emitterRadius = (emitter->radius) ? emitter->radius->GetValue(emitter->GetTime()) : 0.0f;
	DAVA::RenderHelper::Instance()->DrawCircle3D(center, DAVA::Vector3(0.0f, 0.0f, 1.0f), emitterRadius, true);
}

void EditorParticlesSystem::DrawSizeCircle(DAVA::Entity *entity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	float32 emitterRadius = 0.0f;
	DAVA::Vector3 emitterVector;
							
	if (emitter->radius)
	{
		emitterRadius = emitter->radius->GetValue(emitter->GetTime());
	}

	if (emitter->emissionVector)
	{
		DAVA::Matrix4 wMat = entity->GetWorldTransform();
		wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));

		emitterVector = emitter->emissionVector->GetValue(emitter->GetTime());
		emitterVector = emitterVector * wMat;
	}
							
	DAVA::RenderHelper::Instance()->DrawCircle3D(center, emitterVector, emitterRadius, true);
}

void EditorParticlesSystem::DrawSizeBox(DAVA::Entity *entity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	// Default value of emitter size
	DAVA::Vector3 emitterSize;

	DAVA::Vector3 p[8];
	
	if (emitter->size)
	{
		emitterSize = emitter->size->GetValue(emitter->GetTime());
	}
	
	float halfSizeX = emitterSize.x / 2;
	float halfSizeY = emitterSize.y / 2;
	float halfSizeZ = emitterSize.z / 2;
	
	// Calculate box min and max values
	p[0] = DAVA::Vector3(center.x + halfSizeX, center.y + halfSizeY, center.z - halfSizeZ);
	p[1] = DAVA::Vector3(center.x + halfSizeX, center.y + halfSizeY, center.z + halfSizeZ);
	p[2] = DAVA::Vector3(center.x - halfSizeX, center.y + halfSizeY, center.z + halfSizeZ);
	p[3] = DAVA::Vector3(center.x - halfSizeX, center.y + halfSizeY, center.z - halfSizeZ);

	p[4] = DAVA::Vector3(center.x + halfSizeX, center.y - halfSizeY, center.z - halfSizeZ);
	p[5] = DAVA::Vector3(center.x + halfSizeX, center.y - halfSizeY, center.z + halfSizeZ);
	p[6] = DAVA::Vector3(center.x - halfSizeX, center.y - halfSizeY, center.z + halfSizeZ);
	p[7] = DAVA::Vector3(center.x - halfSizeX, center.y - halfSizeY, center.z - halfSizeZ);

	DAVA::Matrix4 wMat = entity->GetWorldTransform();
	wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));

	for(int i = 0; i < 8; ++i)
	{
		p[i] = p[i] * wMat;
	}

	DAVA::Polygon3 poly;
	poly.AddPoint(p[0]);
	poly.AddPoint(p[1]);
	poly.AddPoint(p[2]);
	poly.AddPoint(p[3]);
	RenderHelper::Instance()->FillPolygon(poly);

	poly.Clear();
	poly.AddPoint(p[0]);
	poly.AddPoint(p[1]);
	poly.AddPoint(p[5]);
	poly.AddPoint(p[4]);
	RenderHelper::Instance()->FillPolygon(poly);

	poly.Clear();
	poly.AddPoint(p[1]);
	poly.AddPoint(p[2]);
	poly.AddPoint(p[6]);
	poly.AddPoint(p[5]);
	RenderHelper::Instance()->FillPolygon(poly);

	poly.Clear();
	poly.AddPoint(p[2]);
	poly.AddPoint(p[3]);
	poly.AddPoint(p[7]);
	poly.AddPoint(p[6]);
	RenderHelper::Instance()->FillPolygon(poly);

	poly.Clear();
	poly.AddPoint(p[0]);
	poly.AddPoint(p[3]);
	poly.AddPoint(p[7]);
	poly.AddPoint(p[4]);
	RenderHelper::Instance()->FillPolygon(poly);

	poly.Clear();
	poly.AddPoint(p[4]);
	poly.AddPoint(p[5]);
	poly.AddPoint(p[6]);
	poly.AddPoint(p[7]);
	RenderHelper::Instance()->FillPolygon(poly);
}

void EditorParticlesSystem::DrawVectorArrow(DAVA::Entity *entity, DAVA::ParticleEmitter *emitter, DAVA::Vector3 center)
{
	DAVA::Vector3 emitterVector(0.f, 0.f, 1.f);
	DAVA::float32 arrowBaseSize = 5.0f;
				
	if (emitter->emissionVector)
	{
		emitterVector = emitter->emissionVector->GetValue(emitter->GetTime());
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
	
	DAVA::Matrix4 wMat = entity->GetWorldTransform();
	wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));
	emitterVector = emitterVector * wMat;

	DAVA::RenderHelper::Instance()->FillArrow(center, emitterVector, arrowSize, 1);
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
			SceneSignals::Instance()->EmitParticleLayerAdded(activeScene, castedCmd->GetCreatedLayer());
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



