#include "Render/Highlevel/BillboardRenderObject.h"

namespace DAVA
{
BillboardRenderObject::BillboardRenderObject()
{
    type = RenderObject::eType::TYPE_BILLBOARD;
    flags |= RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER;
}

void BillboardRenderObject::RecalcBoundingBox()
{
    bbox = AABBox3();
    for (const IndexedRenderBatch& i : renderBatchArray)
    {
        RenderBatch* batch = i.renderBatch;
        bbox.AddAABBox(batch->GetBoundingBox().GetMaxRotationExtentBox(Vector3(0.0f, 0.0f, 0.0f)));
    }
}

void BillboardRenderObject::PrepareToRender(Camera* camera)
{
    Matrix4 wTransform = *worldTransform;
    Vector3 translation = wTransform.GetTranslationVector();
    wTransform.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));

    Vector3 up = camera->GetUp();
    Vector3 dir = translation - camera->GetPosition();
    Vector3 side;

    if (billboardType == BillboardType::BILLBOARD_CYLINDRICAL)
    {
        side = dir.CrossProduct(up);
        side.Normalize();
        dir = up.CrossProduct(side);
    }
    else
    {
        dir.Normalize();
        side = dir.CrossProduct(up);
        side.Normalize();
        up = side.CrossProduct(dir);
    }

    Matrix4 bMatrix
    (
    side.x, side.y, side.z, 0.0,
    dir.x, dir.y, dir.z, 0.0,
    up.x, up.y, up.z, 0.0,
    0.0, 0.0, 0.0, 1.0
    );

    billboardTransform = bMatrix * wTransform;
    billboardTransform.SetTranslationVector(translation);
}

void BillboardRenderObject::BindDynamicParameters(Camera* camera)
{
    RenderObject::BindDynamicParameters(camera);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &billboardTransform, reinterpret_cast<pointer_size>(&billboardTransform));
}

void BillboardRenderObject::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Save(archive, serializationContext);
    archive->SetUInt32("billboardrenderobject.billboardType", static_cast<uint32>(billboardType));
}

void BillboardRenderObject::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);
    billboardType = static_cast<BillboardType>(archive->GetUInt32("billboardrenderobject.billboardType"));
}

void BillboardRenderObject::SetBillboardType(uint32 type)
{
    DVASSERT((type == BillboardType::BILLBOARD_SPHERICAL) || (type == BillboardType::BILLBOARD_CYLINDRICAL));
    billboardType = type;
    RecalcBoundingBox();
}
}
