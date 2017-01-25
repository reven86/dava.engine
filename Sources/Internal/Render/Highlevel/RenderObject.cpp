#include "Render/Highlevel/RenderObject.h"
#include "Base/ObjectFactory.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Render/Renderer.h"

namespace DAVA
{
RenderObject::RenderObject()
{
    lights[0] = NULL;
    lights[1] = NULL;
}

RenderObject::~RenderObject()
{
    for (IndexedRenderBatch& batch : renderBatchArray)
    {
        SafeRelease(batch.renderBatch);
    }
}

void RenderObject::AddRenderBatch(RenderBatch* batch)
{
    AddRenderBatch(batch, -1, -1);
}

void RenderObject::AddRenderBatch(RenderBatch* batch, int32 _lodIndex, int32 _switchIndex)
{
    batch->Retain();
    DVASSERT((batch->GetRenderObject() == 0) || (batch->GetRenderObject() == this));
    batch->SetRenderObject(this);

    IndexedRenderBatch ind;
    ind.lodIndex = _lodIndex;
    ind.switchIndex = _switchIndex;
    ind.renderBatch = batch;
    renderBatchArray.push_back(ind);

    if ((_lodIndex == lodIndex && _switchIndex == switchIndex) || (_lodIndex == -1 && _switchIndex == -1))
    {
        activeRenderBatchArray.push_back(batch);
    }

    if (renderSystem)
        renderSystem->RegisterBatch(batch);

    RecalcBoundingBox();
}

void RenderObject::RemoveRenderBatch(RenderBatch* batch)
{
    batch->SetRenderObject(0);

    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (renderBatchArray[k].renderBatch == batch)
        {
            if (renderSystem)
                renderSystem->UnregisterBatch(batch);
            batch->Release();

            renderBatchArray[k] = renderBatchArray[size - 1];
            renderBatchArray.pop_back();
            k--;
            size--;
        }
    }

    UpdateActiveRenderBatches();

    RecalcBoundingBox();
}

void RenderObject::RemoveRenderBatch(uint32 batchIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatch* batch = renderBatchArray[batchIndex].renderBatch;
    if (renderSystem)
        renderSystem->UnregisterBatch(batch);

    batch->SetRenderObject(0);
    batch->Release();

    renderBatchArray[batchIndex] = renderBatchArray[size - 1];
    renderBatchArray.pop_back();
    FindAndRemoveExchangingWithLast(activeRenderBatchArray, batch);

    RecalcBoundingBox();
}

void RenderObject::ReplaceRenderBatch(RenderBatch* oldBatch, RenderBatch* newBatch)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (renderBatchArray[k].renderBatch == oldBatch)
        {
            ReplaceRenderBatch(k, newBatch);
            return;
        }
    }
}

void RenderObject::ReplaceRenderBatch(uint32 batchIndex, RenderBatch* newBatch)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatch* batch = renderBatchArray[batchIndex].renderBatch;
    renderBatchArray[batchIndex].renderBatch = newBatch;

    batch->SetRenderObject(0);
    newBatch->SetRenderObject(this);

    if (renderSystem)
    {
        renderSystem->UnregisterBatch(batch);
        renderSystem->RegisterBatch(newBatch);
    }

    batch->Release();
    newBatch->Retain();

    UpdateActiveRenderBatches();
    RecalcBoundingBox();
}

void RenderObject::SetRenderBatchLODIndex(uint32 batchIndex, int32 newLodIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    IndexedRenderBatch& iBatch = renderBatchArray[batchIndex];
    iBatch.lodIndex = newLodIndex;

    UpdateActiveRenderBatches();
}

void RenderObject::SetRenderBatchSwitchIndex(uint32 batchIndex, int32 newSwitchIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    IndexedRenderBatch& iBatch = renderBatchArray[batchIndex];
    iBatch.switchIndex = newSwitchIndex;

    UpdateActiveRenderBatches();
}

void RenderObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    for (const IndexedRenderBatch& i : renderBatchArray)
    {
        RenderBatch* batch = i.renderBatch;
        bbox.AddAABBox(batch->GetBoundingBox());
    }
}

void RenderObject::CollectRenderBatches(int32 requestLodIndex, int32 requestSwitchIndex, Vector<RenderBatch*>& batches, bool includeShareLods /* = false */) const
{
    uint32 batchesCount = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < batchesCount; ++i)
    {
        const IndexedRenderBatch& irb = renderBatchArray[i];
        if ((requestLodIndex == -1 || requestLodIndex == irb.lodIndex || (includeShareLods && irb.lodIndex == -1)) &&
            (requestSwitchIndex == -1 || requestSwitchIndex == irb.switchIndex))
        {
            batches.push_back(irb.renderBatch);
        }
    }
}

RenderObject* RenderObject::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<RenderObject>(this), "Can clone only RenderObject");
        newObject = new RenderObject();
    }

    newObject->flags = flags;
    newObject->RemoveFlag(MARKED_FOR_UPDATE);
    newObject->debugFlags = debugFlags;
    newObject->staticOcclusionIndex = staticOcclusionIndex;
    newObject->lodIndex = lodIndex;
    newObject->switchIndex = switchIndex;
    //ro->bbox = bbox;
    //ro->worldBBox = worldBBox;

    //TODO:VK: Do we need remove all renderbatches from newObject?
    DVASSERT(newObject->GetRenderBatchCount() == 0);

    uint32 size = GetRenderBatchCount();
    newObject->renderBatchArray.reserve(size);
    for (uint32 i = 0; i < size; ++i)
    {
        int32 batchLodIndex, batchSwitchIndex;
        RenderBatch* batch = GetRenderBatch(i, batchLodIndex, batchSwitchIndex)->Clone();
        newObject->AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
        batch->Release();
    }
    newObject->ownerDebugInfo = ownerDebugInfo;

    return newObject;
}

void RenderObject::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    AnimatedObject::SaveObject(archive);

    if (NULL != archive)
    {
        archive->SetUInt32("ro.debugflags", debugFlags);
        archive->SetUInt32("ro.batchCount", GetRenderBatchCount());
        archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

        //VI: save only VISIBLE flag for now. May be extended in the future
        archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

        KeyedArchive* batchesArch = new KeyedArchive();
        for (uint32 i = 0; i < GetRenderBatchCount(); ++i)
        {
            RenderBatch* batch = GetRenderBatch(i);
            if (NULL != batch)
            {
                archive->SetInt32(Format("rb%d.lodIndex", i), renderBatchArray[i].lodIndex);
                archive->SetInt32(Format("rb%d.switchIndex", i), renderBatchArray[i].switchIndex);
                KeyedArchive* batchArch = new KeyedArchive();
                batch->Save(batchArch, serializationContext);
                if (batchArch->Count() > 0)
                {
                    batchArch->SetString("rb.classname", batch->GetClassName());
                }
                batchesArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), batchArch);
                batchArch->Release();
            }
        }

        archive->SetArchive("ro.batches", batchesArch);
        batchesArch->Release();
    }
}

void RenderObject::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        debugFlags = archive->GetUInt32("ro.debugflags", 0);
        staticOcclusionIndex = static_cast<uint16>(archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX));

        //VI: load only VISIBLE flag for now. May be extended in the future.
        uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);

        flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));

        uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        KeyedArchive* batchesArch = archive->GetArchive("ro.batches");
        for (uint32 i = 0; i < roBatchCount; ++i)
        {
            int32 batchLodIndex = archive->GetInt32(Format("rb%d.lodIndex", i), -1);
            int32 batchSwitchIndex = archive->GetInt32(Format("rb%d.switchIndex", i), -1);

            KeyedArchive* batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (NULL != batchArch)
            {
                RenderBatch* batch = ObjectFactory::Instance()->New<RenderBatch>(batchArch->GetString("rb.classname"));

                if (NULL != batch)
                {
                    batch->Load(batchArch, serializationContext);
                    AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
                    batch->Release();
                }
            }
        }
    }

    AnimatedObject::LoadObject(archive);
}

void RenderObject::BindDynamicParameters(Camera* camera)
{
    DVASSERT(worldTransform != 0);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, worldTransform, reinterpret_cast<pointer_size>(worldTransform));

    if (camera && lights[0])
    {
        const Vector4& lightPositionDirection0InCameraSpace = lights[0]->CalculatePositionDirectionBindVector(camera);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &lightPositionDirection0InCameraSpace, reinterpret_cast<pointer_size>(&lightPositionDirection0InCameraSpace));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &lights[0]->GetDiffuseColor(), reinterpret_cast<pointer_size>(lights[0]));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_AMBIENT_COLOR, &lights[0]->GetAmbientColor(), reinterpret_cast<pointer_size>(lights[0]));
    }
    else
    {
        //in case we don't have light we are to bind some default values to prevent fall or using previously bound light producing strange artifacts
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &Vector4::Zero, reinterpret_cast<pointer_size>(&Vector4::Zero));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_AMBIENT_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));
    }

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LOCAL_BOUNDING_BOX, &bbox, reinterpret_cast<pointer_size>(&bbox));
}

void RenderObject::SetRenderSystem(RenderSystem* _renderSystem)
{
    renderSystem = _renderSystem;
}

RenderSystem* RenderObject::GetRenderSystem()
{
    return renderSystem;
}

void RenderObject::BakeGeometry(const Matrix4& transform)
{
}

void RenderObject::RecalculateWorldBoundingBox()
{
    DVASSERT(!bbox.IsEmpty());
    DVASSERT(worldTransform != nullptr);
    bbox.GetTransformedBox(*worldTransform, worldBBox);
}

void RenderObject::PrepareToRender(Camera* camera)
{
}

void RenderObject::SetLodIndex(int32 _lodIndex)
{
    if (lodIndex != _lodIndex)
    {
        lodIndex = _lodIndex;
        UpdateActiveRenderBatches();
    }
}

void RenderObject::SetSwitchIndex(int32 _switchIndex)
{
    if (switchIndex != _switchIndex)
    {
        switchIndex = _switchIndex;
        UpdateActiveRenderBatches();
    }
}

int32 RenderObject::GetLodIndex() const
{
    return lodIndex;
}

int32 RenderObject::GetSwitchIndex() const
{
    return switchIndex;
}

void RenderObject::UpdateActiveRenderBatches()
{
    activeRenderBatchArray.clear();
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        IndexedRenderBatch& irb = renderBatchArray[i];
        if ((irb.lodIndex == lodIndex || -1 == irb.lodIndex) && (irb.switchIndex == switchIndex || -1 == irb.switchIndex))
        {
            activeRenderBatchArray.push_back(irb.renderBatch);
        }
    }
}

int32 RenderObject::GetMaxLodIndex() const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch& irb = renderBatchArray[i];
        ret = Max(ret, irb.lodIndex);
    }

    return ret;
}

int32 RenderObject::GetMaxLodIndexForSwitchIndex(int32 forSwitchIndex) const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch& irb = renderBatchArray[i];
        if (irb.switchIndex == forSwitchIndex)
        {
            ret = Max(ret, irb.lodIndex);
        }
    }

    return ret;
}

int32 RenderObject::GetMaxSwitchIndex() const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const IndexedRenderBatch& irb = renderBatchArray[i];
        ret = Max(ret, irb.switchIndex);
    }

    return ret;
}

void RenderObject::GetDataNodes(Set<DataNode*>& dataNodes)
{
    for (IndexedRenderBatch& batch : renderBatchArray)
        batch.renderBatch->GetDataNodes(dataNodes);
}
};
