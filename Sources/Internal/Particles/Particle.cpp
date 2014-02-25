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


#include "Render/2D/Sprite.h"
#include "Render/RenderManager.h"
#include "Particles/Particle.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleEmitter3D.h"

namespace DAVA 
{
ForceData::ForceData(float32 forceValue, const Vector3& forceDirection, float32 forceOverlife,
					 bool forceOverlifeEnabled)
{
	this->value = forceValue;
	this->direction = forceDirection;
	this->overlife = forceOverlife;
	this->overlifeEnabled = forceOverlifeEnabled;
}

Particle::Particle()
{
    animTime = 0.0f;
	innerParticleEmitter = NULL;
}

Particle::~Particle()
{
    CleanupInnerEmitter();
}

void Particle::AddForce(float32 value, const Vector3& direction, float32 overlife,
			  bool overlifeEnabled)
{
	this->forces.push_back(ForceData(value, direction, overlife, overlifeEnabled));
}

void Particle::UpdateForceOverlife(int32 index, float32 overlife)
{
	if (index < (int32)this->forces.size())
	{
		this->forces[index].overlife = overlife;
		this->forces[index].overlifeEnabled = true;
	}
}
	
void Particle::CleanupForces()
{
	this->forces.clear();
}

	
bool Particle::Update(float32 timeElapsed)
{
	life += timeElapsed;
	if (life >= lifeTime)
	{
		return false;
	}

	position += speed * timeElapsed * velocityOverLife;
	angle += spin * timeElapsed * spinOverLife;
	
	int32 forcesCount = (int32)forces.size();
	
	for(int i = 0; i < forcesCount; i++)
	{
		Vector3 newVelocity = forces[i].direction * forces[i].value;
		if (forces[i].overlifeEnabled)
		{
			newVelocity *= forces[i].overlife;
		}

		speed += (newVelocity  * timeElapsed);
	}
	
	
	// In case the particle contains inner emitter - update it too.
	UpdateInnerEmitter(timeElapsed);
	return true;
}

void Particle::Draw()
{
	// Yuri Coder, 2013/04/03. This method is called for 2D sprites only. For 3D sprites
	// please see ParticleLayer3D::Draw()
	if (IsDead())return;
	RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	sprite->SetAngle(angle);
	sprite->SetPosition(position.x, position.y);
	sprite->SetScale(size.x * sizeOverLife.x, size.y * sizeOverLife.y);
	sprite->SetFrame(frame);
	sprite->Draw();
}

float32 Particle::GetArea()
{
	float resultSize = (size.x * sizeOverLife.x) * (size.y * sizeOverLife.y);
	if (sprite)
	{
		resultSize *= (sprite->GetWidth() * sprite->GetHeight());
	}
	
	return resultSize;
}

void Particle::InitializeInnerEmitter(ParticleEmitter* parentEmitter, ParticleEmitter* referenceEmitter)
{
	// Inner Emitter must not exist at this moment.
	DVASSERT(innerParticleEmitter == NULL);
	innerParticleEmitter = static_cast<ParticleEmitter*>(referenceEmitter->Clone(NULL));

	// After the emitter is loaded (and all its render batches are created),
	// register it in the Render System to get updates.
	innerParticleEmitter->SetRenderSystem(parentEmitter->GetRenderSystem());
	innerParticleEmitter->SetWorldTransformPtr(parentEmitter->GetWorldTransformPtr());
	innerParticleEmitter->RememberInitialTranslationVector();
	innerParticleEmitter->SetParentParticle(this);		
	innerParticleEmitter->SetAutoRestart(false);
	innerParticleEmitter->Play();

	RegisterInnerEmitterInRenderSystem(true);
}

ParticleEmitter* Particle::GetInnerEmitter()
{
	return innerParticleEmitter;
}

void Particle::CleanupInnerEmitter()
{
	if(NULL != innerParticleEmitter)
	{
		innerParticleEmitter->SetParentParticle(NULL);
		innerParticleEmitter->SetToBeDeleted(true);
	}
}

void Particle::UpdateInnerEmitter(float32 timeElapsed)
{
	if (!innerParticleEmitter)
	{
		return;
	}
	
	innerParticleEmitter->SetPosition(this->position);
}
	
void Particle::RegisterInnerEmitterInRenderSystem(bool isRegister)
{
	if (!innerParticleEmitter || !innerParticleEmitter->GetRenderSystem())
	{
		return;
	}

	RenderSystem* renderSystem = innerParticleEmitter->GetRenderSystem();
	if (isRegister)
	{
		renderSystem->RenderPermanent(innerParticleEmitter);
		renderSystem->MarkForUpdate(innerParticleEmitter);
	}
	else
	{
		renderSystem->RemoveFromRender(innerParticleEmitter);
	}
}

};
