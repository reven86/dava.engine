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



#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneGraph.h"

#include "DockParticleEditor/ParticlesEditorController.h"
#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/InnerEmitterParticleEditorNode.h"

#include "ParticlesEditorQT/Helpers/ParticlesEditorNodeNameHelper.h"

#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "SceneEditor/EditorSettings.h"
#include "StringConstants.h"

#include <QFileDialog>
#include <QString>

#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandUpdateEffect::CommandUpdateEffect(ParticleEffectComponent* particleEffect):
	CommandAction(CMDID_PARTICLE_EFFECT_UPDATE)
{
	this->particleEffect = particleEffect;
}

void CommandUpdateEffect::Init(float32 playbackSpeed)
{
	this->playbackSpeed = playbackSpeed;	
}

void CommandUpdateEffect::Redo()
{
	DVASSERT(particleEffect);
	particleEffect->SetPlaybackSpeed(playbackSpeed);	
}

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitter* emitter):
	CommandAction(CMDID_PARTICLE_EMITTER_UPDATE)
{
	this->emitter = emitter;
}

void CommandUpdateEmitter::Init(ParticleEmitter::eType emitterType,
								RefPtr<PropertyLine<float32> > emissionRange,
								RefPtr<PropertyLine<Vector3> > emissionVector,
								RefPtr<PropertyLine<float32> > radius,
								RefPtr<PropertyLine<Color> > colorOverLife,
								RefPtr<PropertyLine<Vector3> > size,
								float32 life,
								float32 playbackSpeed,
								bool isShortEffect)
{
	this->emitterType = emitterType;
	this->emissionRange = emissionRange;
	this->emissionVector = emissionVector;
	this->radius = radius;
	this->colorOverLife = colorOverLife;
	this->size = size;
	this->life = life;
	this->playbackSpeed = playbackSpeed;
	this->isShortEffect = isShortEffect;
}

void CommandUpdateEmitter::Redo()
{
	DVASSERT(emitter);

	emitter->emitterType = emitterType;
	PropertyLineHelper::SetValueLine(emitter->emissionRange, emissionRange);
	PropertyLineHelper::SetValueLine(emitter->emissionVector, emissionVector);
	PropertyLineHelper::SetValueLine(emitter->radius, radius);
	PropertyLineHelper::SetValueLine(emitter->colorOverLife, colorOverLife);
	PropertyLineHelper::SetValueLine(emitter->size, size);
	emitter->SetLifeTime(life);
	emitter->SetPlaybackSpeed(playbackSpeed);
	emitter->SetShortEffect(isShortEffect);
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer) :
	CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE)
{
	this->emitter = emitter;
	this->layer = layer;
}

void CommandUpdateParticleLayer::Init(const String& layerName,
									  ParticleLayer::eType layerType,
									  bool isDisabled,									  
									  bool inheritPosition,
									  bool isLong,
									  float32 scaleVelocityBase,
									  float32 scaleVelocityFactor,
									  bool isLooped,
									  Sprite* sprite,
									  eBlendMode srcFactor,
									  eBlendMode dstFactor,
									  bool enableFog,
									  bool enableFrameBlending,
									  int32 particleOrientation,
									  RefPtr< PropertyLine<float32> > life,
									  RefPtr< PropertyLine<float32> > lifeVariation,
									  RefPtr< PropertyLine<float32> > number,
									  RefPtr< PropertyLine<float32> > numberVariation,
									  RefPtr< PropertyLine<Vector2> > size,
									  RefPtr< PropertyLine<Vector2> > sizeVariation,
									  RefPtr< PropertyLine<Vector2> > sizeOverLife,
									  RefPtr< PropertyLine<float32> > velocity,
									  RefPtr< PropertyLine<float32> > velocityVariation,
									  RefPtr< PropertyLine<float32> > velocityOverLife,
									  RefPtr< PropertyLine<float32> > spin,
									  RefPtr< PropertyLine<float32> > spinVariation,
									  RefPtr< PropertyLine<float32> > spinOverLife,
									  bool randomSpinDirection,

									  RefPtr< PropertyLine<Color> > colorRandom,
									  RefPtr< PropertyLine<float32> > alphaOverLife,
									  RefPtr< PropertyLine<Color> > colorOverLife,
									  RefPtr< PropertyLine<float32> > angle,
									  RefPtr< PropertyLine<float32> > angleVariation,

									  float32 startTime,
									  float32 endTime,
									  float32 deltaTime,
									  float32 deltaVariation,
									  float32 loopEndTime,
									  float32 loopVariation,
									  bool frameOverLifeEnabled,
									  float32 frameOverLifeFPS,
									  bool randomFrameOnStart,
									  bool loopSpriteAnimation,
									  RefPtr< PropertyLine<float32> > animSpeedOverLife,
									  
									  float32 pivotPointX,
									  float32 pivotPointY)
{
	this->layerName = layerName;
	this->layerType = layerType;
	this->isDisabled = isDisabled;	
	this->inheritPosition = inheritPosition;
	this->isLooped = isLooped;
	this->isLong = isLong;
	this->scaleVelocityBase = scaleVelocityBase;
	this->scaleVelocityFactor = scaleVelocityFactor;
	this->sprite = sprite;
	this->srcFactor = srcFactor;
	this->dstFactor = dstFactor;
	this->enableFog = enableFog;
	this->enableFrameBlending = enableFrameBlending;
	this->life = life;
	this->lifeVariation = lifeVariation;
	this->number = number;
	this->numberVariation = numberVariation;
	this->size = size;
	this->sizeVariation = sizeVariation;
	this->sizeOverLife = sizeOverLife;
	this->velocity = velocity;
	this->velocityVariation = velocityVariation;
	this->velocityOverLife = velocityOverLife;
	this->spin = spin;
	this->spinVariation = spinVariation;
	this->spinOverLife = spinOverLife;
	this->randomSpinDirection = randomSpinDirection;
	this->particleOrientation = particleOrientation;

	this->colorRandom = colorRandom;
	this->alphaOverLife = alphaOverLife;
	this->colorOverLife = colorOverLife;	
	this->angle = angle;
	this->angleVariation = angleVariation;

	this->startTime = startTime;
	this->endTime = endTime;
	this->deltaTime = deltaTime;
	this->deltaVariation = deltaVariation;
	this->loopEndTime = loopEndTime;
	this->loopVariation = loopVariation;
	this->frameOverLifeEnabled = frameOverLifeEnabled;
	this->frameOverLifeFPS = frameOverLifeFPS;
	this->randomFrameOnStart = randomFrameOnStart;
	this->loopSpriteAnimation = loopSpriteAnimation;
	this->animSpeedOverLife = animSpeedOverLife;
	
	this->pivotPointX = pivotPointX;
	this->pivotPointY = pivotPointY;
}


void CommandUpdateParticleLayer::Redo()
{
	layer->layerName = layerName;
	layer->SetDisabled(isDisabled);	
	layer->SetInheritPosition(inheritPosition);
	layer->SetLong(isLong);
	layer->scaleVelocityBase = scaleVelocityBase;
	layer->scaleVelocityFactor = scaleVelocityFactor;
	layer->SetLooped(isLooped);
	layer->SetBlendMode(srcFactor, dstFactor);
	layer->SetFog(enableFog);
	layer->SetFrameBlend(enableFrameBlending);
	PropertyLineHelper::SetValueLine(layer->life , life);
	PropertyLineHelper::SetValueLine(layer->lifeVariation, lifeVariation);
	PropertyLineHelper::SetValueLine(layer->number, number);
	PropertyLineHelper::SetValueLine(layer->numberVariation, numberVariation);
	PropertyLineHelper::SetValueLine(layer->size, size);
	PropertyLineHelper::SetValueLine(layer->sizeVariation, sizeVariation);
	PropertyLineHelper::SetValueLine(layer->sizeOverLifeXY, sizeOverLife);
	PropertyLineHelper::SetValueLine(layer->velocity, velocity);
	PropertyLineHelper::SetValueLine(layer->velocityVariation, velocityVariation);
	PropertyLineHelper::SetValueLine(layer->velocityOverLife, velocityOverLife);
	PropertyLineHelper::SetValueLine(layer->spin, spin);
	PropertyLineHelper::SetValueLine(layer->spinVariation, spinVariation);
	PropertyLineHelper::SetValueLine(layer->spinOverLife, spinOverLife);
	layer->randomSpinDirection = randomSpinDirection;
	layer->particleOrientation = particleOrientation;

	PropertyLineHelper::SetValueLine(layer->colorRandom, colorRandom);
	PropertyLineHelper::SetValueLine(layer->alphaOverLife, alphaOverLife);
	PropertyLineHelper::SetValueLine(layer->colorOverLife, colorOverLife);
	
	layer->frameOverLifeEnabled = frameOverLifeEnabled;
	layer->frameOverLifeFPS = frameOverLifeFPS;
	layer->randomFrameOnStart = randomFrameOnStart;
	layer->loopSpriteAnimation = loopSpriteAnimation;
	PropertyLineHelper::SetValueLine(layer->animSpeedOverLife, animSpeedOverLife);

	PropertyLineHelper::SetValueLine(layer->angle, angle);
	PropertyLineHelper::SetValueLine(layer->angleVariation, angleVariation);

	layer->UpdateLayerTime(startTime, endTime);	
	layer->deltaTime = deltaTime;
	layer->deltaVariation = deltaVariation;
	layer->loopEndTime = loopEndTime;
	layer->loopVariation = loopVariation;
	
	layer->SetPivotPoint(Vector2(pivotPointX, pivotPointY));

	// This code must be after layer->frameOverlife set call, since setSprite
	// may change the frames.
	if (layer->GetSprite() != sprite)
	{
		emitter->Stop();
		layer->SetSprite(sprite);
		emitter->Play();
	}
	
	// The same is for emitter type.
	if (layer->type != layerType)
	{
		emitter->Stop();
		if (layerType == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
		{
			layer->RemoveInnerEmitter();
		}
		layer->type = layerType;		
		if (layerType == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
		{
			layer->CreateInnerEmitter();
			if (!layer->innerEmitterPath.IsEmpty())
			{
				layer->GetInnerEmitter()->LoadFromYaml(layer->innerEmitterPath);				
			}
			layer->GetInnerEmitter()->SetPlaybackSpeed(layer->GetPlaybackSpeed());
		}
		emitter->Play();
	}
	
	// "IsLong" flag.
	if (layer->IsLong() != isLong)
	{
		emitter->Stop();
		layer->SetLong(isLong);
		emitter->Play();
	}

	// In case we are switching to "SuperEmitter" type - need to create Inner Emitter
	// for the layer.
	if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES && !layer->GetInnerEmitter())
	{
		layer->CreateInnerEmitter();
	}

	// TODO: Yuri Coder, 2013/07/23. Refresh Particle Layer is needed for Superemitter.
	// Currently it is temporarily commented out but have to be recovered soon.
	// SceneDataManager::Instance()->RefreshParticlesLayer(layer);
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer) :
	CommandUpdateParticleLayerBase(CMDID_PARTILCE_LAYER_UPDATE_TIME)
{
	this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
	this->startTime = startTime;
	this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Redo()
{
	layer->UpdateLayerTime(startTime, endTime);	
}

CommandUpdateParticleLayerEnabled::CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled) :
	CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE_ENABLED)
{
	this->layer = layer;
	this->isEnabled = isEnabled;
}

void CommandUpdateParticleLayerEnabled::Redo()
{
	if (this->layer)
	{
		this->layer->SetDisabled(!isEnabled);
		//ParticlesEditorController::Instance()->RefreshSelectedNode(true); //looks like depricated
	}
}

CommandUpdateParticleLayerLods::CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods) :
CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE_LODS)
{
	this->layer = layer;
	this->lods = lods;
}

void CommandUpdateParticleLayerLods::Redo()
{
	if (this->layer)
	{
		for (size_t i=0; i<lods.size(); i++)
		{
			this->layer->SetLodActive(i, lods[i]);
		}		
		//ParticlesEditorController::Instance()->RefreshSelectedNode(true); //looks like depricated
	}
}

CommandUpdateParticleForce::CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId) :
	CommandAction(CMDID_PARTICLE_FORCE_UPDATE)
{
	this->layer = layer;
	this->forceId = forceId;
}

void CommandUpdateParticleForce::Init(RefPtr< PropertyLine<Vector3> > force,
									  RefPtr< PropertyLine<Vector3> > forcesVariation,
									RefPtr< PropertyLine<float32> > forcesOverLife)
{
	PropertyLineHelper::SetValueLine(this->force,force);
	PropertyLineHelper::SetValueLine(this->forcesVariation, forcesVariation);
	PropertyLineHelper::SetValueLine(this->forcesOverLife, forcesOverLife);
}

void CommandUpdateParticleForce::Redo()
{
	layer->UpdateForce(forceId, force, forcesVariation, forcesOverLife);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandAddParticleEmitter::CommandAddParticleEmitter(DAVA::Entity* effect) :
    CommandAction(CMDID_PARTICLE_EMITTER_ADD)
{
	this->effectEntity = effect;
}

void CommandAddParticleEmitter::Redo()
{
	if (!effectEntity)
	{
		return;
	}
	
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);

	ParticleEmitter3D* newEmitter = new ParticleEmitter3D();
	RenderComponent * renderComponent = new RenderComponent();
	renderComponent->SetRenderObject(newEmitter);
	newEmitter->Release();

	Entity* emitterEntity = new Entity();
	emitterEntity->SetName("Particle Emitter");
	emitterEntity->AddComponent(renderComponent);
	renderComponent->Release();

	emitterEntity->AddComponent(ScopedPtr<LodComponent> (new LodComponent()));
	effectEntity->AddNode(emitterEntity);
}

CommandStartStopParticleEffect::CommandStartStopParticleEffect(DAVA::Entity* effect, bool isStart) :
    CommandAction(CMDID_PARTICLE_EFFECT_START_STOP)
{
    this->isStart = isStart;
	this->effectEntity = effect;
}

void CommandStartStopParticleEffect::Redo()
{
	if (!effectEntity)
	{
		return;
	}

	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);

    if (this->isStart)
    {
        effectComponent->Start();
    }
    else
    {
        effectComponent->Stop();
    }
}

DAVA::Entity* CommandStartStopParticleEffect::GetEntity() const
{
	return this->effectEntity;
}

CommandRestartParticleEffect::CommandRestartParticleEffect(DAVA::Entity* effect) :
    CommandAction(CMDID_PARTICLE_EFFECT_RESTART)
{
	this->effectEntity = effect;
}

void CommandRestartParticleEffect::Redo()
{
	if (!effectEntity)
	{
		return;
	}
	
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
    effectComponent->Restart();
}

DAVA::Entity* CommandRestartParticleEffect::GetEntity() const
{
	return this->effectEntity;
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer(ParticleEmitter* emitter) :
    CommandAction(CMDID_PARTICLE_EMITTER_LAYER_ADD)
{
	this->selectedEmitter = emitter;
	this->createdLayer = NULL;
}

void CommandAddParticleEmitterLayer::Redo()
{
	static const float32 LIFETIME_FOR_NEW_PARTICLE_EMITTER = 4.0f;
	if (!selectedEmitter)
	{
		return;
	}

	bool isStopped = selectedEmitter->IsStopped();
	if (!isStopped)
	{
		selectedEmitter->Stop();
	}

    // Create the new layer. NOTE: only 3D layers are supported!
	DVASSERT(selectedEmitter->GetIs3D());
    createdLayer = new ParticleLayer3D(selectedEmitter);

	createdLayer->startTime = 0;
    createdLayer->endTime = LIFETIME_FOR_NEW_PARTICLE_EMITTER;
	createdLayer->life = new PropertyLineValue<float32>(selectedEmitter->GetLifeTime());
    createdLayer->layerName = ParticlesEditorNodeNameHelper::GetNewLayerName(ResourceEditor::LAYER_NODE_NAME, selectedEmitter);

	createdLayer->SetLoopEndTime(selectedEmitter->GetLifeTime());	
    selectedEmitter->AddLayer(createdLayer);

	if (!isStopped)
	{
		selectedEmitter->Restart();
	}
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer(ParticleLayer* layer) :
    CommandAction(CMDID_PARTICLE_EMITTER_LAYER_REMOVE)
{
	this->selectedLayer = layer;
}

void CommandRemoveParticleEmitterLayer::Redo()
{
	if (!selectedLayer)
	{
		return;
	}
	
	ParticleEmitter* emitter = selectedLayer->GetEmitter();
    if (!emitter)
    {
        return;
    }

	bool isStopped = emitter->IsStopped();
	if (!isStopped)
	{
		emitter->Stop();
	}

	emitter->RemoveLayer(selectedLayer);
    
	if (!isStopped)
	{
		emitter->Restart();
	}
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer(ParticleLayer* layer) :
	CommandAction(CMDID_PARTICLE_EMITTER_LAYER_CLONE)
{
	this->selectedLayer = layer;
}

void CommandCloneParticleEmitterLayer::Redo()
{
	if (!selectedLayer)
	{
		return;
	}

	ParticleEmitter* emitter = selectedLayer->GetEmitter();
    if (!emitter)
    {
        return;
    }

    ParticleLayer* clonedLayer = selectedLayer->Clone();
	clonedLayer->layerName = selectedLayer->layerName + " Clone";
    emitter->AddLayer(clonedLayer);
}

CommandAddParticleEmitterForce::CommandAddParticleEmitterForce(ParticleLayer* layer) :
    CommandAction(CMDID_PARTICLE_EMITTER_FORCE_ADD)
{
	this->selectedLayer = layer;
}

void CommandAddParticleEmitterForce::Redo()
{
	if (!selectedLayer)
	{
		return;
	}

	ParticleEmitter* emitter = selectedLayer->GetEmitter();
    if (!emitter)
    {
        return;
    }
	
	bool isStopped = emitter->IsStopped();
	if (!isStopped)
	{
		emitter->Stop();
	}
	
    // Add the new Force to the Layer.
	ParticleForce* newForce = new ParticleForce(RefPtr<PropertyLine<Vector3> >(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))),
												RefPtr<PropertyLine<Vector3> >(NULL), RefPtr<PropertyLine<float32> >(NULL));
	selectedLayer->AddForce(newForce);
	newForce->Release();

	if (!isStopped)
	{
		emitter->Restart();
	}
}

CommandRemoveParticleEmitterForce::CommandRemoveParticleEmitterForce(ParticleLayer* layer, ParticleForce* force) :
    CommandAction(CMDID_PARTICLE_EMITTER_FORCE_REMOVE)
{
	this->selectedLayer = layer;
	this->selectedForce = force;
}

void CommandRemoveParticleEmitterForce::Redo()
{
	if (!selectedLayer || !selectedForce)
	{
		return;
	}
	
	ParticleEmitter* emitter = selectedLayer->GetEmitter();
    if (!emitter)
    {
        return;
    }

	bool isStopped = emitter->IsStopped();
	if (!isStopped)
	{
		emitter->Stop();
	}
	
	selectedLayer->RemoveForce(selectedForce);
	
	if (!isStopped)
	{
		emitter->Restart();
	}
}

CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml(ParticleEmitter* emitter, const FilePath& path) :
	CommandAction(CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML)
{
	this->selectedEmitter = emitter;
	this->filePath = path;
}

void CommandLoadParticleEmitterFromYaml::Redo()
{
    if(!selectedEmitter)
    {
    	return;
    }

	bool isStopped = selectedEmitter->IsStopped();
	if (!isStopped)
	{
		selectedEmitter->Stop();
	}

    selectedEmitter->LoadFromYaml(filePath);

	if (!isStopped)
	{
		selectedEmitter->Restart();
	}
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(ParticleEmitter* emitter, const FilePath& path) :
	CommandAction(CMDID_PARTICLE_EMITTER_SAVE_TO_YAML)
{
	this->selectedEmitter = emitter;
	this->filePath = path;
}

void CommandSaveParticleEmitterToYaml::Redo()
{
	if (!selectedEmitter)
	{
		return;
	}
	
	selectedEmitter->SaveToYaml(filePath);
}
/*
CommandLoadInnerEmitterFromYaml::CommandLoadInnerEmitterFromYaml() :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_LOAD_INNER_EMITTER_FROM_YAML)
{
}

void CommandLoadInnerEmitterFromYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    InnerEmitterParticleEditorNode* innerEmitterNode = dynamic_cast<InnerEmitterParticleEditorNode*>(selectedNode);
    if (!innerEmitterNode || !innerEmitterNode->GetInnerEmitter() ||!innerEmitterNode->GetParticleLayer())
    {
        return;
    }
    
    QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                    projectPath, QString("YAML File (*.yaml)"));
	if (filePath.isEmpty())
    {
		return;
    }

    ParticleEmitter* innerEmitter = innerEmitterNode->GetInnerEmitter();
    innerEmitter->LoadFromYaml(filePath.toStdString());

	// No additional NULL check is needed here - already performed at the beginning.
	QFileInfo fileInfo(filePath);
	ParticleLayer* innerEmitterLayer = innerEmitterNode->GetParticleLayer();
	innerEmitterLayer->innerEmitterPath = FilePath(fileInfo.path().toStdString(), fileInfo.fileName().toStdString());

	// Perform the validation of the Yaml file loaded.
	String validationMessage;
	if (ParticlesEditorSceneDataHelper::ValidateParticleEmitter(innerEmitter, validationMessage) == false)
	{
		ShowErrorDialog(validationMessage);
	}

	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandSaveInnerEmitterToYaml::CommandSaveInnerEmitterToYaml(bool forceAskFilename) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_INNER_EMITTER_TO_YAML)
{
    this->forceAskFilename = forceAskFilename;
}

void CommandSaveInnerEmitterToYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    InnerEmitterParticleEditorNode* emitterNode = dynamic_cast<InnerEmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetInnerEmitter())
    {
        return;
    }
	
    ParticleEmitter * emitter = emitterNode->GetInnerEmitter();
	FilePath yamlPath = emitter->GetConfigPath();
    if (this->forceAskFilename || yamlPath.IsEmpty() )
    {
        QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
		
        if (filePath.isEmpty())
        {
            return;
        }
        
        yamlPath = FilePath(filePath.toStdString());
    }

    emitter->SaveToYaml(yamlPath);
}
 */
