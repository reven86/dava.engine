#ifndef __DAVAENGINE_RENDER_HIERARCHY_H__
#define __DAVAENGINE_RENDER_HIERARCHY_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Render/UniqueStateSet.h"
//#include "Render/Highlevel/LayerSetUniqueHandler.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class RenderObject;
class Camera;
class RenderHierarchy
{
public:
    virtual ~RenderHierarchy()
    {
    }

    virtual void AddRenderObject(RenderObject* renderObject) = 0;
    virtual void RemoveRenderObject(RenderObject* renderObject) = 0;
    virtual void ObjectUpdated(RenderObject* renderObject) = 0;
    virtual void Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria) = 0;

    virtual void GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray) = 0;

    virtual void Initialize()
    {
    }
    virtual void Update()
    {
    }
    virtual void DebugDraw(const Matrix4& cameraMatrix)
    {
    }
};

class LinearRenderHierarchy : public RenderHierarchy
{
    void AddRenderObject(RenderObject* renderObject) override;
    void RemoveRenderObject(RenderObject* renderObject) override;
    void ObjectUpdated(RenderObject* renderObject) override;
    void Clip(Camera* camera, Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria) override;
    void GetAllObjectsInBBox(const AABBox3& bbox, Vector<RenderObject*>& visibilityArray) override;

private:
    Vector<RenderObject*> renderObjectArray;
};

} // ns

#endif /* __DAVAENGINE_RENDER_HIERARCHY_H__ */
