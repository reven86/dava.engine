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



#include "Render/Highlevel/ParticleLayerBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Material.h"
#include "Render/RenderManager.h"
#include "Particles/ParticleLayer.h"

namespace DAVA
{


ParticleLayerBatch::ParticleLayerBatch()
:	totalCount(0),
	particleLayer(0),
	indices(0)
{

}

ParticleLayerBatch::~ParticleLayerBatch()
{
}
void ParticleLayerBatch::SetLayerBoundingBox(const AABBox3 & bbox)
{
	aabbox = bbox;
}
void ParticleLayerBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
	//Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
	//if (!worldTransformPtr)return;

	uint32 flags = renderObject->GetFlags();
	if (((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA) || particleLayer->GetDisabled())
		return;

	Matrix4 worldMatrix = Matrix4::IDENTITY;
	Matrix4 finalMatrix = worldMatrix * camera->GetMatrix();
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);	

    particleLayer->Draw(camera); //note - it is mostly deprecated and is here for compatibility with old not-3d particles
    
	if(!totalCount)return;

	RenderManager::Instance()->SetRenderData(renderDataObject);
	material->PrepareRenderState();

	//RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, 6*totalCount);
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, 6*totalCount, EIF_16, &((*indices)[0]));
}

void ParticleLayerBatch::SetTotalCount(int32 _totalCount)
{
	totalCount = _totalCount;
}

RenderBatch * ParticleLayerBatch::Clone(RenderBatch * destination)
{
	ParticleLayerBatch * rb = new ParticleLayerBatch();

	return rb;
}

void ParticleLayerBatch::SetParticleLayer(ParticleLayer * _particleLayer)
{
	particleLayer = _particleLayer;
}

void ParticleLayerBatch::SetIndices(Vector<uint16> *_indices)
{
	indices = _indices;
}





}