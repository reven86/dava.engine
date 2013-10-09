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



#ifndef __DAVAENGINE_PARTICLE_EMITTER_3D_H__
#define __DAVAENGINE_PARTICLE_EMITTER_3D_H__

#include "Particles/ParticleEmitter.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class Camera;
class ParticleEmitter3D : public ParticleEmitter
{
public:
	ParticleEmitter3D();

	virtual void AddLayer(ParticleLayer * layer);
	virtual void InsertLayer(ParticleLayer * layer, ParticleLayer * beforeLayer);

	virtual bool Is3DFlagCorrect();

	void Draw(Camera * camera);
	virtual void RenderUpdate(Camera *camera, float32 timeElapsed);

	virtual RenderObject * Clone(RenderObject *newObject);

	virtual void RecalcBoundingBox();

protected:
	// Virtual methods which are different for 2D and 3D emitters.
	virtual void PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex);
	virtual void LoadParticleLayerFromYaml(const YamlNode* yamlNode, bool isLiong);
	
	// 3D-specific methods.
	void CalculateParticlePositionForCircle(Particle* particle, const Vector3& tempPosition,
											const Matrix3& rotationMatrix);
	void PrepareEmitterParametersShockwave(Particle * particle, float32 velocity,
										   int32 emitIndex, const Vector3& tempPosition,
										   const Matrix3& rotationMatrix);
	void PrepareEmitterParametersGeneric(Particle * particle, float32 velocity,
										 int32 emitIndex, const Vector3& tempPosition,
										 const Matrix3& rotationMatrix);
};

};

#endif //__DAVAENGINE_PARTICLE_EMITTER_3D_H__