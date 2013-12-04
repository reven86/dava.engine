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


#ifndef __DAVAENGINE_PARTICLE_H__
#define __DAVAENGINE_PARTICLE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/AllocatorFactory.h"

namespace DAVA 
{
// Hepler class to store Force-specific data.
class ForceData
{
public:
	ForceData(float32 forceValue, const Vector3& forceDirection, float32 forceOverlife,
			  bool forceOverlifeEnabled);

	float32 value;
	Vector3 direction;
	float32 overlife;
	bool overlifeEnabled;
};

class Sprite;
class ParticleEmitter;
	
class Particle
{
public:
	IMPLEMENT_POOL_ALLOCATOR(Particle, 1000);

	Particle();
	virtual ~Particle();
	
	inline bool	IsDead();
	bool	Update(float32 timeElapsed);
	void	Draw();

	Sprite * sprite;
	Particle * next;

	Color color;
	Color drawColor;
	Vector3 position;
	Vector2 size;

	//Vector3 direction;
	Vector3 speed;

	float32 angle;
	float32 spin;

	float32 life;			// in seconds
	float32 lifeTime;		// in seconds

	Vector2 sizeOverLife;
	float32 velocityOverLife;
	float32 spinOverLife;

	// Add the new force,
	void AddForce(float32 value, const Vector3& direction, float32 overlife, bool overlifeEnabled);
	
	// Update the Force Overlife value.
	void UpdateForceOverlife(int32 index, float32 overlife);

	// Cleanup the existing forces.
	void CleanupForces();

	// Get the area currently occupied by particle.
	float32 GetArea();

	// YuriCoder, 2013/04/25. SuperEmitter functionality.
	// Initialize/cleanup the inner emitter for Particle.
	void InitializeInnerEmitter(ParticleEmitter* parentEmitter, ParticleEmitter* referenceEmitter);
	void CleanupInnerEmitter();
	ParticleEmitter* GetInnerEmitter();

	int32	frame;
	float32 animTime;
	
	friend class ParticleEmitter;
	
protected:
	// Register/unregister Inner Emitter in Render Systtem.
	void RegisterInnerEmitterInRenderSystem(bool isRegister);

	// Update the position of Inner Emitter.
	void UpdateInnerEmitter(float32 timeElapsed);

	// Forces attached to Particle.
	Vector<ForceData> forces;
	
	// Inner Particle Emitter.
	ParticleEmitter* innerParticleEmitter;
};

// Implementation
inline bool Particle::IsDead()
{
	return (life > lifeTime);
}

};

#endif // __DAVAENGINE_PARTICLE_H__
