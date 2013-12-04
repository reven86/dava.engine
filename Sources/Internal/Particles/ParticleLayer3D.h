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



#ifndef __DAVAENGINE_PARTICLE_LAYER_3D_H__
#define __DAVAENGINE_PARTICLE_LAYER_3D_H__

#include "Base/BaseTypes.h"
#include "Particles/ParticleLayer.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class RenderDataObject;
class Material;
class Camera;
class ParticleLayer3D : public ParticleLayer
{
public:
	ParticleLayer3D(ParticleEmitter* parent);
	virtual ~ParticleLayer3D();
	
	virtual ParticleLayer * Clone(ParticleLayer * dstLayer = 0);

	virtual void Draw(Camera * camera);

	// Draw method for generic and long emitters. former DrawLayer
	virtual void PrepareRenderData(Camera * camera);

	Material * GetMaterial();
	
	virtual void SetBlendMode(eBlendMode sFactor, eBlendMode dFactor);

	virtual void SetFog(bool enable);

	virtual void SetFrameBlend(bool enable);

	// Whether this layer should be drawn as "long" one?
	virtual bool IsLong();
	virtual void SetLong(bool value);

	// Access to parent.
	ParticleEmitter* GetParent() const {return emitter;};
	void SetParent(ParticleEmitter* parent) {this->emitter = parent;};
	
	// Create the inner emitter where needed.
	virtual void CreateInnerEmitter();

protected:
	void CalcNonLong(Particle* current,
							Vector3& topLeft,
							Vector3& topRight,
							Vector3& botLeft,
							Vector3& botRight);

	void CalcLong(Particle* current,
						 Vector3& topLeft,
						 Vector3& topRight,
						 Vector3& botLeft,
						 Vector3& botRight);
		

	// Update the current particle position according to the current emitter type.
	void UpdateCurrentParticlePosition(Particle* particle);

	bool isLong;

	RenderDataObject * renderData;
	Vector<float32> verts;
	Vector<float32> textures;
	Vector<float32> textures2;
	Vector<float32> times;
	Vector<uint32> colors;
	static Vector<uint16> indices;

	Vector3 _up;
	Vector3 _left;
	Vector3 direction;

	// Current position of the particle - cached for speedup.
	Vector3 currentParticlePosition;

public:
    //INTROSPECTION_EXTEND(ParticleLayer3D, ParticleLayer,
    //    MEMBER(material, "Material", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
   // );
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_3D_H__
