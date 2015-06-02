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


#ifndef __DAVAENGINE_PARTICLE_GROUP_H_
#define __DAVAENGINE_PARTICLE_GROUP_H_

#include "ParticleEmitter.h"
#include "ParticleLayer.h"
#include "Particle.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{


struct ParticleGroup
{
	ParticleEmitter *emitter;
	ParticleLayer *layer;
	NMaterial *material;
	
	Particle *head;
	int32 activeParticleCount;
	
	bool finishingGroup;

	bool visibleLod;

	float32 time;	
	float32 loopStartTime, loopLyaerStartTime, loopDuration;
	float32 particlesToGenerate;	

	int32 positionSource;

    Vector3 spawnPosition;

  	ParticleGroup() : emitter(0), layer(0), material(0), head(0), activeParticleCount(0), finishingGroup(false), visibleLod(true), time(0), particlesToGenerate(0), positionSource(0){}
};

struct ParentInfo
{
	Vector3 position;
	Vector2 size;
};

struct ParticleEffectData
{
	Vector<ParentInfo> infoSources;
	List<ParticleGroup> groups;
};

}

#endif