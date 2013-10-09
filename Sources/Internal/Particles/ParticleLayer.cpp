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


#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Utils/StringFormat.h"
#include "Render/RenderManager.h"
#include "Render/Image.h"
#include "Utils/Random.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Components/LodComponent.h"

namespace DAVA
{

const ParticleLayer::LayerTypeNamesInfo ParticleLayer::layerTypeNamesInfoMap[] =
{
	{ TYPE_SINGLE_PARTICLE, "single" },
	{ TYPE_PARTICLES, "particles" },
	{ TYPE_SUPEREMITTER_PARTICLES, "superEmitter" }
};

ParticleLayer::ParticleLayer()
	: head(0)
	, count(0)
	, limit(1000)
	, emitter(0)
	, sprite(0)
	, innerEmitter(NULL)
{
	renderBatch = new ParticleLayerBatch();
	renderBatch->SetParticleLayer(this);

	life = 0;
	lifeVariation = 0;

	number = 0;	
	numberVariation = 0;		

	size = 0;
	sizeVariation = 0;

	velocity = 0;
	velocityVariation = 0;	
	velocityOverLife = 0;

	spin = 0;			
	spinVariation = 0;
	spinOverLife = 0;

	motionRandom = 0;		
	motionRandomVariation = 0;
	motionRandomOverLife = 0;

	bounce = 0;				
	bounceVariation = 0;
	bounceOverLife = 0;
	
	colorOverLife = 0;
	colorRandom = 0;
	alphaOverLife = 0;

	particlesToGenerate = 0.0f;
	alignToMotion = 0.0f;
	
	angle = 0;
	angleVariation = 0;

	layerTime = 0.0f;
	loopLayerTime = 0.0f;
	additive = true;
	inheritPosition = true;
	type = TYPE_PARTICLES;
    
    endTime = 100000000.0f;
	deltaTime = 0.0f;
	deltaVariation = 0.0f;
	loopVariation = 0.0f;
	loopEndTime = 0.0f;
	currentLoopVariation = 0.0f;
	currentDeltaVariation = 0.0f;	

	frameOverLifeEnabled = false;
	frameOverLifeFPS = 0;
	randomFrameOnStart = false;

    isDisabled = false;
	isLooped = false;

	playbackSpeed = 1.0f;

	activeLODS.resize(LodComponent::MAX_LOD_LAYERS, true);
}

ParticleLayer::~ParticleLayer()
{
	DeleteAllParticles();

	SafeRelease(renderBatch);
	SafeRelease(sprite);
	SafeRelease(innerEmitter);

	head = 0;
	CleanupForces();
	// dynamic cache automatically delete all particles
}

ParticleLayer * ParticleLayer::Clone(ParticleLayer * dstLayer)
{
	if(!dstLayer)
	{
		dstLayer = new ParticleLayer();
	}

	if (life)
		dstLayer->life.Set(life->Clone());
	
	if (lifeVariation)
		dstLayer->lifeVariation.Set(lifeVariation->Clone());

	if (number)
		dstLayer->number.Set(number->Clone());
	
	if (numberVariation)
		dstLayer->numberVariation.Set(numberVariation->Clone());

	if (size)
		dstLayer->size.Set(size->Clone());
	
	if (sizeVariation)
		dstLayer->sizeVariation.Set(sizeVariation->Clone());
	
	if (sizeOverLifeXY)
		dstLayer->sizeOverLifeXY.Set(sizeOverLifeXY->Clone());
	
	if (velocity)
		dstLayer->velocity.Set(velocity->Clone());
	
	if (velocityVariation)
		dstLayer->velocityVariation.Set(velocityVariation->Clone());
	
	if (velocityOverLife)
		dstLayer->velocityOverLife.Set(velocityOverLife->Clone());

	// Copy the forces.
	for (int32 f = 0; f < (int32)forces.size(); ++ f)
	{
		ParticleForce* clonedForce = new ParticleForce(this->forces[f]);
		dstLayer->AddForce(clonedForce);
	}

	if (spin)
		dstLayer->spin.Set(spin->Clone());
	
	if (spinVariation)
		dstLayer->spinVariation.Set(spinVariation->Clone());
	
	if (spinOverLife)
		dstLayer->spinOverLife.Set(spinOverLife->Clone());
	
	if (motionRandom)
		dstLayer->motionRandom.Set(motionRandom->Clone());
	
	if (motionRandomVariation)
		dstLayer->motionRandomVariation.Set(motionRandomVariation->Clone());
	
	if (motionRandomOverLife)
		dstLayer->motionRandomOverLife.Set(motionRandomOverLife->Clone());
	
	if (bounce)
		dstLayer->bounce.Set(bounce->Clone());
	
	if (bounceVariation)
		dstLayer->bounceVariation.Set(bounceVariation->Clone());
	
	if (bounceOverLife)
		dstLayer->bounceOverLife.Set(bounceOverLife->Clone());
	
	if (colorOverLife)
		dstLayer->colorOverLife.Set(colorOverLife->Clone());
	
	if (colorRandom)
		dstLayer->colorRandom.Set(colorRandom->Clone());
	
	if (alphaOverLife)
		dstLayer->alphaOverLife.Set(alphaOverLife->Clone());
	
	if (angle)
		dstLayer->angle.Set(angle->Clone());
	
	if (angleVariation)
		dstLayer->angleVariation.Set(angleVariation->Clone());

	if (innerEmitter)
		dstLayer->innerEmitter = static_cast<ParticleEmitter*>(innerEmitter->Clone(NULL));

	dstLayer->layerName = layerName;
	dstLayer->alignToMotion = alignToMotion;
	dstLayer->SetAdditive(additive);
	dstLayer->SetInheritPosition(inheritPosition);
	dstLayer->startTime = startTime;
	dstLayer->endTime = endTime;
	
	
	dstLayer->isLooped = isLooped;
	dstLayer->deltaTime = deltaTime;
	dstLayer->deltaVariation = deltaVariation;
	dstLayer->loopVariation = loopVariation;
	dstLayer->loopEndTime = loopEndTime;
	
	dstLayer->type = type;
	dstLayer->sprite = SafeRetain(sprite);
	dstLayer->layerPivotPoint = layerPivotPoint;	

	dstLayer->frameOverLifeEnabled = frameOverLifeEnabled;
	dstLayer->frameOverLifeFPS = frameOverLifeFPS;
	dstLayer->randomFrameOnStart = randomFrameOnStart;

    dstLayer->isDisabled = isDisabled;
	dstLayer->spritePath = spritePath;
	dstLayer->activeLODS = activeLODS;

	return dstLayer;
}

bool ParticleLayer::IsLodActive(int32 lod)
{
	if ((lod>=0)&&(lod<activeLODS.size()))
		return activeLODS[lod];
	
	return false;
}

void ParticleLayer::SetLodActive(int32 lod, bool active)
{
	if ((lod>=0)&&(lod<activeLODS.size()))
		activeLODS[lod] = active;
}

template <class T> void UpdatePropertyLineKeys(PropertyLine<T> * line, float32 startTime, float32 translateTime, float32 endTime)
{	
	if (!line)	
		return;	
	Vector<typename PropertyLine<T>::PropertyKey> &keys = line->GetValues();
	int32 size = keys.size();		
	int32 i;
	for (i=0; i<size; ++i)
	{
		keys[i].t += translateTime;
		if (keys[i].t>endTime)		
			break;		
		
	}
	if (i==0)
		i+=1; //keep at least 1
	keys.erase(keys.begin()+i, keys.end());
	if (keys.size() == 1)
	{
		keys[0].t = startTime;
	}
}

template <class T> void UpdatePropertyLineOnLoad(PropertyLine<T> * line, float32 startTime, float32 endTime)
{
	if (!line)
		return;
	Vector<typename PropertyLine<T>::PropertyKey> &keys = line->GetValues();
	int32 size = keys.size();		
	int32 i;
	/*drop keys before*/
	for (i=0; i<size; ++i)
	{
		if (keys[i].t>=startTime)
			break;
	}
	if (i!=0)
	{
		T v0 = line->GetValue(startTime);
		keys.erase(keys.begin(), keys.begin()+i);
		typename PropertyLine<T>::PropertyKey key;
		key.t = startTime;
		key.value = v0;
		keys.insert(keys.begin(), key);
	}	
	
	/*drop keys after*/
	size = keys.size();
	for (i=0; i<size; i++)
	{
		if (keys[i].t>endTime)
			break;
	}
	if (i!=size)
	{
		T v1 = line->GetValue(endTime);
		keys.erase(keys.begin()+i, keys.end());
		typename PropertyLine<T>::PropertyKey key;
		key.t = endTime;
		key.value = v1;
		keys.push_back(key);
	}
}

void ParticleLayer::UpdateLayerTime(float32 startTime, float32 endTime)
{
	float32 translateTime = startTime-this->startTime;
	this->startTime = startTime;
	this->endTime = endTime;
	/*validate all time depended property lines*/	
	UpdatePropertyLineKeys(life.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(lifeVariation.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(number.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(numberVariation.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(size.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(sizeVariation.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(velocity.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(velocityVariation.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(spin.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(spinVariation.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(angle.Get(), startTime, translateTime, endTime);
	UpdatePropertyLineKeys(angleVariation.Get(), startTime, translateTime, endTime);
}

ParticleEmitter* ParticleLayer::GetEmitter() const
{
	return emitter;
}

void ParticleLayer::SetEmitter(ParticleEmitter * _emitter)
{
	emitter = _emitter;
}

void ParticleLayer::SetSprite(Sprite * _sprite)
{
    DeleteAllParticles();
	SafeRelease(sprite);
	sprite = SafeRetain(_sprite);
	UpdateFrameTimeline();

	if(sprite)
	{
		spritePath = sprite->GetRelativePathname();
	}
}
	
Sprite * ParticleLayer::GetSprite()
{
	return sprite;
}

float32 ParticleLayer::GetLayerTime()
{
    return layerTime;
}
    
void ParticleLayer::DeleteAllParticles()
{
	if(TYPE_SINGLE_PARTICLE == type)
	{

	}
	Particle * current = head;
	while(current)
	{
		Particle * next = current->next;
		delete(current);
		count--;
		current = next;
	}
	head = 0;
	
	DVASSERT(count == 0);
}

void ParticleLayer::RunParticle(Particle * particle)
{
	particle->next = head;
	head = particle;
	count++;
}


/*void ParticleLayer::SetLimit(int32 _limit)
{
	limit = _limit;
}

void ParticleLayer::SetPosition(Vector3 _position)
{
	position = _position;
}*/

void ParticleLayer::Restart(bool isDeleteAllParticles)
{
	if (isDeleteAllParticles)
		DeleteAllParticles();

	layerTime = 0.0f;
	particlesToGenerate = 0.0f;
	loopLayerTime = 0.0f;

	RecalculateVariation();
	
	if (innerEmitter)
	{
		innerEmitter->Restart(isDeleteAllParticles);
	}
}

void ParticleLayer::RestartLoop(bool isDeleteAllParticles)
{
	if (isDeleteAllParticles)
		DeleteAllParticles();

	layerTime = startTime;
	particlesToGenerate = 0.0f;
	
	RecalculateVariation();
	
	if (innerEmitter)
	{
		innerEmitter->Restart(isDeleteAllParticles);
	}
}

void ParticleLayer::SetLooped(bool _isLooped)
{
	isLooped = _isLooped;
	// Reset loop values if isLooped flag is changed
	loopVariation = 0.0f;
	deltaTime = 0.0f;
	deltaVariation = 0.0f;
	/*if (isLooped)
	{
		if (emitter)
		{
			loopEndTime = emitter->GetLifeTime();
		}
	}*/
}

bool ParticleLayer::GetLooped()
{
	return isLooped;
}

void ParticleLayer::SetDeltaTime(float32 _deltaTime)
{
	deltaTime = _deltaTime;
}
	
float32 ParticleLayer::GetDeltaTime()
{
	return deltaTime;
}

void ParticleLayer::SetDeltaVariation(float32 _deltaVariation)
{
	deltaVariation = _deltaVariation;
	RecalculateVariation();
}
	
float32 ParticleLayer::GetDeltaVariation()
{
	return deltaVariation;
}

void ParticleLayer::SetLoopEndTime(float32 endTime)
{
	loopEndTime = endTime;
}

float32 ParticleLayer::GetLoopEndTime()
{
	return loopEndTime;
}

void ParticleLayer::SetLoopVariation(float32 _loopVariation)
{
	loopVariation = _loopVariation;
	RecalculateVariation();
}

float32 ParticleLayer::GetLoopVariation()
{
	return loopVariation;
}

float32 ParticleLayer::GetRandomFactor()
{
	return (float32)(Rand() & 255) / 255.0f;
}

void ParticleLayer::RecalculateVariation()
{
	currentLoopVariation = (loopVariation * GetRandomFactor());
	
	if (deltaTime > 0)
	{
		currentDeltaVariation = (deltaVariation * GetRandomFactor());
	}	
}

void ParticleLayer::RestartLayerIfNeed()
{
	float32 layerRestartTime = (endTime + currentLoopVariation) + (deltaTime + currentDeltaVariation);
	// Restart layer effect if auto restart option is on and layer time exceeds its endtime
	// Endtime can be increased by DeltaTime
	if(isLooped && (layerTime > layerRestartTime) && !emitter->IsPaused())
	{
		RestartLoop(false);
	}
}

void ParticleLayer::Update(float32 timeElapsed, bool generateNewParticles)
{
	// increment timer, take the playbackSpeed into account.
	timeElapsed *= playbackSpeed;
	layerTime += timeElapsed;
	loopLayerTime += timeElapsed;
	
	RestartLayerIfNeed();
	// Don't emit particles when lood end is reached
	bool useLoopStop = false;
	if (isLooped)
	{
		useLoopStop = (loopLayerTime >= loopEndTime);
	}
	else
	{
		currentLoopVariation = 0.0f;
	}

	switch(type)
	{
		case TYPE_PARTICLES:
		case TYPE_SUPEREMITTER_PARTICLES:
		{
			Particle * prev = 0;
			Particle * current = head;
			while(current)
			{
				Particle * next = current->next;
				if (!current->Update(timeElapsed))
				{
					if (prev == 0)head = next;
					else prev->next = next;
					delete(current);
					count--;
				}else
				{
					ProcessParticle(current);
					prev = current;
				}

				current = next;
			}
			
			if (count == 0)
			{
				DVASSERT(head == 0);
			}
			
			if ((layerTime >= startTime) && (layerTime < (endTime + currentLoopVariation)) &&
				!emitter->IsPaused() && !useLoopStop)
			{
				float32 newParticles = 0.0f;
				if (generateNewParticles)
				{
					if (number)
						newParticles += timeElapsed * number->GetValue(layerTime);
					if (numberVariation)
						newParticles += GetRandomFactor() * timeElapsed * numberVariation->GetValue(layerTime);
					//newParticles *= emitter->GetCurrentNumberScale();
				}				
				
				particlesToGenerate += newParticles;

				while(particlesToGenerate >= 1.0f)
				{
					particlesToGenerate -= 1.0f;
					
					int32 emitPointsCount = emitter->GetEmitPointsCount();
					
					if (emitPointsCount == -1)
						GenerateNewParticle(-1);
					else {
						for (int k = 0; k < emitPointsCount; ++k)
							 GenerateNewParticle(k);
					}

				}
			}
			break;
		}

		case TYPE_SINGLE_PARTICLE:
		{
			bool needUpdate = true;
			if ((layerTime >= startTime) && (layerTime < (endTime + currentLoopVariation)) &&
				!emitter->IsPaused() && !useLoopStop)
			{
				if(!head)
				{
					if (generateNewParticles)
					{
						GenerateSingleParticle();
						needUpdate = false;
					}
					
				}
			}
            if(head && needUpdate)
            {
				if (!head->Update(timeElapsed))
				{
					delete(head);
					count--;
					DVASSERT(0 == count);
					head = 0;
					if ((layerTime >= startTime) && (layerTime < (endTime + currentLoopVariation)) &&
						!emitter->IsPaused() && !useLoopStop)
					{
						if (generateNewParticles)
							GenerateSingleParticle();
					}
				}
				else
				{
					ProcessParticle(head);
				}
            }
			
			break;		
		}
	}
}
	
void ParticleLayer::GenerateSingleParticle()
{
	GenerateNewParticle(-1);
	
	// Yuri Coder, 2013/03/26. head->angle = 0.0f commented out because of DF-877.
	//head->angle = 0.0f;

	//particle->velocity.x = 0.0f;
	//particle->velocity.y = 0.0f;
}

void ParticleLayer::GenerateNewParticle(int32 emitIndex)
{
	if (count >= limit)
	{
		return;
	}
	
	Particle * particle = new Particle();

	// SuperEmitter particles contains the emitter inside.
	if (type == TYPE_SUPEREMITTER_PARTICLES)
	{
		innerEmitter->SetLongToAllLayers(IsLong());
		particle->InitializeInnerEmitter(this->emitter, innerEmitter);
	}

	particle->CleanupForces();

	particle->next = 0;
	particle->sprite = sprite;
	particle->life = 0.0f;

	float32 randCoeff = GetRandomFactor();	
	
	particle->color = Color();
	if (colorRandom)
	{
		particle->color = colorRandom->GetValue(randCoeff);
	}

	particle->lifeTime = 0.0f;
	if (life)
		particle->lifeTime += life->GetValue(layerTime);
	if (lifeVariation)
		particle->lifeTime += (lifeVariation->GetValue(layerTime) * randCoeff);
	
	// size 
	particle->size = Vector2(0.0f, 0.0f); 
	if (size)
		particle->size = size->GetValue(layerTime);
	if (sizeVariation)
		particle->size +=(sizeVariation->GetValue(layerTime) * randCoeff);
	
	if(sprite)
	{
		particle->size.x /= (float32)sprite->GetWidth();
		particle->size.y /= (float32)sprite->GetHeight();
	}
	else
	{
		particle->size = Vector2(0, 0);
	}

	float32 vel = 0.0f;
	if (velocity)
		vel += velocity->GetValue(layerTime);
	if (velocityVariation)
		vel += (velocityVariation->GetValue(layerTime) * randCoeff);
	
	particle->angle = 0.0f;
	particle->spin = 0.0f;
	if (spin)
		particle->spin = DegToRad(spin->GetValue(layerTime));
	if (spinVariation)
		particle->spin += DegToRad(spinVariation->GetValue(layerTime) * randCoeff);

	//particle->position = emitter->GetPosition();	
	// parameters should be prepared inside prepare emitter parameters
	emitter->PrepareEmitterParameters(particle, vel, emitIndex);
	if (this->emitter&&!inheritPosition) //just generate at correct position
	{		
		particle->position += emitter->GetPosition()-emitter->GetInitialTranslationVector();
	}

	//particle->angle += alignToMotion;
	if (angle)
		particle->angle += DegToRad(angle->GetValue(layerTime));
	if (angleVariation)
		particle->angle += DegToRad(angleVariation->GetValue(layerTime) * randCoeff);

	particle->sizeOverLife.x = 1.0f;
	particle->sizeOverLife.y = 1.0f;
	particle->velocityOverLife = 1.0f;
	particle->spinOverLife = 1.0f;
//	particle->forceOverLife0 = 1.0f;
//		
//	particle->force0.x = 0.0f;
//	particle->force0.y = 0.0f;
//
//	if ((forces.size() == 1) && (forces[0]))
//	{
//		particle->force0 = forces[0]->GetValue(layerTime);
//	}
//	if ((forcesVariation.size() == 1) && (forcesVariation[0]))
//	{
//		particle->force0 += forcesVariation[0]->GetValue(layerTime) * randCoeff;
//	}

	// Add the forces.
    int32 forcesCount = (int32)forces.size();
    for(int i = 0; i < forcesCount; i++)
	{
		Vector3 toAddForceDirection;
		float32 toAddForceValue = 0;
		float32 toAddForceOverlife = 0;
		bool toAddForceOvelifeEnabled = false;

		RefPtr<PropertyLine<Vector3> > currentForce = forces[i]->GetForce();
        if(currentForce && currentForce.Get())
		{
			const Vector3 & force = currentForce->GetValue(layerTime);
			float32 forceValue = force.Length();
			Vector3 forceDirection;
			if(forceValue)
			{
				forceDirection = force/forceValue;
			}

			toAddForceDirection = forceDirection;
			toAddForceValue = forceValue;
		}
		
		// Check the forces variations.
		RefPtr<PropertyLine<Vector3> > currentForceVariation = forces[i]->GetForceVariation();
        if(currentForceVariation && currentForceVariation.Get())
		{
			const Vector3 & force = currentForceVariation->GetValue(layerTime) * randCoeff;
			float32 forceValue = force.Length();
			Vector3 forceDirection = force/forceValue;

			toAddForceDirection += forceDirection;
			toAddForceValue += forceValue;
		}
		
		// Now check the overlife flag.
		RefPtr<PropertyLine<float32> > currentForceOverlife = forces[i]->GetForceOverlife();
		if(currentForceOverlife && currentForceOverlife.Get())
		{
			toAddForceOverlife = currentForceOverlife->GetValue(layerTime);
			toAddForceOvelifeEnabled = true;
		}
		
		// All the data is calculated - can add the force data to particle.
		particle->AddForce(toAddForceValue, toAddForceDirection, toAddForceOverlife, toAddForceOvelifeEnabled);
	}
    
	particle->frame = 0;
	if (randomFrameOnStart&&sprite)
	{
		particle->frame =  (int32)(randCoeff * (float32)(this->sprite->GetFrameCount()));
	}	
	
	// process it to fill first life values
	ProcessParticle(particle);

	// go to life
	RunParticle(particle);
}

void ParticleLayer::ProcessParticle(Particle * particle)
{
	float32 t = particle->life / particle->lifeTime;
	if (sizeOverLifeXY)
		particle->sizeOverLife = sizeOverLifeXY->GetValue(t);
	if (spinOverLife)
		particle->spinOverLife = spinOverLife->GetValue(t);
	if (velocityOverLife)
		particle->velocityOverLife = velocityOverLife->GetValue(t);
	if (colorOverLife)
		particle->color = colorOverLife->GetValue(t);
	if (alphaOverLife)
		particle->color.a = alphaOverLife->GetValue(t);

	Color emitterColor;
	if(emitter->GetCurrentColor(&emitterColor))
	{
		particle->drawColor = particle->color*emitterColor*emitter->ambientColor;
	}
	else
	{
		particle->drawColor = particle->color*emitter->ambientColor;
	}

	// Frame Overlife FPS defines how many frames should be displayed in a second.
	// This property is cycled - if we reached the last frame, we'll update to the new one.
	if (frameOverLifeEnabled && frameOverLifeFPS > 0)
	{
		float32 timeElapsed = particle->life - particle->frameLastUpdateTime;
		if (timeElapsed > (1 / frameOverLifeFPS))
		{
			particle->frame ++;
			// Spright might not be loaded (see please DF-1661).
			if (!this->sprite || particle->frame >= this->sprite->GetFrameCount())
			{
				particle->frame = 0;
			}

			particle->frameLastUpdateTime = particle->life;
		}
	}
    
	int32 forcesCount = (int32)forces.size();
    for(int i = 0; i < forcesCount; i++)
	{
        if (forces[i]->GetForceOverlife() && forces[i]->GetForceOverlife().Get())
		{
			particle->UpdateForceOverlife(i, forces[i]->GetForceOverlife()->GetValue(t));
		}
	}
}

void ParticleLayer::PrepareRenderData(Camera * camera)
{

}

void ParticleLayer::Draw(Camera * camera)
{
	if (additive)
	{
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ONE);
	}
	else 
	{
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	const Vector2& drawPivotPoint = GetDrawPivotPoint();
	switch(type)
	{
		case TYPE_PARTICLES:
		case TYPE_SUPEREMITTER_PARTICLES:
		{
			Particle * current = head;
			while(current)
			{
				sprite->SetPivotPoint(drawPivotPoint);
				current->Draw();
				current = current->next;
			}
			break;
		}
		case TYPE_SINGLE_PARTICLE:
		{
			if(head)
			{	
				sprite->SetPivotPoint(drawPivotPoint);
				head->Draw();
			}
			break;
		}
	}
}


void ParticleLayer::LoadFromYaml(const FilePath & configPath, const YamlNode * node)
{
// 	PropertyLine<float32> * life;				// in seconds
// 	PropertyLine<float32> * lifeVariation;		// variation part of life that added to particle life during generation of the particle
// 
// 	PropertyLine<float32> * number;				// number of particles per second
// 	PropertyLine<float32> * numberVariation;		// variation part of number that added to particle count during generation of the particle
// 
// 	PropertyLine<Vector2> * size;				// size of particles in pixels 
// 	PropertyLine<Vector2> * sizeVariation;		// size variation in pixels
// 	PropertyLine<float32> * sizeOverLife;
// 
// 	PropertyLine<float32> * velocity;			// velocity in pixels
// 	PropertyLine<float32> * velocityVariation;	
// 	PropertyLine<float32> * velocityOverLife;
// 
// 	PropertyLine<Vector2> * weight;				// weight property from 
// 	PropertyLine<Vector2> * weightVariation;
// 	PropertyLine<float32> * weightOverLife;
// 
// 	PropertyLine<float32> * spin;				// spin of angle / second
// 	PropertyLine<float32> * spinVariation;
// 	PropertyLine<float32> * spinOverLife;
// 
// 	PropertyLine<float32> * motionRandom;		//
// 	PropertyLine<float32> * motionRandomVariation;
// 	PropertyLine<float32> * motionRandomOverLife;
// 
// 	PropertyLine<float32> * bounce;				//
// 	PropertyLine<float32> * bounceVariation;
// 	PropertyLine<float32> * bounceOverLife;	
	
	
	type = TYPE_PARTICLES;
	const YamlNode * typeNode = node->Get("layerType");
	if (typeNode)
	{
		type = StringToLayerType(typeNode->AsString(), TYPE_PARTICLES);
	}

	const YamlNode * nameNode = node->Get("name");
	if (nameNode)
	{
		layerName = nameNode->AsString();
	}

	const YamlNode * pivotPointNode = node->Get("pivotPoint");
	if(pivotPointNode)
	{
		layerPivotPoint = pivotPointNode->AsPoint();
	}

	const YamlNode * spriteNode = node->Get("sprite");
	if (spriteNode && !spriteNode->AsString().empty())
	{
		// Store the absolute path to sprite.
		spritePath = FilePath(configPath.GetDirectory(), spriteNode->AsString());

		Sprite * _sprite = Sprite::Create(spritePath);
		SetSprite(_sprite);
        SafeRelease(_sprite);
	}
	const YamlNode *lodsNode = node->Get("activeLODS");
	if (lodsNode)
	{
		const Vector<YamlNode*> & vec = lodsNode->AsVector();
		for (uint32 i=0; i<(uint32)vec.size(); ++i)
			SetLodActive(i, (bool)vec[i]->AsInt()); //as AddToArray has no override for bool, flags are stored as int
	}


	colorOverLife = PropertyLineYamlReader::CreateColorPropertyLineFromYamlNode(node, "colorOverLife");
	colorRandom = PropertyLineYamlReader::CreateColorPropertyLineFromYamlNode(node, "colorRandom");
	alphaOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "alphaOverLife");
	
	const YamlNode* frameOverLifeEnabledNode = node->Get("frameOverLifeEnabled");
	if (frameOverLifeEnabledNode)
	{
		frameOverLifeEnabled = frameOverLifeEnabledNode->AsBool();
	}

	const YamlNode* randomFrameOnStartNode = node->Get("randomFrameOnStart");
	if (randomFrameOnStartNode)
	{
		randomFrameOnStart = randomFrameOnStartNode->AsBool();
	}

	const YamlNode* frameOverLifeFPSNode = node->Get("frameOverLifeFPS");
	if (frameOverLifeFPSNode)
	{
		frameOverLifeFPS = frameOverLifeFPSNode->AsFloat();
	}

	life = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "life");	
	lifeVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "lifeVariation");	

	number = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "number");	
	numberVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "numberVariation");	

	size = PropertyLineYamlReader::CreateVector2PropertyLineFromYamlNode(node, "size");
	sizeVariation = PropertyLineYamlReader::CreateVector2PropertyLineFromYamlNode(node, "sizeVariation");

	sizeOverLifeXY = PropertyLineYamlReader::CreateVector2PropertyLineFromYamlNode(node, "sizeOverLifeXY");

	// Yuri Coder, 2013/04/03. sizeOverLife is outdated and kept here for the backward compatibility only.
	// New property is sizeOverlifeXY and contains both X and Y components.
	RefPtr< PropertyLine<float32> > sizeOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "sizeOverLife");
	if (sizeOverLife)
	{
		if (sizeOverLifeXY)
		{
			// Both properties can't be present in the same config.
			Logger::Error("Both sizeOverlife and sizeOverlifeXY are defined for Particle Layer %s, taking sizeOverlifeXY as default",
						  configPath.GetAbsolutePathname().c_str());
			DVASSERT(false);
		}
		else
		{
			// Only the outdated sizeOverlife is defined - create sizeOverlifeXY property based on outdated one.
			FillSizeOverlifeXY(sizeOverLife);
		}
	}

	velocity = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocity");
	velocityVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocityVariation");	
	velocityOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocityOverLife");
	
	angle = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "angle");
	angleVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "angleVariation");
	
	int32 forceCount = 0;
	const YamlNode * forceCountNode = node->Get("forceCount");
	if (forceCountNode)
		forceCount = forceCountNode->AsInt();

	for (int k = 0; k < forceCount; ++k)
	{
        // Any of the Force Parameters might be NULL, and this is acceptable.
		RefPtr< PropertyLine<Vector3> > force = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(node, Format("force%d", k) );
		RefPtr< PropertyLine<Vector3> > forceVariation = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(node, Format("forceVariation%d", k));
		RefPtr< PropertyLine<float32> > forceOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, Format("forceOverLife%d", k));

		ParticleForce* particleForce = new ParticleForce(force, forceVariation, forceOverLife);
		AddForce(particleForce);
	}

	spin = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spin");
	spinVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spinVariation");	
	spinOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spinOverLife");	

	motionRandom = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandom");	
	motionRandomVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandomVariation");	
	motionRandomOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandomOverLife");	


	const YamlNode * blend = node->Get("blend");
	if (blend)
	{
		if (blend->AsString() == "alpha")
			additive = false;
		if (blend->AsString() == "add")
			additive = true;
	}

	const YamlNode * alignToMotionNode = node->Get("alignToMotion");
	if (alignToMotionNode)
		alignToMotion = DegToRad(alignToMotionNode->AsFloat());

	startTime = 0.0f;
	endTime = 100000000.0f;
	const YamlNode * startTimeNode = node->Get("startTime");
	if (startTimeNode)
		startTime = startTimeNode->AsFloat();

	const YamlNode * endTimeNode = node->Get("endTime");
	if (endTimeNode)
		endTime = endTimeNode->AsFloat();
		
	isLooped = false;	
	deltaTime = 0.0f;
	deltaVariation = 0.0f;
	loopVariation = 0.0f;
	
	const YamlNode * isLoopedNode = node->Get("isLooped");
	if (isLoopedNode)
		isLooped = isLoopedNode->AsBool();
		
	const YamlNode * deltaTimeNode = node->Get("deltaTime");
	if (deltaTimeNode)
		deltaTime = deltaTimeNode->AsFloat();
		
	const YamlNode * deltaVariationNode = node->Get("deltaVariation");
	if (deltaVariationNode)
		deltaVariation = deltaVariationNode->AsFloat();
		
	const YamlNode * loopVariationNode = node->Get("loopVariation");
	if (loopVariationNode)
		loopVariation = loopVariationNode->AsFloat();
		
	const YamlNode * loopEndTimeNode = node->Get("loopEndTime");
	if (loopEndTimeNode)
		loopEndTime = loopEndTimeNode->AsFloat();			

	const YamlNode * isDisabledNode = node->Get("isDisabled");
	if (isDisabledNode)
	{
		isDisabled = isDisabledNode->AsBool();
	}

	/*validate all time depended property lines*/	
	UpdatePropertyLineOnLoad(life.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(lifeVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(number.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(numberVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(size.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(sizeVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(velocity.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(velocityVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(spin.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(spinVariation.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(angle.Get(), startTime, endTime);
	UpdatePropertyLineOnLoad(angleVariation.Get(), startTime, endTime);

	const YamlNode * inheritPositionNode = node->Get("inheritPosition");
	if (inheritPositionNode)
	{
		inheritPosition = inheritPositionNode->AsBool();
	}

	// Load the Inner Emitter parameters.
	const YamlNode * innerEmitterPathNode = node->Get("innerEmitterPath");
	if (innerEmitterPathNode)
	{
		CreateInnerEmitter();

		// Since Inner Emitter path is stored as Relative, convert it to absolute when loading.
		innerEmitterPath = FilePath(configPath.GetDirectory(), innerEmitterPathNode->AsString());
		innerEmitter->LoadFromYaml(this->innerEmitterPath);
	}

	// Yuri Coder, 2013/01/31. After all the data is loaded, check the Frame Overlife timelines and
	// synchronize them with the maximum available frames in the sprite. See also DF-573.
	UpdateFrameTimeline();
}

void ParticleLayer::SaveToYamlNode(YamlNode* parentNode, int32 layerIndex)
{
    YamlNode* layerNode = new YamlNode(YamlNode::TYPE_MAP);
    String layerNodeName = Format("layer%d", layerIndex);
    parentNode->AddNodeToMap(layerNodeName, layerNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "name", layerName);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "type", "layer");
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "layerType",
																 LayerTypeToString(type, "particles"));
    if (this->IsLong())
    {
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLong", true);
    }

	PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector2>(layerNode, "pivotPoint", layerPivotPoint);

    // Truncate an extension of the sprite file.
	String relativePath = spritePath.GetRelativePathname(emitter->GetConfigPath().GetDirectory());
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "sprite",
        relativePath.substr(0, relativePath.size()-4));

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "life", this->life);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "lifeVariation", this->lifeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "number", this->number);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "numberVariation", this->numberVariation);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeVariation", this->sizeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeOverLifeXY", this->sizeOverLifeXY);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocity", this->velocity);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityVariation", this->velocityVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityOverLife", this->velocityOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spin", this->spin);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinVariation", this->spinVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinOverLife", this->spinOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandom", this->motionRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandomVariation", this->motionRandomVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandomOverLife", this->motionRandomOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounce", this->bounce);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounceVariation", this->bounceVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounceOverLife", this->bounceOverLife);
	
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angle", this->angle);
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angleVariation", this->angleVariation);

    PropertyLineYamlWriter::WriteColorPropertyLineToYamlNode(layerNode, "colorRandom", this->colorRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "alphaOverLife", this->alphaOverLife);

    PropertyLineYamlWriter::WriteColorPropertyLineToYamlNode(layerNode, "colorOverLife", this->colorOverLife);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "frameOverLifeEnabled", this->frameOverLifeEnabled);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "frameOverLifeFPS", this->frameOverLifeFPS);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "randomFrameOnStart", this->randomFrameOnStart);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "alignToMotion", this->alignToMotion);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "blend", this->additive ? "add" : "alpha");

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "startTime", this->startTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "endTime", this->endTime);
	
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLooped", this->isLooped);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaTime", this->deltaTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaVariation", this->deltaVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopVariation", this->loopVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopEndTime", this->loopEndTime);

	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isDisabled", this->isDisabled);

	layerNode->Set("inheritPosition", inheritPosition);	

	YamlNode *lodsNode = new YamlNode(YamlNode::TYPE_ARRAY);
	for (int32 i =0; i<LodComponent::MAX_LOD_LAYERS; i++)
		lodsNode->AddValueToArray((int32)activeLODS[i]); //as for now AddValueToArray has no bool type - force it to int
	layerNode->SetNodeToMap("activeLODS", lodsNode);

	if (innerEmitter)
	{
		String innerRelativePath = innerEmitterPath.GetRelativePathname(emitter->GetConfigPath().GetDirectory());
		PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "innerEmitterPath", innerRelativePath);
	}

    // Now write the forces.
    SaveForcesToYamlNode(layerNode);
}

void ParticleLayer::SaveForcesToYamlNode(YamlNode* layerNode)
{
    int32 forceCount = (int32)this->forces.size();
    if (forceCount == 0)
    {
        // No forces to write.
        return;
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "forceCount", forceCount);
    for (int32 i = 0; i < forceCount; i ++)
    {
		ParticleForce* currentForce = this->forces[i];
        String forceDataName = Format("force%d", i);
		
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->GetForce());

		// Force Variation and Force Overlife might be empty, so will not be stored in Yaml. This is handled
		// by PropertyLineYamlWriter, no further changes should be done.
        forceDataName = Format("forceVariation%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->GetForceVariation());

        forceDataName = Format("forceOverLife%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, forceDataName, currentForce->GetForceOverlife());
    }
}

Particle * ParticleLayer::GetHeadParticle()
{
	return head;
}

RenderBatch * ParticleLayer::GetRenderBatch()
{
	return renderBatch;
}

void ParticleLayer::UpdateFrameTimeline()
{
	if (!this->frameOverLifeEnabled)
	{
		return;
	}

	if (!this->sprite)
	{
		this->frameOverLifeEnabled = false;
		return;
	}
}
	
void ParticleLayer::SetAdditive(bool additive)
{
	this->additive = additive;
}

void ParticleLayer::SetInheritPosition(bool inherit)
{
	inheritPosition = inherit;
}

void ParticleLayer::AddForce(ParticleForce* force)
{
	SafeRetain(force);
	this->forces.push_back(force);
}

void ParticleLayer::UpdateForce(int32 forceIndex, RefPtr< PropertyLine<Vector3> > force,
										RefPtr< PropertyLine<Vector3> > forceVariation,
										RefPtr< PropertyLine<float32> > forceOverLife)
{
	if (forceIndex <= (int32)this->forces.size())
	{
		this->forces[forceIndex]->Update(force, forceVariation, forceOverLife);
	}
}

void ParticleLayer::RemoveForce(ParticleForce* force)
{
	Vector<ParticleForce*>::iterator iter = std::find(this->forces.begin(),
													  this->forces.end(),
													  force);
	if (iter != this->forces.end())
	{
		SafeRelease(*iter);
		this->forces.erase(iter);
	}
}

void ParticleLayer::RemoveForce(int32 forceIndex)
{
	if (forceIndex <= (int32)this->forces.size())
	{
		SafeRelease(this->forces[forceIndex]);
		this->forces.erase(this->forces.begin() + forceIndex);
	}
}

void ParticleLayer::CleanupForces()
{
	for (Vector<ParticleForce*>::iterator iter = this->forces.begin();
		 iter != this->forces.end(); iter ++)
	{
		SafeRelease(*iter);
	}
	
	this->forces.clear();
}

void ParticleLayer::FillSizeOverlifeXY(RefPtr< PropertyLine<float32> > sizeOverLife)
{
	Vector<PropValue<float32> > wrappedPropertyValues = PropLineWrapper<float32>(sizeOverLife).GetProps();
	if (wrappedPropertyValues.empty())
	{
		this->sizeOverLifeXY = NULL;
		return;
	}
	else if (wrappedPropertyValues.size() == 1)
	{
		Vector2 singleValue(wrappedPropertyValues[0].v, wrappedPropertyValues[0].v);
		this->sizeOverLifeXY = RefPtr< PropertyLine<Vector2> >(new PropertyLineValue<Vector2>(singleValue));
		return;
	}

	RefPtr<PropertyLineKeyframes<Vector2> > sizeOverLifeXYKeyframes =
		RefPtr<PropertyLineKeyframes<Vector2> >(new PropertyLineKeyframes<Vector2>);
	int32 propsCount = wrappedPropertyValues.size();
	for (int32 i = 0; i < propsCount; i ++)
	{
		Vector2 curValue(wrappedPropertyValues[i].v, wrappedPropertyValues[i].v);
		sizeOverLifeXYKeyframes->AddValue(wrappedPropertyValues[i].t, curValue);
	}
	
	this->sizeOverLifeXY = sizeOverLifeXYKeyframes;
}

void ParticleLayer::SetPlaybackSpeed(float32 value)
{
	this->playbackSpeed = Clamp(value, PARTICLE_EMITTER_MIN_PLAYBACK_SPEED,
								PARTICLE_EMITTER_MAX_PLAYBACK_SPEED);
	
	// Lookup through the active particles to update playback speed for the
	// inner emitters.
	UpdatePlaybackSpeedForInnerEmitters(value);
}

float32 ParticleLayer::GetPlaybackSpeed()
{
	return this->playbackSpeed;
}

int32 ParticleLayer::GetActiveParticlesCount()
{
	if (!innerEmitter)
	{
		return count;
	}

	// Ask each end every active particle's inner emitter regarding the total.
	int32 totalCount = count;
	Particle* current = head;
	while (current)
	{
		if (current->GetInnerEmitter())
		{
			ParticleEmitter* currentInnerEmitter = current->GetInnerEmitter();
			for (Vector<ParticleLayer*>::iterator iter = currentInnerEmitter->GetLayers().begin();
				 iter != currentInnerEmitter->GetLayers().end(); iter ++)
			{
				totalCount += (*iter)->GetActiveParticlesCount();
			}
		}
		
		current = current->next;
	}

	return totalCount;
}

float32 ParticleLayer::GetActiveParticlesArea()
{
	// Yuri Coder, 2013/04/16. Since the particles size are updated in runtime,
	// we have to recalculate their area each time this method is called.
	float32 activeArea = 0;
	Particle * current = head;
	while(current)
	{
		activeArea += current->GetArea();
		current = current->next;
	}

	return activeArea;
}

ParticleLayer::eType ParticleLayer::StringToLayerType(const String& layerTypeName, eType defaultLayerType)
{
	int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
	for (int32 i = 0; i < layerTypesCount; i ++)
	{
		if (layerTypeNamesInfoMap[i].layerTypeName == layerTypeName)
		{
			return layerTypeNamesInfoMap[i].layerType;
		}
	}
	
	return defaultLayerType;
}

String ParticleLayer::LayerTypeToString(eType layerType, const String& defaultLayerTypeName)
{
	int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
	for (int32 i = 0; i < layerTypesCount; i ++)
	{
		if (layerTypeNamesInfoMap[i].layerType == layerType)
		{
			return layerTypeNamesInfoMap[i].layerTypeName;
		}
	}
	
	return defaultLayerTypeName;
}

void ParticleLayer::UpdatePlaybackSpeedForInnerEmitters(float value)
{
	Particle* current = this->head;
	while (current)
	{
		if (current->GetInnerEmitter())
		{
			current->GetInnerEmitter()->SetPlaybackSpeed(value);
		}
		
		current = current->next;
	}
}

void ParticleLayer::CreateInnerEmitter()
{
	SafeRelease(innerEmitter);
	innerEmitter = new ParticleEmitter();
}

ParticleEmitter* ParticleLayer::GetInnerEmitter()
{
	return innerEmitter;
}

void ParticleLayer::RemoveInnerEmitter()
{
	DeleteAllParticles();
	if (innerEmitter)
	{
		innerEmitter->Stop();
	}

	SafeRelease(innerEmitter);
}

void ParticleLayer::PauseInnerEmitter(bool _isPaused)
{
	if (innerEmitter)
	{
		innerEmitter->Pause(_isPaused);
	}
}

void ParticleLayer::SetDisabled(bool value)
{
	this->isDisabled = value;
	
	// Update all the inner layers.
	Particle* current = this->head;
	while (current)
	{
		if (current->GetInnerEmitter())
		{
			current->GetInnerEmitter()->SetDisabledForAllLayers(value);
		}

		current = current->next;
	}
}

Vector2 ParticleLayer::GetPivotPoint() const
{
	return layerPivotPoint;
}

void ParticleLayer::SetPivotPoint(const Vector2& value)
{
	this->layerPivotPoint = value;
}

void ParticleLayer::HandleRemoveFromSystem()
{
	DeleteAllParticles();
}
	
};
