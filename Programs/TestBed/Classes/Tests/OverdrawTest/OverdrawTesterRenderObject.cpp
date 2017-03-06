#include "OverdrawTesterRenderObject.h"

#include "Base/BaseTypes.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/DynamicBufferAllocator.h"
#include "UI/UIControlSystem.h"

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
using DAVA::Size2i;

OverdrawTesterRenderObject::OverdrawTesterRenderObject(float32 addOverdrawPercent_, uint32 maxStepsCount_, uint16 textureResolution_)
    : addOverdrawPercent(addOverdrawPercent_)
    , addOverdrawPercentNormalized(addOverdrawPercent_ * 0.01f)
    , textureResolution(textureResolution_)
{
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

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

void OverdrawTesterRenderObject::RecalculateWorldBoundingBox()
{
    worldBBox = bbox;
}

void OverdrawTesterRenderObject::BindDynamicParameters(Camera* camera)
{
    DAVA::Renderer::GetDynamicBindings().SetDynamicParam(DAVA::DynamicBindings::PARAM_WORLD, &DAVA::Matrix4::IDENTITY, reinterpret_cast<DAVA::pointer_size>(&DAVA::Matrix4::IDENTITY));
}

void OverdrawTesterRenderObject::GenerateQuad(uint32 index, uint32 layoutId)
{
    static const float32 threshold = 0.999f;

    float32 start = addOverdrawPercentNormalized * index;
    start = start - static_cast<int32>(start);
    start = start < threshold ? start : 0.0f;

    start = start * 2 - 1.0f;
    float32 end = start + 2.0f * addOverdrawPercentNormalized;
    end = end < threshold ? end : 1.0f;

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
    renderBatch->vertexCount = 4;

    quads.push_back(renderBatch);
}

DAVA::Array<OverdrawTesterRenderObject::QuadVertex, 4> OverdrawTesterRenderObject::GetQuadVerts(float32 xStart, float32 xEnd)
{
    // Try to keep 2pix - 1tex ratio.
    Size2i size = DAVA::UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();

    float32 maxX = size.dx * 0.5f / textureResolution;
    float32 maxY = size.dy * 0.5f / textureResolution;

    float32 uvStart = (xStart * 0.5f + 0.5f) * maxX;
    float32 uvEnd = (xEnd * 0.5f + 0.5f) * maxX;
    return
    { {
    { { xStart, -1.0f, 1.0f }, { uvStart, 0.0f } },
    { { xStart, 1.0f, 1.0f }, { uvStart, maxY } },
    { { xEnd, 1.0f, 1.0f }, { uvEnd, maxY } },
    { { xEnd, -1.0f, 1.0f }, { uvEnd, 0.0f } }
    } };
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