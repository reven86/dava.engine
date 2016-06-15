#include "Scene/System/CollisionSystem/CollisionBox.h"

CollisionBox::CollisionBox(Selectable::Object* object_, btCollisionWorld* word, DAVA::Vector3 position, DAVA::float32 boxSize)
    : CollisionBaseObject(object_, word)
{
    if (word != nullptr)
    {
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(position.x, position.y, position.z));

        btShape = new btBoxShape(btVector3(boxSize / 2, boxSize / 2, boxSize / 2));
        btObject = new btCollisionObject();
        btObject->setCollisionShape(btShape);
        btObject->setWorldTransform(trans);
        btWord->addCollisionObject(btObject);

        object.SetBoundingBox(DAVA::AABBox3(DAVA::Vector3(), boxSize));
    }
}

CollisionBox::~CollisionBox()
{
    if (btObject != nullptr)
    {
        btWord->removeCollisionObject(btObject);
        DAVA::SafeDelete(btObject);
        DAVA::SafeDelete(btShape);
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionBox::ClassifyToPlane(const DAVA::Plane& plane)
{
    return ClassifyBoundingBoxToPlane(object.GetBoundingBox(), TransformPlaneToLocalSpace(plane));
}

CollisionBaseObject::ClassifyPlanesResult CollisionBox::ClassifyToPlanes(DAVA::Plane* plane, size_t numPlanes)
{
    for (size_t i = 0; i < numPlanes; ++i)
    {
        if (ClassifyToPlane(plane[i]) == CollisionBaseObject::ClassifyPlaneResult::Behind)
        {
            return CollisionBaseObject::ClassifyPlanesResult::Outside;
        }
    }
    return CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects;
}