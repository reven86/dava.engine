#ifndef __SCENE_COLLISION_BOX_H__
#define __SCENE_COLLISION_BOX_H__

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"

class CollisionBox : public CollisionBaseObject
{
public:
    CollisionBox(Selectable::Object* object, btCollisionWorld* word, DAVA::Vector3 position, DAVA::float32 boxSize);
    ~CollisionBox();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes) override;

private:
    btCollisionShape* btShape = nullptr;
};

#endif // __SCENE_COLLISION_BOX_H__
