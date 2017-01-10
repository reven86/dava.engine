#ifndef __SCENE_COLLISION_RENDER_OBJECT_H__
#define __SCENE_COLLISION_RENDER_OBJECT_H__

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"
#include "Render/Highlevel/RenderObject.h"

class CollisionRenderObject : public CollisionBaseObject
{
public:
    CollisionRenderObject(DAVA::Entity* entity, btCollisionWorld* word, DAVA::RenderObject* renderObject);
    ~CollisionRenderObject() override;

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes) override;

protected:
    btTriangleMesh* btTriangles = nullptr;
    btCollisionShape* btShape = nullptr;
};

#endif // __SCENE_COLLISION_BASE_OBJECT_H__
