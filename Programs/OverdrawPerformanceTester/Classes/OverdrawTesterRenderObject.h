#pragma once

#include "Base/BaseTypes.h"
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

    OverdrawTesterRenderObject(DAVA::float32 addOverdrawPercent_);
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
    DAVA::Array<OverdrawTesterRenderObject::QuadVertex, 6> GetQuad(float32 xStart, float32 xEnd);

    DAVA::Vector<QuadVertex> activeVerts;
    DAVA::uint32 vertexLayoutId;
    DAVA::NMaterial* material = nullptr;
    DAVA::RenderBatch* batch = nullptr;
    DAVA::float32 addOverdrawPercent;
    DAVA::float32 addOverdrawPercentNormalized;
    DAVA::uint32 stepsCount;
    DAVA::uint32 vertexStride;
    DAVA::uint32 currentStepsCount;
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
    batch->SetMaterial(material);
}

void OverdrawTesterRenderObject::RecalcBoundingBox()
{}
}
