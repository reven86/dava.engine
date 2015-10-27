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


#include "ParticleRenderObject.h"
#include "Render/Renderer.h"
#include "Render/DynamicBufferAllocator.h"

namespace DAVA
{

//camera_facing, x_emitter, y_emitter, z_emitter, x_world, y_world, z_world
static Vector3 basisVectors[7*2] = {Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(0,1,0), Vector3(0,0,1), 
									Vector3(1,0,0), Vector3(0,0,1), 
									Vector3(0,1,0), Vector3(1,0,0)};

ParticleRenderObject::ParticleRenderObject(ParticleEffectData* effect)
    : effectData(effect)
    , sortingOffset(15)
{
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    //may be this is good place to determine 2d mode?

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    regularVertexLayoutId = rhi::VertexLayout::UniqueId(layout);
    layout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 2);
    layout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 1);
    frameBlendVertexLayoutId = rhi::VertexLayout::UniqueId(layout);
}

ParticleRenderObject::~ParticleRenderObject()
{
    for (size_t i = 0, sz = renderBatchCache.size(); i < sz; ++i)
    {
        SafeRelease(renderBatchCache[i]);
    }
}


void ParticleRenderObject::PrepareToRender(Camera *camera)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::PARTICLES_PREPARE_BUFFERS))
        return;

    PrepareRenderData(camera);

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::PARTICLES_DRAW))
    {
        activeRenderBatchArray.clear();
		return;
    }
}


void ParticleRenderObject::SetSortingOffset(uint32 offset)
{
    sortingOffset = offset;
    for (size_t i = 0, sz = renderBatchCache.size(); i < sz; ++i)
        renderBatchCache[i]->SetSortingOffset(offset);
}

void ParticleRenderObject::PrepareRenderData(Camera * camera)
{
    activeRenderBatchArray.clear();
    currRenderBatchId = 0;

    DVASSERT(worldTransform);

    Vector3 currCamDirection = camera->GetDirection();

    /*prepare effect basises*/
    const Matrix4& mv = camera->GetMatrix();
    basisVectors[0] = Vector3(mv._00, mv._10, mv._20);
	basisVectors[1] = Vector3(mv._01, mv._11, mv._21);
	basisVectors[0].Normalize();
	basisVectors[1].Normalize();	
	Vector3 ex(worldTransform->_00, worldTransform->_01, worldTransform->_02);
	Vector3 ey(worldTransform->_10, worldTransform->_11, worldTransform->_12);
	Vector3 ez(worldTransform->_20, worldTransform->_21, worldTransform->_22);
	ex.Normalize();
	ey.Normalize();
	ez.Normalize();
	basisVectors[2] = ey; basisVectors[3] = ez;
	basisVectors[4] = ex; basisVectors[5] = ez;
    basisVectors[6] = ey;
    basisVectors[7] = ex;

    auto itGroupStart = effectData->groups.begin();
    uint32 particlesInGroup = 0;
    for (List<ParticleGroup>::iterator itGroupCurr = effectData->groups.begin(), e = effectData->groups.end(); itGroupCurr != e; ++itGroupCurr)
    {
        const ParticleGroup& group = (*itGroupCurr);
        //note - isDisabled just stop it from being rendered, still processing particles in ParticleEffectSystem
        if ((!group.material) || (!group.head) || (group.layer->isDisabled) || (!group.layer->sprite))
            continue; //if no material was set up, or empty group, or layer rendering is disabled or sprite is removed - don't draw anyway

        if (itGroupStart->material != itGroupCurr->material)
        {
            /*currMaterial = currGroup.material;
			renderGroupCount++;
			if (renderGroupCache.size()<renderGroupCount)
			{
				currRenderGroup = new ParticleRenderGroup();
				currRenderGroup->renderBatch = new RenderBatch();
                currRenderGroup->renderBatch->SetSortingOffset(sortingOffset);
				renderGroupCache.push_back(currRenderGroup);
			}	
			else
				currRenderGroup=renderGroupCache[renderGroupCount-1];
			currRenderGroup->currParticlesCount = 0;
			currRenderGroup->enableFrameBlend = currGroup.layer->enableFrameBlend;
			currRenderGroup->renderBatch->SetMaterial(currMaterial);*/

            AppendParticleGroup(itGroupStart, itGroupCurr, particlesInGroup, currCamDirection);
            itGroupStart = itGroupCurr;
            particlesInGroup = 0;
        }
        particlesInGroup += CalculateParticleCount(*itGroupCurr);
    }
    if (itGroupStart != effectData->groups.end())
        AppendParticleGroup(itGroupStart, effectData->groups.end(), particlesInGroup, currCamDirection);
}
int32 ParticleRenderObject::CalculateParticleCount(const ParticleGroup& group)
{
    int32 basisCount = 0;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
        basisCount++;

    return group.activeParticleCount * basisCount;
}

struct ParticleVertex
{
    Vector3 pos;
    Vector2 uv;
    uint32 color;
    Vector2 uv1;
    float32 time;
};

//void ParticleRenderObject::AppendParticleGroup(const ParticleGroup &group, ParticleRenderGroup *renderGroup, const Vector3& cameraDirection)
void ParticleRenderObject::AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer)
{
    DVASSERT(particlesCount);

    //now we need to create batch
    if (currRenderBatchId >= renderBatchCache.size())
    {
        RenderBatch* newBatch = new RenderBatch();
        newBatch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        newBatch->SetRenderObject(this);
        renderBatchCache.push_back(newBatch);
        DVASSERT(renderBatchCache.size() > currRenderBatchId); //O_o
    }

    RenderBatch* targetBatch = renderBatchCache[currRenderBatchId];
    targetBatch->SetMaterial(material);

    targetBatch->vertexBuffer = vBuffer.buffer;
    targetBatch->vertexCount = vBuffer.allocatedVertices;
    targetBatch->vertexBase = vBuffer.baseVertex;

    targetBatch->indexCount = particlesCount * 6;
    targetBatch->indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(particlesCount);
    targetBatch->startIndex = 0;
    targetBatch->vertexLayoutId = vertexLayout;
    activeRenderBatchArray.push_back(targetBatch);
    currRenderBatchId++;
}

void ParticleRenderObject::AppendParticleGroup(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection)
{
    if (!particlesCount)
        return; //hmmm?

    uint32 vertexStride = (3 + 2 + 1) * sizeof(float); //vertex*3 + texcoord*2 + color * 1;
    if (begin->layer->enableFrameBlend)
        vertexStride += (2 + 1) * sizeof(float); //texcoord2 * 2 + time * 1;

    uint32 verteciesToAllocate = particlesCount * 4;
    DynamicBufferAllocator::AllocResultVB target = DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, verteciesToAllocate);
    uint8* currpos = target.data;

    uint32 verteciesAppended = 0;
    uint32 particleStride = vertexStride * 4;
    for (auto it = begin; it != end; ++it)
    {
        const ParticleGroup& group = *it;
        if ((!group.material) || (!group.head) || (group.layer->isDisabled) || (!group.layer->sprite))
            continue; //if no material was set up, or empty group, or layer rendering is disabled or sprite is removed - don't draw anyway

        //prepare basis indexes
        int32 basisCount = 0;
        int32 basises[4]; //4 basises max per particle
        bool worldAlign = (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_WORLD_ALIGN) != 0;
        if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
            basises[basisCount++] = 0;
        if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
            basises[basisCount++] = worldAlign ? 4 : 1;
        if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
            basises[basisCount++] = worldAlign ? 5 : 2;
        if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
            basises[basisCount++] = worldAlign ? 6 : 3;

        Particle* current = group.head;
        while (current)
        {
            float32* pT = group.layer->sprite->GetTextureVerts(current->frame);
            Color currColor = current->color;
            if (group.layer->colorOverLife)
                currColor = group.layer->colorOverLife->GetValue(current->life / current->lifeTime);
            if (group.layer->alphaOverLife)
                currColor.a = group.layer->alphaOverLife->GetValue(current->life / current->lifeTime);
            uint32 color = rhi::NativeColorRGBA(currColor.r, currColor.g, currColor.b, Min(currColor.a, 1.0f));
            float32 sin_angle;
            float32 cos_angle;
            SinCosFast(-current->angle, sin_angle, cos_angle); //- is because artists consider positive rotation to be clockwise

            for (int32 i = 0; i < basisCount; i++)
            {
                if (verteciesAppended + 4 > target.allocatedVertices)
                {
                    uint32 vLayout = group.layer->enableFrameBlend ? frameBlendVertexLayoutId : regularVertexLayoutId;
                    AppendRenderBatch(group.material, verteciesAppended / 4, vLayout, target);
                    verteciesToAllocate -= verteciesAppended;
                    verteciesAppended = 0;

                    target = DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, verteciesToAllocate);
                    currpos = target.data;
                }

                ParticleVertex* verts[4];
                verts[0] = (ParticleVertex*)currpos;
                verts[1] = (ParticleVertex*)(currpos + vertexStride);
                verts[2] = (ParticleVertex*)(currpos + 2 * vertexStride);
                verts[3] = (ParticleVertex*)(currpos + 3 * vertexStride);

                Vector3 ex = basisVectors[basises[i] * 2];
                Vector3 ey = basisVectors[basises[i] * 2 + 1];
                //TODO: rethink this code - it should be easier
                if (group.layer->isLong) //note that for now it's just a copy of long implementatio - later rethink it;
                {
                    ey = current->speed;
                    float32 vel = ey.Length();
                    ex = ey.CrossProduct(cameraDirection);
                    ex.Normalize();
                    ey *= (group.layer->scaleVelocityBase / vel + group.layer->scaleVelocityFactor); //optimized ex=(svBase+svFactor*vel)/vel
                }

                Vector3 left = ex * cos_angle + ey * sin_angle;
                Vector3 right = -left;
                Vector3 top = ey * (-cos_angle) + ex * sin_angle;
                Vector3 bot = -top;

                left *= 0.5f * current->currSize.x * (1 + group.layer->layerPivotPoint.x);
                right *= 0.5f * current->currSize.x * (1 - group.layer->layerPivotPoint.x);
                top *= 0.5f * current->currSize.y * (1 + group.layer->layerPivotPoint.y);
                bot *= 0.5f * current->currSize.y * (1 - group.layer->layerPivotPoint.y);

                Vector3 particlePosition = current->position;
                if (group.layer->inheritPosition)
                    particlePosition += effectData->infoSources[group.positionSource].position;
                verts[0]->pos = particlePosition + left + bot;
                verts[1]->pos = particlePosition + right + bot;
                verts[2]->pos = particlePosition + left + top;
                verts[3]->pos = particlePosition + right + top;

                for (int32 i = 0; i < 4; i++)
                {
                    verts[i]->uv.x = pT[i * 2];
                    verts[i]->uv.y = pT[i * 2 + 1];
                    verts[i]->color = color;
                }

                if (begin->layer->enableFrameBlend)
                {
                    int32 nextFrame = current->frame + 1;
                    if (nextFrame >= group.layer->sprite->GetFrameCount())
                    {
                        if (group.layer->loopSpriteAnimation)
                            nextFrame = 0;
                        else
                            nextFrame = group.layer->sprite->GetFrameCount() - 1;
                    }
                    float32* pT = group.layer->sprite->GetTextureVerts(nextFrame);

                    for (int32 i = 0; i < 4; i++)
                    {
                        verts[i]->uv1.x = *(pT++);
                        verts[i]->uv1.y = *(pT++);
                        verts[i]->time = current->animTime;
                    }
                }
                currpos += particleStride;
                verteciesAppended += 4;
            }
            current = current->next;
        }
    }

    if (verteciesAppended)
    {
        AppendRenderBatch(begin->material, verteciesAppended / 4, begin->layer->enableFrameBlend ? frameBlendVertexLayoutId : regularVertexLayoutId, target);
    }
}

void ParticleRenderObject::BindDynamicParameters(Camera* camera)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
}


}//namespace