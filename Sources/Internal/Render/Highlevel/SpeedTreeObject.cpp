#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SpeedTreeObject)
{
    ReflectionRegistrator<SpeedTreeObject>::Begin()
    .Field("lightSmoothing", &SpeedTreeObject::GetLightSmoothing, &SpeedTreeObject::SetLightSmoothing)[M::DisplayName("Light Smoothing")]
    .End();
}

const FastName SpeedTreeObject::FLAG_WIND_ANIMATION("WIND_ANIMATION");

SpeedTreeObject::SpeedTreeObject()
    :
    lightSmoothing(0.f)
{
    type = TYPE_SPEED_TREE;

    sphericalHarmonics = { 1.f / 0.564188f, 1.f / 0.564188f, 1.f / 0.564188f }; //fake SH value to make original object color
}

SpeedTreeObject::~SpeedTreeObject()
{
}

void SpeedTreeObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* rb = renderBatchArray[k].renderBatch;
        bbox.AddAABBox(CalcBBoxForSpeedTreeGeometry(rb));
    }
}

void SpeedTreeObject::SetTreeAnimationParams(const Vector2& trunkOscillationParams, const Vector2& leafOscillationParams)
{
    trunkOscillation = trunkOscillationParams;
    leafOscillation = leafOscillationParams;
}

void SpeedTreeObject::SetSphericalHarmonics(const DAVA::Array<float32, SpeedTreeObject::HARMONICS_BUFFER_CAPACITY>& coeffs)
{
    sphericalHarmonics = coeffs;
}

const DAVA::Array<float32, SpeedTreeObject::HARMONICS_BUFFER_CAPACITY>& SpeedTreeObject::GetSphericalHarmonics() const
{
    return sphericalHarmonics;
}

void SpeedTreeObject::SetLightSmoothing(const float32& smooth)
{
    lightSmoothing = smooth;
}

const float32& SpeedTreeObject::GetLightSmoothing() const
{
    return lightSmoothing;
}

void SpeedTreeObject::BindDynamicParameters(Camera* camera)
{
    RenderObject::BindDynamicParameters(camera);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SPEED_TREE_TRUNK_OSCILLATION, &trunkOscillation, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SPEED_TREE_LEAFS_OSCILLATION, &leafOscillation, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SPEED_TREE_LIGHT_SMOOTHING, &lightSmoothing, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SPHERICAL_HARMONICS, sphericalHarmonics.data(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}

void SpeedTreeObject::UpdateAnimationFlag(int32 maxAnimatedLod)
{
    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        int32 flagValue = (renderBatchArray[k].lodIndex > maxAnimatedLod) ? 0 : 1;

        auto material = renderBatchArray[k].renderBatch->GetMaterial();
        if (material->HasLocalFlag(FLAG_WIND_ANIMATION))
        {
            material->SetFlag(FLAG_WIND_ANIMATION, flagValue);
        }
        else
        {
            material->AddFlag(FLAG_WIND_ANIMATION, flagValue);
        }
        material->PreCacheFX();
    }
}

RenderObject* SpeedTreeObject::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<SpeedTreeObject>(this), "Can clone only SpeedTreeObject");
        newObject = new SpeedTreeObject();
    }

    RenderObject::Clone(newObject);

    SpeedTreeObject* treeObject = static_cast<SpeedTreeObject*>(newObject);
    treeObject->SetSphericalHarmonics(GetSphericalHarmonics());
    treeObject->SetLightSmoothing(GetLightSmoothing());

    return newObject;
}

void SpeedTreeObject::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Save(archive, serializationContext);

    archive->SetByteArray("sto.SHCoeff", reinterpret_cast<uint8*>(sphericalHarmonics.data()), int32(sphericalHarmonics.size() * sizeof(float32)));
    archive->SetFloat("sto.lightSmoothing", lightSmoothing);
}

void SpeedTreeObject::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);

    const float32* sphericalArray = reinterpret_cast<const float32*>(archive->GetByteArray("sto.SHCoeff"));
    int32 shCount = archive->GetInt32("sto.SHBasisCount"); //old SpeedTreeObject format
    if ((shCount > 0) && (shCount * 3 <= int32(sphericalHarmonics.size())))
        Memcpy(sphericalHarmonics.data(), sphericalArray, shCount * sizeof(Vector3));
    else
        Memcpy(sphericalHarmonics.data(), sphericalArray, sphericalHarmonics.size() * sizeof(float32));

    lightSmoothing = archive->GetFloat("sto.lightSmoothing", lightSmoothing);

    //RHI_COMPLETE TODO: Remove setting WIND_ANIMATION flag. We need to add/set flag manually (and save it) to reduce material prebuild count
    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        NMaterial* material = renderBatchArray[k].renderBatch->GetMaterial();
        if (!material->HasLocalFlag(FLAG_WIND_ANIMATION))
            material->AddFlag(FLAG_WIND_ANIMATION, 1);
        else
            material->SetFlag(FLAG_WIND_ANIMATION, 1);

        material->PreBuildMaterial(PASS_FORWARD);
    }
}

AABBox3 SpeedTreeObject::CalcBBoxForSpeedTreeGeometry(RenderBatch* rb)
{
    if (IsTreeLeafBatch(rb))
    {
        AABBox3 pgBbox;
        PolygonGroup* pg = rb->GetPolygonGroup();

        if ((pg->GetFormat() & EVF_PIVOT) == 0)
            return rb->GetBoundingBox();

        int32 vertexCount = pg->GetVertexCount();
        for (int32 vi = 0; vi < vertexCount; vi++)
        {
            Vector3 pivot;
            pg->GetPivot(vi, pivot);

            Vector3 pointX, pointY, pointZ;
            Vector3 offsetX, offsetY;

            pg->GetCoord(vi, pointZ);
            offsetX = offsetY = pointZ - pivot;

            Swap(offsetX.x, offsetX.z);
            Swap(offsetX.y, offsetX.z);

            pointX = pivot + offsetX;
            pointY = pivot + offsetY;

            pgBbox.AddPoint(pointX);
            pgBbox.AddPoint(pointY);
            pgBbox.AddPoint(pointZ);
        }

        return pgBbox;
    }

    return rb->GetBoundingBox();
}

bool SpeedTreeObject::IsTreeLeafBatch(RenderBatch* batch)
{
    if (batch && batch->GetMaterial())
    {
        const FastName& materialFXName = batch->GetMaterial()->GetEffectiveFXName();
        return (materialFXName == NMaterialName::SPEEDTREE_LEAF) || (materialFXName == NMaterialName::SPHERICLIT_SPEEDTREE_LEAF);
    }

    return false;
}
};
