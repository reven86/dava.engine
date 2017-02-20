#include "OverdrawTesterRenderObject.h"

#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/RenderBatch.h"

namespace OverdrawPerformanceTester
{

using DAVA::NMaterial;
using DAVA::RenderBatch;

OverdrawTesterRenderObject::OverdrawTesterRenderObject(NMaterial* drawMaterial) : material(drawMaterial)
{
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    batch = new RenderBatch();
}

OverdrawTesterRenderObject::~OverdrawTesterRenderObject()
{
    SafeRelease(batch);
}

void OverdrawTesterRenderObject::PrepareToRender(DAVA::Camera* camera)
{

}

void OverdrawTesterRenderObject::RecalculateWorldBoundingBox()
{
    worldBBox = bbox;
}

}