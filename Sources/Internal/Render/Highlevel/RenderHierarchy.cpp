#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Frustum.h"

namespace DAVA
{
void LinearRenderHierarchy::AddRenderObject(RenderObject* object)
{
    renderObjectArray.push_back(object);
}

void LinearRenderHierarchy::RemoveRenderObject(RenderObject* renderObject)
{
    uint32 size = static_cast<uint32>(renderObjectArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (renderObjectArray[k] == renderObject)
        {
            renderObjectArray[k] = renderObjectArray[size - 1];
            renderObjectArray.pop_back();
            return;
        }
    }
    DVASSERT(0 && "Failed to find object");
}

void LinearRenderHierarchy::ObjectUpdated(RenderObject* renderObject)
{
}

void LinearRenderHierarchy::Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria)
{
    Frustum* frustum = camera->GetFrustum();
    uint32 size = static_cast<uint32>(renderObjectArray.size());
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject* node = renderObjectArray[pos];
        if ((node->GetFlags() & visibilityCriteria) != visibilityCriteria)
            continue;
        //still need to add flags for particles to dicede if to use DefferedUpdate
        if ((RenderObject::ALWAYS_CLIPPING_VISIBLE & node->GetFlags()) || frustum->IsInside(node->GetWorldBoundingBox()))
            visibilityArray.push_back(node);
    }
}

void LinearRenderHierarchy::GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray)
{
    uint32 size = static_cast<uint32>(renderObjectArray.size());
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject* ro = renderObjectArray[pos];
        if (bbox.IntersectsWithBox(ro->GetWorldBoundingBox()))
        {
            visibilityArray.push_back(ro);
        }
    }
}
};