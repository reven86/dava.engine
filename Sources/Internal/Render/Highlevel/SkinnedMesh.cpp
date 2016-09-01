#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Renderer.h"

namespace DAVA
{
SkinnedMesh::SkinnedMesh()
{
    type = TYPE_SKINNED_MESH;
    bbox = AABBox3(Vector3(0, 0, 0), Vector3(0, 0, 0));
    jointsCount = 0;
    positionArray = NULL;
    quaternionArray = NULL;
}

RenderObject* SkinnedMesh::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<SkinnedMesh>(this), "Can clone only SkinnedMesh");
        newObject = new SkinnedMesh();
    }
    RenderObject::Clone(newObject);
    return newObject;
}

void SkinnedMesh::BindDynamicParameters(Camera* camera)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &jointsCount, reinterpret_cast<pointer_size>(&jointsCount));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, &positionArray[0], reinterpret_cast<pointer_size>(positionArray));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, &quaternionArray[0], reinterpret_cast<pointer_size>(quaternionArray));

    RenderObject::BindDynamicParameters(camera);
}
}