#include "Qt/Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Render/Highlevel/Heightmap.h"

#include "Base/AlignedAllocator.h"

CollisionLandscape::CollisionLandscape(DAVA::Entity* entity, btCollisionWorld* word, DAVA::Landscape* landscape)
    : CollisionBaseObject(entity, word)
{
    if ((landscape != nullptr) && (word != nullptr))
    {
        DAVA::Heightmap* heightmap = landscape->GetHeightmap();
        if ((heightmap != nullptr) && (heightmap->Size() > 0))
        {
            DAVA::Vector3 landSize;
            DAVA::AABBox3 landBox = landscape->GetBoundingBox();
            landSize = landBox.max - landBox.min;

            DAVA::float32 landWidth = landSize.x;
            DAVA::float32 landScaleW = landWidth / heightmap->Size();
            DAVA::float32 landHeight = landSize.z;
            DAVA::float32 landScaleH = landHeight / 65535.f;

            btHMap.resize(heightmap->Size() * heightmap->Size());

            for (DAVA::int32 y = 0; y < heightmap->Size(); ++y)
            {
                for (DAVA::int32 x = 0; x < heightmap->Size(); ++x)
                {
                    DAVA::int32 heightIndex = x + y * heightmap->Size();
                    btHMap[heightIndex] = heightmap->GetHeight(x, y) * landScaleH;
                }
            }

            btTransform landTransform;
            landTransform.setIdentity();
            landTransform.setOrigin(btVector3(0, 0, landHeight / 2.0f));

            btTerrain = DAVA::CreateObjectAligned<btHeightfieldTerrainShape, 16>(heightmap->Size(), heightmap->Size(), &btHMap.front(), landScaleH, 0.0f, landHeight, 2, PHY_FLOAT, true);
            btTerrain->setLocalScaling(btVector3(landScaleW, landScaleW, 1.0f));
            btObject = new btCollisionObject();
            btObject->setWorldTransform(landTransform);
            btObject->setCollisionShape(btTerrain);
            btWord->addCollisionObject(btObject);

            object.SetBoundingBox(landBox);
        }
    }
}

CollisionLandscape::~CollisionLandscape()
{
    if (NULL != btObject)
    {
        btWord->removeCollisionObject(btObject);
        DAVA::SafeDelete(btObject);
        DAVA::DestroyObjectAligned(btTerrain);
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionLandscape::ClassifyToPlane(const DAVA::Plane& plane)
{
    return ClassifyPlaneResult::Behind;
}

CollisionBaseObject::ClassifyPlanesResult CollisionLandscape::ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes)
{
    return ClassifyPlanesResult::Outside;
}