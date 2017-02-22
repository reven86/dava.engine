#include "OverdrawTesterRenderObject.h"

#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/DynamicBufferAllocator.h"
#include "Base/BaseTypes.h"

namespace OverdrawPerformanceTester
{
using DAVA::NMaterial;
using DAVA::RenderBatch;
using DAVA::Vector2;
using DAVA::Vector3;
using DAVA::uint8;
using DAVA::uint16;
using DAVA::uint32;
using DAVA::int32;
using DAVA::float32;
using DAVA::DynamicBufferAllocator::AllocResultVB;
using DAVA::DynamicBufferAllocator::AllocResultIB;
using DAVA::Camera;

OverdrawTesterRenderObject::OverdrawTesterRenderObject(float32 addOverdrawPercent_, uint32 maxStepsCount_) 
    : addOverdrawPercent(addOverdrawPercent_), addOverdrawPercentNormalized(addOverdrawPercent_ * 0.01f)
{
    this->AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    this->AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    uint32 layoutId = rhi::VertexLayout::UniqueId(layout);

    vertexStride = (3 + 2) * sizeof(float);
    GenerateIndexBuffer();

    for (uint32 i = 0; i < maxStepsCount_; i++)
    {
        GenerateQuad(i, layoutId);
    }

    bbox.AddPoint(Vector3(1.0f, 1.0f, 1.0f));
}

void OverdrawTesterRenderObject::GenerateQuad(uint32 index, uint32 layoutId)
{
    float32 start = addOverdrawPercentNormalized * index;
    start = start - static_cast<int32>(start);
    start = start < 0.999f ? start : 0.0f;

    start = start * 2 - 1.0f;
    float32 end = start + 2.0f * addOverdrawPercentNormalized;
    end = end < 0.999f ? end : 1.0f;

    auto quad = GetQuadVerts(start, end);

    rhi::VertexBuffer::Descriptor desc;
    desc.usage = rhi::USAGE_STATICDRAW;
    desc.size = 4 * sizeof(QuadVertex);
    desc.initialData = quad.data();
    rhi::HVertexBuffer vBuffer = rhi::CreateVertexBuffer(desc);

    RenderBatch* renderBatch = new RenderBatch();
    renderBatch->SetRenderObject(this);
    renderBatch->vertexLayoutId = layoutId;
    renderBatch->vertexBuffer = vBuffer;
    renderBatch->indexBuffer = iBuffer;
    renderBatch->indexCount = 6;
    renderBatch->vertexCount = 6;

    quads.push_back(renderBatch);
}

OverdrawTesterRenderObject::~OverdrawTesterRenderObject()
{
    for (auto batch : quads)
    {
        if (batch->vertexBuffer.IsValid())
            rhi::DeleteVertexBuffer(batch->vertexBuffer);
        DAVA::SafeRelease(batch);
    }
    quads.clear();

    if (iBuffer.IsValid())
        rhi::DeleteIndexBuffer(iBuffer);
}

void OverdrawTesterRenderObject::PrepareToRender(DAVA::Camera* camera)
{
    activeRenderBatchArray.clear();
    if (material == nullptr || currentStepsCount == 0)
        return;
    activeVerts.clear();

    for (uint32 i = 0; i < currentStepsCount; i++)
        activeRenderBatchArray.push_back(quads[i]);
}

DAVA::Array<OverdrawTesterRenderObject::QuadVertex, 6> OverdrawTesterRenderObject::GetQuadVerts(float32 xStart, float32 xEnd)
{
    return
    { {
        { { xStart, -1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { xStart, 1.0f, 1.0f }, { 0.0f, 1.0f } },
        { { xEnd, 1.0f, 1.0f }, { 1.0f, 1.0f } },
        { { xEnd, -1.0f, 1.0f }, { 1.0f, 0.0f } }
    } };
}

void OverdrawTesterRenderObject::RecalculateWorldBoundingBox()
{
    worldBBox = bbox;
}

void OverdrawTesterRenderObject::BindDynamicParameters(Camera* camera)
{
    DAVA::Renderer::GetDynamicBindings().SetDynamicParam(DAVA::DynamicBindings::PARAM_WORLD, &DAVA::Matrix4::IDENTITY, reinterpret_cast<DAVA::pointer_size>(&DAVA::Matrix4::IDENTITY));
}

void OverdrawTesterRenderObject::GenerateIndexBuffer()
{
    DAVA::Array<uint16, 6> indices = { 0, 3, 1, 1, 3, 2 };
    rhi::IndexBuffer::Descriptor iDesc;
    iDesc.indexSize = rhi::INDEX_SIZE_16BIT;
    iDesc.size = 6 * sizeof(uint16);
    iDesc.usage = rhi::USAGE_STATICDRAW;
    iDesc.initialData = indices.data();
    iBuffer = rhi::CreateIndexBuffer(iDesc);
}

}