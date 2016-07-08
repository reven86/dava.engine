#ifndef __DAVAENGINE_PARTICLE_RENDER_OBJECT_H_
#define __DAVAENGINE_PARTICLE_RENDER_OBJECT_H_

#include "ParticleGroup.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/DynamicBufferAllocator.h"

namespace DAVA
{
class ParticleRenderObject : public RenderObject
{
    ParticleEffectData* effectData;
    //Vector<ParticleRenderGroup*> renderGroupCache;
    Vector<RenderBatch*> renderBatchCache;

    //void AppendParticleGroup(const ParticleGroup &group, ParticleRenderGroup *renderGroup, const Vector3& cameraDirection);
    void AppendParticleGroup(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection, Vector3* basisVectors);
    void AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer);
    void PrepareRenderData(Camera* camera);
    Vector<uint16> indices;
    uint32 sortingOffset;

    uint32 currRenderBatchId;

public:
    ParticleRenderObject(ParticleEffectData* effect);
    ~ParticleRenderObject();

    virtual void PrepareToRender(Camera* camera);

    void SetSortingOffset(uint32 offset);

    virtual void BindDynamicParameters(Camera* camera);
    virtual void RecalcBoundingBox()
    {
    }
    virtual void RecalculateWorldBoundingBox()
    {
        worldBBox = bbox;
    }

private:
    int32 CalculateParticleCount(const ParticleGroup& group);

    uint32 regularVertexLayoutId, frameBlendVertexLayoutId;
};
}

#endif