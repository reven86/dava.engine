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


#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Platform/SystemTimer.h"
#include "Utils/Random.h"
#include "Core/PerformanceSettings.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Render/Material/NMaterialNames.h"
#include "Particles/ParticleRenderObject.h"
#include "Debug/Stats.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/RenderPassNames.h"

namespace DAVA
{
NMaterial* ParticleEffectSystem::GetMaterial(Texture* texture, bool enableFog, bool enableFrameBlend, eBlending blending)
{
    if (!texture) //for superemitter particles eg
        return NULL;

    uint64 materialKey = blending;
    if (enableFog)
        materialKey += 1 << 4;
    if (enableFrameBlend)
        materialKey += 1 << 5;
    materialKey += (uint32)texture->handle << 6;

    Map<uint64, NMaterial*>::iterator it = materialMap.find(materialKey);
    if (it != materialMap.end()) //return existing
    {
        return (*it).second;
    }
    else //create new
    {
        NMaterial* material = new NMaterial();
        material->SetParent(particleBaseMaterial);

        if (enableFrameBlend)
            material->AddFlag(NMaterialFlagName::FLAG_FRAME_BLEND, 1);

        if ((!enableFog) || (is2DMode)) //inverse logic to suspend vertex fog inherited from global material
            material->AddFlag(NMaterialFlagName::FLAG_VERTEXFOG, 0);

        if (is2DMode)
            material->AddFlag(NMaterialFlagName::FLAG_FORCE_2D_MODE, 1);

        material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, texture);
        material->AddFlag(NMaterialFlagName::FLAG_BLENDING, blending);

        materialMap[materialKey] = material;

        material->PreBuildMaterial(PASS_FORWARD);

        return material;
    }
}

ParticleEffectSystem::ParticleEffectSystem(Scene* scene, bool _is2DMode)
    : SceneSystem(scene)
    , allowLodDegrade(false)
    , is2DMode(_is2DMode)
{	
    if (scene) //for 2d particles there would be no scene
    {
	    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::START_PARTICLE_EFFECT);
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STOP_PARTICLE_EFFECT);
    }

    particleBaseMaterial = new NMaterial();
    particleBaseMaterial->SetFXName(NMaterialName::PARTICLES);
}
ParticleEffectSystem::~ParticleEffectSystem()
{
    for (Map<uint64, NMaterial *>::iterator it = materialMap.begin(), e = materialMap.end(); it != e; ++it)
    {
        SafeRelease(it->second);
    }
    SafeRelease(particleBaseMaterial);
}

void ParticleEffectSystem::SetGlobalMaterial(NMaterial *material)
{
    particleBaseMaterial->SetParent(material);
}

void ParticleEffectSystem::PrebuildMaterials(ParticleEffectComponent* component)
{
    for (auto emitter : component->emitters)
    {
        for (auto layer : emitter->layers)
        {
            if (layer->sprite && (layer->type != ParticleLayer::TYPE_SUPEREMITTER_PARTICLES))
                GetMaterial(layer->sprite->GetTexture(0), layer->enableFog, layer->enableFrameBlend, layer->blending);
        }
    }
}

void ParticleEffectSystem::RunEmitter(ParticleEffectComponent *effect, ParticleEmitter *emitter, const Vector3& spawnPosition, int32 positionSource)
{
	for (size_t layerId=0, layersCount = emitter->layers.size(); layerId<layersCount; ++layerId)
	{
		ParticleLayer *layer = emitter->layers[layerId];
		bool isLodActive = layer->IsLodActive(effect->activeLodLevel);
		if ((!isLodActive)&&emitter->shortEffect) //layer could never become active
			continue; 
		ParticleGroup group;
		group.emitter = SafeRetain(emitter);
		group.layer = SafeRetain(layer);
        group.spawnPosition = spawnPosition;
		group.visibleLod = isLodActive;
		group.positionSource = positionSource;
		//prepare 1st loop info - so even not looped layers will follow common logic
		group.loopStartTime = 0;
		group.loopLyaerStartTime = group.layer->startTime;
		group.loopDuration = group.layer->endTime;		

		if (layer->sprite&&(layer->type != ParticleLayer::TYPE_SUPEREMITTER_PARTICLES))
            group.material = GetMaterial(layer->sprite->GetTexture(0), layer->enableFog, layer->enableFrameBlend, layer->blending);
        else
            group.material = NULL;

        effect->effectData.groups.push_back(group);
    }
}

void ParticleEffectSystem::RunEffect(ParticleEffectComponent *effect)
{	
    Scene *scene = GetScene();
    
    if (scene)
        scene->lodSystem->ForceUpdate(effect->GetEntity(), scene->GetCurrentCamera(), 1.0f/60.0f);
    if (effect->activeLodLevel!=effect->desiredLodLevel)
        UpdateActiveLod(effect);

	if (effect->effectData.groups.empty()) //clean position sources
		effect->effectData.infoSources.resize(1);
	//create particle groups
	for (size_t emitterId = 0, emittersCount = effect->emitters.size(); emitterId<emittersCount; ++emitterId)
	{
		RunEmitter(effect, effect->emitters[emitterId], effect->spawnPositions[emitterId]);		
	}
		
	effect->state = ParticleEffectComponent::STATE_PLAYING;
	effect->time = 0;
}

void ParticleEffectSystem::AddToActive(ParticleEffectComponent *effect)
{
    if (effect->state==ParticleEffectComponent::STATE_STOPPED)
    {
        //add to active effects and to render
        activeComponents.push_back(effect);        
        for (Map<String, float32>::iterator it = globalExternalValues.begin(), e = globalExternalValues.end(); it!=e; ++it)
            effect->SetExtertnalValue((*it).first, (*it).second);
        Scene *scene = GetScene();
        if (scene)
        {
            Matrix4 * worldTransformPointer = ((TransformComponent*)effect->GetEntity()->GetComponent(Component::TRANSFORM_COMPONENT))->GetWorldTransformPtr();
            effect->effectRenderObject->SetWorldTransformPtr(worldTransformPointer);
            Vector3 pos = worldTransformPointer->GetTranslationVector();
            effect->effectRenderObject->SetAABBox(AABBox3(pos, pos));
            scene->GetRenderSystem()->RenderPermanent(effect->effectRenderObject);
        }

    }
}

void ParticleEffectSystem::RemoveFromActive(ParticleEffectComponent *effect)
{
	Vector<ParticleEffectComponent*>::iterator it = std::find(activeComponents.begin(), activeComponents.end(), effect);
	DVASSERT(it!=activeComponents.end());
	activeComponents.erase(it);	
	effect->state = ParticleEffectComponent::STATE_STOPPED;	
    Scene *scene = GetScene();
    if (scene)
        scene->GetRenderSystem()->RemoveFromRender(effect->effectRenderObject);
}

void ParticleEffectSystem::AddEntity(Entity* entity)
{
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    PrebuildMaterials(effect);
}
void ParticleEffectSystem::AddComponent(Entity* entity, Component* component)
{
    ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(component);
    PrebuildMaterials(effect);
}

void ParticleEffectSystem::RemoveEntity(Entity * entity)
{
	ParticleEffectComponent * effect = static_cast<ParticleEffectComponent *>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	if (effect&&effect->state!=ParticleEffectComponent::STATE_STOPPED)
		RemoveFromActive(effect);

}
void ParticleEffectSystem::RemoveComponent(Entity * entity, Component * component)
{
	ParticleEffectComponent * effect = static_cast<ParticleEffectComponent *>(component);
	if (effect&&effect->state!=ParticleEffectComponent::STATE_STOPPED)
		RemoveFromActive(effect);
}

void ParticleEffectSystem::ImmediateEvent(Component * component, uint32 event)
{
    DVASSERT(component->GetType() == Component::PARTICLE_EFFECT_COMPONENT);
	ParticleEffectComponent *effect = static_cast<ParticleEffectComponent*>(component);
	if (event == EventSystem::START_PARTICLE_EFFECT)
    {
		if (effect->state == ParticleEffectComponent::STATE_STOPPED)
            AddToActive(effect);            
        effect->state = ParticleEffectComponent::STATE_STARTING;                    
        effect->currRepeatsCont = 0;
    }
	else if (event == EventSystem::STOP_PARTICLE_EFFECT)
		RemoveFromActive(effect);

}

void ParticleEffectSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("ParticleEffectSystem::Process");

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_PARTICLE_EMMITERS))
        return;
    /*shortEffectTime*/
    float32 currFps = 1.0f / timeElapsed;
    float32 currPSValue = (currFps - PerformanceSettings::Instance()->GetPsPerformanceMinFPS()) / (PerformanceSettings::Instance()->GetPsPerformanceMaxFPS() - PerformanceSettings::Instance()->GetPsPerformanceMinFPS());
    currPSValue = Clamp(currPSValue, 0.0f, 1.0f);
    float32 speedMult = 1.0f+(PerformanceSettings::Instance()->GetPsPerformanceSpeedMult()-1.0f)*(1-currPSValue);
	float32 shortEffectTime = timeElapsed*speedMult;
	
	size_t componentsCount = activeComponents.size();
	for(size_t i=0; i<componentsCount; i++)
	{
		ParticleEffectComponent * effect = activeComponents[i];
        if (effect->activeLodLevel!=effect->desiredLodLevel)
            UpdateActiveLod(effect);
        if (effect->state == ParticleEffectComponent::STATE_STARTING)  
        {            
            RunEffect(effect);
        }
        
		if (effect->isPaused) 
			continue;
		UpdateEffect(effect, timeElapsed*effect->playbackSpeed, shortEffectTime*effect->playbackSpeed);	

		bool effectEnded = effect->stopWhenEmpty?effect->effectData.groups.empty():(effect->time>effect->effectDuration);
		if (effectEnded)
		{
			effect->currRepeatsCont++;
			if (((effect->repeatsCount==0)||(effect->currRepeatsCont<effect->repeatsCount))&&(effect->state != ParticleEffectComponent::STATE_STOPPING)) //0 means infinite repeats
			{
				if (effect->clearOnRestart)
					effect->ClearCurrentGroups();
				RunEffect(effect);
			}
			else
			{
				effect->state = ParticleEffectComponent::STATE_STOPPING;
                effect->SetGroupsFinishing();
			}
		}
		/*finish restart criteria*/		
		if ((effect->state == ParticleEffectComponent::STATE_STOPPING)&&effect->effectData.groups.empty())
		{
			effect->effectData.infoSources.resize(1);
			RemoveFromActive(effect);
			componentsCount--;
			i--;
			effect->state = ParticleEffectComponent::STATE_STOPPED;
			if (!effect->playbackComplete.IsEmpty())
				effect->playbackComplete(effect->GetEntity(), 0);
		}
		else
		{
            Scene *scene = GetScene();
            if (scene)
			    scene->GetRenderSystem()->MarkForUpdate(effect->effectRenderObject);
		}
		
		
	}
}


void ParticleEffectSystem::UpdateActiveLod(ParticleEffectComponent *effect)
{
    DVASSERT(effect->activeLodLevel!=effect->desiredLodLevel);
    effect->activeLodLevel = effect->desiredLodLevel;
    for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;++it)
    {
        ParticleGroup& group = *it;
        if (!group.emitter->shortEffect)
            group.visibleLod = group.layer->IsLodActive(effect->activeLodLevel);
    }    

    if (allowLodDegrade && (effect->activeLodLevel == 0)) //degrade existing groups if needed
    {
        for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;++it)
        {
            ParticleGroup& group = *it;
            if (group.layer->degradeStrategy==ParticleLayer::DEGRADE_REMOVE)
            {
                Particle * current = group.head;
                while (current)
                {
                    Particle *next = current->next;
                    delete current;
                    current = next;
                }
                group.head = NULL;
            }
            else if (group.layer->degradeStrategy==ParticleLayer::DEGRADE_CUT_PARTICLES)
            {

                Particle * current = group.head;
                Particle * prev = NULL;
                int32 i=0;
                while (current)
                {
                    Particle *next = current->next;
                    if (i%2) //cut every second particle
                    {                            
                        delete current;                 
                        group.activeParticleCount--;
                        if (prev)
                            prev->next = next;
                        else
                            group.head = next;
                    }
                    prev = current;
                    current = next;
                    i++;                        
                }               
            }
        }
    }
}

void ParticleEffectSystem::UpdateEffect(ParticleEffectComponent *effect, float32 time, float32 shortEffectTime)
{
	effect->time+=time;
    const Matrix4 *worldTransformPtr;
    if (GetScene())
        worldTransformPtr = &effect->GetEntity()->GetWorldTransform();
    else
        worldTransformPtr = effect->effectRenderObject->GetWorldTransformPtr();

	effect->effectData.infoSources[0].position = worldTransformPtr->GetTranslationVector();
	
	AABBox3 bbox;
	for (List<ParticleGroup>::iterator it = effect->effectData.groups.begin(), e=effect->effectData.groups.end(); it!=e;)
	{
		ParticleGroup &group = *it;
		group.activeParticleCount = 0;
		float32 dt = group.emitter->shortEffect?shortEffectTime:time;
		group.time+=dt;		
		float32 groupEndTime = group.layer->isLooped?group.layer->loopEndTime:group.layer->endTime;
		float32 currLoopTime = group.time - group.loopStartTime;
		if (group.time>groupEndTime)
			group.finishingGroup = true;
		
		if ((!group.finishingGroup)&&(group.layer->isLooped)&&(currLoopTime>group.loopDuration))//restart loop
		{
			group.loopStartTime = group.time;
			group.loopLyaerStartTime = group.layer->deltaTime + group.layer->deltaVariation*(float32)Random::Instance()->RandFloat();
			group.loopDuration = group.loopLyaerStartTime+(group.layer->endTime-group.layer->startTime) + group.layer->loopVariation*(float32)Random::Instance()->RandFloat();				
			currLoopTime = 0;
		}			
		
		//prepare forces as they will now actually change in time even for already generated particles
		static Vector<Vector3> currForceValues;
		int32 forcesCount;
		if (group.head)
		{
			forcesCount = static_cast<int32>(group.layer->forces.size());
			if (forcesCount)
			{				
				currForceValues.resize(forcesCount);
				for (int32 i=0; i<forcesCount; ++i)
				{
					if (group.layer->forces[i]->force)
						currForceValues[i]=group.layer->forces[i]->force->GetValue(currLoopTime);
					else
						currForceValues[i]=Vector3(0,0,0);
				}
			}
		}		
		Particle *current = group.head;
		Particle *prev = 0;		
		while (current)
		{
			current->life += dt;
			if (current->life >= current->lifeTime)
			{
				Particle *next = current->next;
				if (prev == 0)
					group.head = next;
				else prev->next = next;				
				delete current;
				current = next;
				continue;
			}			
			group.activeParticleCount++;

			float32 overLifeTime = current->life/current->lifeTime;
			float32 currVelocityOverLife = 1.0f;
			if (group.layer->velocityOverLife)
				currVelocityOverLife = group.layer->velocityOverLife->GetValue(overLifeTime);
			current->position += current->speed * (currVelocityOverLife*dt);				
			float32 currSpinOverLife = 1;
			if (group.layer->spinOverLife)
				currSpinOverLife = group.layer->spinOverLife->GetValue(overLifeTime);
			current->angle += current->spin * currSpinOverLife *dt;	
			if (forcesCount)
			{
				Vector3 acceleration;
				for(int32 i = 0; i < forcesCount; ++i)
				{
					acceleration += (group.layer->forces[i]->forceOverLife)?(currForceValues[i] * group.layer->forces[i]->forceOverLife->GetValue(overLifeTime)) : currForceValues[i];				
				}
				current->speed+=acceleration*dt;
			}		
			
			if (group.layer->sizeOverLifeXY)
			{
				current->currSize=current->baseSize*group.layer->sizeOverLifeXY->GetValue(overLifeTime);
				Vector2 pivotSize = current->currSize*group.layer->layerPivotSizeOffsets;
				current->currRadius = pivotSize.Length();
			}
			if (group.layer->inheritPosition)
				AddParticleToBBox(current->position+effect->effectData.infoSources[group.positionSource].position, current->currRadius, bbox);
			else
				AddParticleToBBox(current->position, current->currRadius, bbox);
			
			if (group.layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
			{
				effect->effectData.infoSources[current->positionTarget].position = current->position;
				effect->effectData.infoSources[current->positionTarget].size = current->currSize;
			}
			
			if (group.layer->frameOverLifeEnabled&&group.layer->sprite)
			{
				float32 animDelta = group.layer->frameOverLifeFPS;
				if (group.layer->animSpeedOverLife)
					animDelta*=group.layer->animSpeedOverLife->GetValue(overLifeTime);
				current->animTime+=animDelta*dt;

				while (current->animTime>1.0f)
				{
					current->frame ++;										
					current->animTime -= 1.0f;
					if (current->frame>=group.layer->sprite->GetFrameCount())
					{
						if (group.layer->loopSpriteAnimation)
							current->frame = 0;
						else
							current->frame = group.layer->sprite->GetFrameCount()-1;
					}
				}
			}			
			prev = current;
			current=current->next;			
		}		
		bool allowParticleGeneration = !group.finishingGroup;
		allowParticleGeneration&=(currLoopTime>group.loopLyaerStartTime);	
		allowParticleGeneration&=group.visibleLod;
		if (allowParticleGeneration)
		{
			if (group.layer->type == ParticleLayer::TYPE_SINGLE_PARTICLE)
			{
				if (!group.head)
				{
					current = GenerateNewParticle(effect, group, currLoopTime, *worldTransformPtr);
					if (group.layer->inheritPosition)
						AddParticleToBBox(current->position+effect->effectData.infoSources[group.positionSource].position, current->currRadius, bbox);
					else
						AddParticleToBBox(current->position, current->currRadius, bbox);
				}
			}else
			{
				float32 newParticles = 0.0f;
				
				if (group.layer->number)
					newParticles = group.layer->number->GetValue(currLoopTime);
				if (group.layer->numberVariation)
					newParticles += group.layer->numberVariation->GetValue(currLoopTime)*(float32)Random::Instance()->RandFloat();
				newParticles*=dt;				
				group.particlesToGenerate += newParticles;

				while(group.particlesToGenerate >= 1.0f)
				{
					group.particlesToGenerate -= 1.0f;					
					current = GenerateNewParticle(effect, group, currLoopTime, *worldTransformPtr);
					if (group.layer->inheritPosition)
						AddParticleToBBox(current->position+effect->effectData.infoSources[group.positionSource].position, current->currRadius, bbox);
					else
						AddParticleToBBox(current->position, current->currRadius, bbox);
				}
			}
			
		}

		/*finally*/
		if (group.finishingGroup&&(!group.head))
		{
			group.emitter->Release();
			group.layer->Release();
			it = effect->effectData.groups.erase(it);
		}
		else
			++it;
	}
	if (bbox.IsEmpty())
	{
		Vector3 pos = worldTransformPtr->GetTranslationVector();
		bbox = AABBox3(pos, pos);
	}
	effect->effectRenderObject->SetAABBox(bbox);
}

void ParticleEffectSystem::AddParticleToBBox(const Vector3& position, float radius, AABBox3& bbox)
{	
	Vector3 sz = Vector3(radius,radius,radius);			
	bbox.AddPoint(position-sz);
	bbox.AddPoint(position+sz);
}


Particle* ParticleEffectSystem::GenerateNewParticle(ParticleEffectComponent *effect, ParticleGroup& group, float32 currLoopTime, const Matrix4 &worldTransform)
{
	Particle *particle = new Particle();		
	particle->life = 0.0f;	

	particle->color = Color();
	if (group.layer->colorRandom)
	{
		particle->color = group.layer->colorRandom->GetValue((float32)Random::Instance()->RandFloat());
	}
	if (group.emitter->colorOverLife)
	{
		particle->color*=group.emitter->colorOverLife->GetValue(group.time);
	}

	particle->lifeTime = 0.0f;
	if (group.layer->life)
		particle->lifeTime += group.layer->life->GetValue(currLoopTime);
	if (group.layer->lifeVariation)
		particle->lifeTime += (group.layer->lifeVariation->GetValue(currLoopTime) * (float32)Random::Instance()->RandFloat());

	// size 
	particle->baseSize = Vector2(1.0f, 1.0f); 
	if (group.layer->size)
		particle->baseSize = group.layer->size->GetValue(currLoopTime);
	if (group.layer->sizeVariation)
		particle->baseSize +=(group.layer->sizeVariation->GetValue(currLoopTime) * (float32)Random::Instance()->RandFloat());
	particle->baseSize*=effect->effectData.infoSources[group.positionSource].size;

	particle->currSize = particle->baseSize;
	if (group.layer->sizeOverLifeXY)
		particle->currSize*=group.layer->sizeOverLifeXY->GetValue(0);
	Vector2 pivotSize = particle->currSize*group.layer->layerPivotSizeOffsets;
	particle->currRadius = pivotSize.Length();
	

	particle->angle = 0.0f;
	particle->spin = 0.0f;
	if (group.layer->angle)
		particle->angle = DegToRad(group.layer->angle->GetValue(currLoopTime));
	if (group.layer->angleVariation)
		particle->angle += DegToRad(group.layer->angleVariation->GetValue(currLoopTime) * (float32)Random::Instance()->RandFloat());		
	if (group.layer->spin)
		particle->spin = DegToRad(group.layer->spin->GetValue(currLoopTime));
	if (group.layer->spinVariation)
		particle->spin += DegToRad(group.layer->spinVariation->GetValue(currLoopTime) * (float32)Random::Instance()->RandFloat());
	if (group.layer->randomSpinDirection)
	{
		int32 dir = Rand()&1;
		particle->spin *= (dir)*2-1;
	}
	particle->frame = 0;
	particle->animTime = 0;
	if (group.layer->randomFrameOnStart&&group.layer->sprite)
	{
		particle->frame =  (int32)((float32)Random::Instance()->RandFloat() * (float32)(group.layer->sprite->GetFrameCount()));
	}	
	
	
	
	PrepareEmitterParameters(particle, group, worldTransform);
	
	float32 vel = 0.0f;
	if (group.layer->velocity)
		vel += group.layer->velocity->GetValue(currLoopTime);
	if (group.layer->velocityVariation)
		vel += (group.layer->velocityVariation->GetValue(currLoopTime) * (float32)Random::Instance()->RandFloat());
	particle->speed*=vel;
		
	if (!group.layer->inheritPosition) //just generate at correct position
	{		
		particle->position += effect->effectData.infoSources[group.positionSource].position;
	}		
		
	particle->next = group.head;
	group.head = particle;	
	group.activeParticleCount++;
	if (group.layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
	{
		ParentInfo info;
		info.position = particle->position;
		info.size = particle->currSize; 
		effect->effectData.infoSources.push_back(info);
		particle->positionTarget = static_cast<int32>(effect->effectData.infoSources.size()-1);
		ParticleEmitter *innerEmitter = group.layer->innerEmitter;
		if (innerEmitter)
			RunEmitter(effect, innerEmitter, Vector3(0,0,0), particle->positionTarget);
	}
	return particle;
}

void ParticleEffectSystem::PrepareEmitterParameters(Particle * particle, ParticleGroup &group, const Matrix4 &worldTransform)
{		
	//calculate position new particle position in emitter space (for point leave it V3(0,0,0))
	if (group.emitter->emitterType == ParticleEmitter::EMITTER_RECT)
	{        
		if (group.emitter->size)
		{
			Vector3 currSize = group.emitter->size->GetValue(group.time);
			particle->position = Vector3(currSize.x*((float32)Random::Instance()->RandFloat() - 0.5f), currSize.y*((float32)Random::Instance()->RandFloat() - 0.5f), currSize.z*((float32)Random::Instance()->RandFloat() - 0.5f));
		}				
	}
	else if ((group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_VOLUME) || (group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_EDGES)||(group.emitter->emitterType == ParticleEmitter::EMITTER_SHOCKWAVE))
	{
		float32 curRadius = 1.0f;
		if (group.emitter->radius)		
			curRadius = group.emitter->radius->GetValue(group.time);		

        float32 angleBase = 0;
        float32 angleVariation = PI_2;
        if (group.emitter->emissionAngle)
            angleBase = DegToRad(group.emitter->emissionAngle->GetValue(group.time));
        if (group.emitter->emissionAngleVariation)
            angleVariation = DegToRad(group.emitter->emissionAngleVariation->GetValue(group.time));

		float32 curAngle = angleBase + angleVariation * (float32)Random::Instance()->RandFloat();
		if (group.emitter->emitterType == ParticleEmitter::EMITTER_ONCIRCLE_VOLUME)		
			curRadius *= (float32)Random::Instance()->RandFloat();		
		float sinAngle = 0.0f;
		float cosAngle = 0.0f;
		SinCosFast(curAngle, sinAngle, cosAngle);
		particle->position = Vector3(curRadius * cosAngle, curRadius * sinAngle, 0.0f);
	}
	
	//current emission vector and it's length
	Vector3 currEmissionVector(0,0,1);
	if (group.emitter->emissionVector)
		currEmissionVector = group.emitter->emissionVector->GetValue(group.time);
	float32 currEmissionPower = currEmissionVector.Length();
	//calculate speed in emitter space not transformed by emission vector yet
	if (group.emitter->emitterType == ParticleEmitter::EMITTER_SHOCKWAVE)
	{
		particle->speed = particle->position;
		float32 spl = particle->speed.SquareLength();
		if (spl>EPSILON)
		{
			particle->speed*=currEmissionPower/sqrtf(spl);
		}
	}
	else
	{
		if (group.emitter->emissionRange)
		{
			float32 theta = (float32)Random::Instance()->RandFloat()*DegToRad(group.emitter->emissionRange->GetValue(group.time))*0.5f;
			float32 phi = (float32)Random::Instance()->RandFloat() * PI_2;
			particle->speed = Vector3(currEmissionPower* cos(phi) * sin(theta), currEmissionPower * sin (phi) * sin(theta), currEmissionPower * cos(theta));			
		}
		else
		{
			particle->speed = Vector3(0, 0, currEmissionPower);
		}
	}

	//now transform position and speed by emissionVector and worldTransfrom rotations - preserving length	
	Matrix3 newTransform(worldTransform);
	if ((fabs(currEmissionVector.x)<EPSILON)&&(fabs(currEmissionVector.y)<EPSILON))
	{
		if (currEmissionVector.z<0)
		{
			Matrix3 rotation;
			rotation.CreateRotation(Vector3(1,0,0), PI);
			//newTransform = rotation*newTransform;
			particle->position=particle->position*rotation;
			particle->speed=particle->speed*rotation;			
		}
	}
	else
	{
		Vector3 axis(currEmissionVector.y, -currEmissionVector.x, 0);
		axis.Normalize();
		float32 angle = acosf(currEmissionVector.z/currEmissionPower);
		Matrix3 rotation;
		rotation.CreateRotation(axis, angle);
		//newTransform = rotation*newTransform;
		particle->position=particle->position*rotation;
		particle->speed=particle->speed*rotation;
	}
	particle->position += group.spawnPosition;
	TransformPerserveLength(particle->speed, newTransform);
	TransformPerserveLength(particle->position, newTransform); //note - from now emitter position is not effected by scale anymore (artist request)	
}



void ParticleEffectSystem::SetGlobalExtertnalValue(const String& name, float32 value)
{
	globalExternalValues[name] = value;
	for (Vector<ParticleEffectComponent *>::iterator it = activeComponents.begin(), e=activeComponents.end(); it!=e; ++it)
		(*it)->SetExtertnalValue(name, value);
}

float32 ParticleEffectSystem::GetGlobalExternalValue(const String& name)
{
	Map<String, float32>::iterator it = globalExternalValues.find(name);
	if (it!=globalExternalValues.end())
		return (*it).second;
	else
		return 0.0f;
}

Map<String, float32> ParticleEffectSystem::GetGlobalExternals()
{
	return globalExternalValues;
}

}