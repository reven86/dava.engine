#ifndef __SCENE_COLLISION_LANDSCAPE_H__
#define __SCENE_COLLISION_LANDSCAPE_H__

#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"
#include "Render/Highlevel/Landscape.h"

class CollisionLandscape : public CollisionBaseObject
{
public:
    CollisionLandscape(DAVA::Entity* entity, btCollisionWorld* word, DAVA::Landscape* landscape);
    virtual ~CollisionLandscape();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes) override;

protected:
    btHeightfieldTerrainShape* btTerrain;
    DAVA::Vector<DAVA::float32> btHMap;
};

#endif // __SCENE_COLLISION_LANDSCAPE_H__
