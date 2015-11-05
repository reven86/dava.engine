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


#ifndef __DAVAENGINE_PARTICLE_RENDER_OBJECT_H_
#define __DAVAENGINE_PARTICLE_RENDER_OBJECT_H_

#include "ParticleGroup.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/DynamicBufferAllocator.h"

namespace DAVA
{



class ParticleRenderObject : public RenderObject
{
	ParticleEffectData *effectData;
    //Vector<ParticleRenderGroup*> renderGroupCache;
    Vector<RenderBatch*> renderBatchCache;

    //void AppendParticleGroup(const ParticleGroup &group, ParticleRenderGroup *renderGroup, const Vector3& cameraDirection);
    void AppendParticleGroup(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection);
    void AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer);
    void PrepareRenderData(Camera* camera);
    Vector<uint16> indices;
    uint32 sortingOffset;

    uint32 currRenderBatchId;

public:
	ParticleRenderObject(ParticleEffectData *effect);
	~ParticleRenderObject();
	

	virtual void PrepareToRender(Camera *camera);	    

    void SetSortingOffset(uint32 offset);

    virtual void BindDynamicParameters(Camera * camera);
	virtual void RecalcBoundingBox(){}
	virtual void RecalculateWorldBoundingBox(){
		worldBBox = bbox;}

private:
    int32 CalculateParticleCount(const ParticleGroup& group);

    uint32 regularVertexLayoutId, frameBlendVertexLayoutId;
};

}

#endif