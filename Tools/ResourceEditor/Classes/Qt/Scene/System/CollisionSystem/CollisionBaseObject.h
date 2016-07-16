#ifndef __SCENE_COLLISION_BASE_OBJECT_H__
#define __SCENE_COLLISION_BASE_OBJECT_H__

#include "bullet/btBulletCollisionCommon.h"
#include "Scene/Selectable.h"

class CollisionBaseObject
{
public:
    enum class ClassifyPointResult
    {
        InFront,
        Behind
    };

    enum class ClassifyPlaneResult
    {
        InFront,
        Intersects,
        Behind
    };

    enum class ClassifyPlanesResult
    {
        ContainsOrIntersects,
        Outside
    };

public:
    CollisionBaseObject(Selectable::Object* object_, btCollisionWorld* word)
        : object(object_)
        , btWord(word)
    {
    }

    virtual ~CollisionBaseObject() = default;

    virtual ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) = 0;
    virtual ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane> &planes) = 0;

    CollisionBaseObject::ClassifyPlaneResult ClassifyBoundingBoxToPlane(const DAVA::AABBox3& bbox, const DAVA::Plane& plane) const;
    DAVA::Plane TransformPlaneToLocalSpace(const DAVA::Plane& plane) const;

    btCollisionObject* btObject = nullptr;
    btCollisionWorld* btWord = nullptr;
    Selectable object;
};

inline CollisionBaseObject::ClassifyPlaneResult CollisionBaseObject::ClassifyBoundingBoxToPlane(const DAVA::AABBox3& bbox, const DAVA::Plane& plane) const
{
    char cornersData[8 * sizeof(DAVA::Vector3)];
    DAVA::Vector3* corners = reinterpret_cast<DAVA::Vector3*>(cornersData);
    bbox.GetCorners(corners);

    DAVA::float32 minDistance = std::numeric_limits<float>::max();
    DAVA::float32 maxDistance = -minDistance;
    for (DAVA::uint32 i = 0; i < 8; ++i)
    {
        float d = plane.DistanceToPoint(corners[i]);
        minDistance = std::min(minDistance, d);
        maxDistance = std::max(maxDistance, d);
    }

    if ((minDistance > 0.0f) && (maxDistance > 0.0f))
        return ClassifyPlaneResult::InFront;

    if ((minDistance < 0.0f) && (maxDistance < 0.0f))
        return ClassifyPlaneResult::Behind;

    return ClassifyPlaneResult::Intersects;
}

inline DAVA::Plane CollisionBaseObject::TransformPlaneToLocalSpace(const DAVA::Plane& plane) const
{
    DAVA::Matrix4 transform = object.GetWorldTransform();
    transform.Transpose();
    return DAVA::Plane(DAVA::Vector4(plane.n.x, plane.n.y, plane.n.z, plane.d) * transform);
}

#endif // __SCENE_COLLISION_BASE_OBJECT_H__
