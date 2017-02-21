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
using DAVA::int32;
using DAVA::DynamicBufferAllocator::AllocResultVB;
using DAVA::DynamicBufferAllocator::AllocResultIB;
using DAVA::Camera;

OverdrawTesterRenderObject::OverdrawTesterRenderObject(DAVA::float32 addOverdrawPercent_)
    : addOverdrawPercent(addOverdrawPercent_), stepsCount(0), addOverdrawPercentNormalized(addOverdrawPercent_ * 0.01f)
{
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    batch = new RenderBatch();
    batch->SetRenderObject(this);

    vertexStride = (3 + 2) * sizeof(float);

    bbox.AddPoint(Vector3(1.0f, 1.0f, 1.0f));
}

OverdrawTesterRenderObject::~OverdrawTesterRenderObject()
{
    SafeRelease(batch);
}

void OverdrawTesterRenderObject::PrepareToRender(DAVA::Camera* camera)
{
    activeRenderBatchArray.clear();
    if (material == nullptr || currentStepsCount == 0)
        return;
    activeVerts.clear();
    for (uint32 i = 0; i < currentStepsCount; i++)
    {
        float32 start = addOverdrawPercentNormalized * i;
        start = start - static_cast<int32>(start);
        start = start < 0.999f ? start : 0.0f;

        start = start * 2 - 1.0f;
        float32 end = start + 2.0f * addOverdrawPercentNormalized;
        end = end < 0.999f ? end : 1.0f;

        auto quad = GetQuad(start, end);
        for (int j = 0; j < 6; j++)
            activeVerts.push_back(quad[j]);
    }
    uint32 vertsToAllocate = static_cast<uint32>(activeVerts.size());
    AllocResultVB vBuffer = DAVA::DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, vertsToAllocate);
    AllocResultIB iBuffer = DAVA::DynamicBufferAllocator::AllocateIndexBuffer(vertsToAllocate);
    uint8* currVert = vBuffer.data;
    uint16* currIndex = iBuffer.data;
    for (int i = 0; i < activeVerts.size(); i++)
    {

        QuadVertex* vert = reinterpret_cast<QuadVertex*>(currVert);
        vert->position = activeVerts[i].position;
        vert->texcoord = activeVerts[i].texcoord;
        *currIndex = i;

        currVert += vertexStride;
        currIndex++;
    }
    batch->vertexBuffer = vBuffer.buffer;
    batch->vertexCount = vertsToAllocate;
    batch->indexBuffer = iBuffer.buffer;
    batch->indexCount = vertsToAllocate;

    activeRenderBatchArray.push_back(batch);
}

DAVA::Array<OverdrawTesterRenderObject::QuadVertex, 6> OverdrawTesterRenderObject::GetQuad(float32 xStart, float32 xEnd)
{
    Vector3 p0(xStart, -1.0f, 0.0f);
    Vector3 p1(xStart, 1.0f, 0.0f);
    Vector3 p2(xEnd, 1.0f, 0.0f);
    Vector3 p3(xEnd, -1.0f, 0.0f);

    Vector2 t0(0.0f, 0.0f);
    Vector2 t1(0.0f, 1.0f);
    Vector2 t2(1.0f, 1.0f);
    Vector2 t3(1.0f, 0.0f);

    return
    { {
        { p0, t0 },
        { p3, t3 },
        { p1, t1 },
        { p3, t3 },
        { p2, t2 },
        { p1, t1 }
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

}