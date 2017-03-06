#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class NMaterial;
class RenderBatch;
class Camera;
}

namespace OverdrawPerformanceTester
{
class OverdrawTesterRenderObject : public DAVA::RenderObject
{
public:
    struct QuadVertex
    {
        DAVA::Vector3 position;
        DAVA::Vector2 texcoord;
    };

    OverdrawTesterRenderObject(DAVA::float32 addOverdrawPercent_, DAVA::uint32 maxStepsCount_, DAVA::uint16 textureResolution_);
    ~OverdrawTesterRenderObject();

    void PrepareToRender(DAVA::Camera* camera) override;
    void RecalculateWorldBoundingBox() override;

    void BindDynamicParameters(DAVA::Camera* camera) override;

    inline DAVA::uint32 GetCurrentStepsCount() const;
    inline void SetCurrentStepsCount(DAVA::uint32 newCount);

    inline DAVA::NMaterial* GetDrawMaterial() const;
    inline void SetDrawMaterial(DAVA::NMaterial* newMat);

    inline void RecalcBoundingBox() override;

private:
    void GenerateQuad(DAVA::uint32 index, DAVA::uint32 layoutId);
    DAVA::Array<OverdrawTesterRenderObject::QuadVertex, 4> GetQuadVerts(DAVA::float32 xStart, DAVA::float32 xEnd);
    void GenerateIndexBuffer();

    DAVA::Vector<QuadVertex> activeVerts;
    DAVA::NMaterial* material = nullptr;
    DAVA::uint32 vertexLayoutId;
    DAVA::float32 addOverdrawPercent;
    DAVA::float32 addOverdrawPercentNormalized;
    DAVA::uint32 vertexStride;
    DAVA::uint32 currentStepsCount;
    DAVA::uint16 textureResolution;

    DAVA::Vector<DAVA::RenderBatch*> quads;

    rhi::HIndexBuffer iBuffer;
};

DAVA::uint32 OverdrawTesterRenderObject::GetCurrentStepsCount() const
{
    return currentStepsCount;
}

void OverdrawTesterRenderObject::SetCurrentStepsCount(DAVA::uint32 newCount)
{
    currentStepsCount = newCount;
}

DAVA::NMaterial* OverdrawTesterRenderObject::GetDrawMaterial() const
{
    return material;
}

void OverdrawTesterRenderObject::SetDrawMaterial(DAVA::NMaterial* newMat)
{
    material = newMat;
    for (auto batch : quads)
        batch->SetMaterial(material);
}

void OverdrawTesterRenderObject::RecalcBoundingBox()
{
}
}
