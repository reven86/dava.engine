/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Scene/System/CollisionSystem/CollisionRenderObject.h"


CollisionRenderObject::CollisionRenderObject(DAVA::Entity *entity, btCollisionWorld *word, DAVA::RenderObject *renderObject)
	: CollisionBaseObject(entity, word)
{
    if ((renderObject != nullptr) && (word != nullptr))
    {
        DAVA::Matrix4 curEntityTransform = entity->GetWorldTransform();

        int maxVertexCount = 0;
        int bestLodIndex = 0;
        int curSwitchIndex = renderObject->GetSwitchIndex();

        // search for best lod index
        for (DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
        {
            int batchLodIndex = 0;
            int batchSwitchIndex = 0;
            DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);
            int vertexCount = batch->GetPolygonGroup()->GetVertexCount();
            if ((vertexCount > maxVertexCount) && (curSwitchIndex == batchSwitchIndex))
            {
                bestLodIndex = batchLodIndex;
                maxVertexCount = vertexCount;
            }
        }

        bool anyPolygonAdded = false;
        for (DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
        {
            int batchLodIndex = 0;
            int batchSwitchIndex = 0;
            DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);

            if ((batchLodIndex == bestLodIndex) && (batchSwitchIndex == curSwitchIndex))
            {
			    DAVA::PolygonGroup* pg = batch->GetPolygonGroup();
                if (pg != nullptr)
                {
                    // is this the first polygon in cycle
                    if (!anyPolygonAdded)
                    {
                        anyPolygonAdded = true;
                        btTriangles = new btTriangleMesh();
                    }

                    for (int i = 0; i < pg->indexCount; i += 3)
                    {
                        DAVA::uint16 index0 = pg->indexArray[i];
                        DAVA::uint16 index1 = pg->indexArray[i + 1];
                        DAVA::uint16 index2 = pg->indexArray[i + 2];

                        DAVA::Vector3 v0;
                        DAVA::Vector3 v1;
                        DAVA::Vector3 v2;
                        pg->GetCoord(index0, v0);
                        pg->GetCoord(index1, v1);
                        pg->GetCoord(index2, v2);

                        v0 = v0 * curEntityTransform;
                        v1 = v1 * curEntityTransform;
                        v2 = v2 * curEntityTransform;

                        btTriangles->addTriangle(btVector3(v0.x, v0.y, v0.z),
                                                 btVector3(v1.x, v1.y, v1.z),
                                                 btVector3(v2.x, v2.y, v2.z), false);
                    }

                    // save original bbox
                    boundingBox.AddAABBox(pg->GetBoundingBox());
                }
            }
		}

        if (anyPolygonAdded)
        {
            // increase bbox a little bit
            boundingBox.AddPoint(boundingBox.min - DAVA::Vector3(0.5f, 0.5f, 0.5f));
            boundingBox.AddPoint(boundingBox.max + DAVA::Vector3(0.5f, 0.5f, 0.5f));

            btShape = new btBvhTriangleMeshShape(btTriangles, true, true);
            btObject = new btCollisionObject();
            btObject->setCollisionShape(btShape);
            btWord->addCollisionObject(btObject);
        }
    }
}

CollisionRenderObject::~CollisionRenderObject()
{
    if (btObject != nullptr)
    {
        btWord->removeCollisionObject(btObject);
        DAVA::SafeDelete(btObject);
        DAVA::SafeDelete(btShape);
        DAVA::SafeDelete(btTriangles);
    }
}

struct ClassifyTrianglesToSinglePlaneCallback : public btInternalTriangleIndexCallback
{
    DAVA::Plane plane;
    CollisionBaseObject::ClassifyPlaneResult result = CollisionBaseObject::ClassifyPlaneResult::Behind;

    ClassifyTrianglesToSinglePlaneCallback(const DAVA::Plane& pl)
        : plane(pl)
    {
    }

    void internalProcessTriangleIndex(btVector3* triangle, int partId, int triangleIndex) override
    {
        if (result == CollisionBaseObject::ClassifyPlaneResult::Behind)
        {
            float d0 = plane.DistanceToPoint(DAVA::Vector3(triangle[0].x(), triangle[0].y(), triangle[0].z()));
            float d1 = plane.DistanceToPoint(DAVA::Vector3(triangle[1].x(), triangle[1].y(), triangle[1].z()));
            float d2 = plane.DistanceToPoint(DAVA::Vector3(triangle[2].x(), triangle[2].y(), triangle[2].z()));
            DAVA::float32 minDistance = std::min(d0, std::min(d1, d2));
            DAVA::float32 maxDistance = std::max(d0, std::max(d1, d2));
            if ((minDistance >= 0.0f) && (maxDistance >= 0.0f))
            {
                result = CollisionBaseObject::ClassifyPlaneResult::InFront;
            }
            else if (((minDistance < 0.0f) && (maxDistance >= 0.0f)) || ((minDistance >= 0.0f) && (maxDistance < 0.0f)))
            {
                result = CollisionBaseObject::ClassifyPlaneResult::Intersects;
            }
        }
    }
};

inline int CROLocal_FloatsIsNegative(float& v1, float& v2)
{
    return ((reinterpret_cast<uint32_t&>(v1) & 0x80000000) & (reinterpret_cast<uint32_t&>(v2) & 0x80000000)) >> 31;
}

inline const DAVA::Vector3& CROLocal_btVectorToDava(const btVector3* v)
{
    return *(reinterpret_cast<const DAVA::Vector3*>(v));
}

inline void CROLocal_Sort(float values[3])
{
    if (values[1] > values[0])
        std::swap(values[1], values[0]);
    if (values[2] > values[1])
        std::swap(values[2], values[1]);
    if (values[1] > values[0])
        std::swap(values[1], values[0]);
}

struct ClassifyTrianglesToMultiplePlanesCallback : public btInternalTriangleIndexCallback
{
    DAVA::Plane* planes = nullptr;
    size_t numPlanes = 0;
    int numTriangles = 0;
    int trianglesBehind = 0;

    ClassifyTrianglesToMultiplePlanesCallback(DAVA::Plane* pl, size_t np, int nt)
        : planes(pl)
        , numPlanes(np)
        , numTriangles(nt)
    {
    }

    void internalProcessTriangleIndex(btVector3* triangle, int partId, int triangleIndex) override
    {
        for (size_t i = 0; i < numPlanes; ++i)
        {
            float distances[3] =
            {
              planes[i].DistanceToPoint(CROLocal_btVectorToDava(triangle)),
              planes[i].DistanceToPoint(CROLocal_btVectorToDava(triangle + 1)),
              planes[i].DistanceToPoint(CROLocal_btVectorToDava(triangle + 2))
            };
            CROLocal_Sort(distances);
            if (CROLocal_FloatsIsNegative(distances[0], distances[2]))
            {
                ++trianglesBehind;
                break;
            }
        }
    }
};

CollisionBaseObject::ClassifyPlaneResult CollisionRenderObject::ClassifyToPlane(const DAVA::Plane& plane)
{
    if ((btShape == nullptr) || (ClassifyBoundingBoxToPlane(boundingBox, plane) == ClassifyPlaneResult::Behind))
        return ClassifyPlaneResult::Behind;

    btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(btShape);

    ClassifyTrianglesToSinglePlaneCallback cb(plane);
    btTriangles->InternalProcessAllTriangles(&cb, shape->getLocalAabbMin(), shape->getLocalAabbMax());
    return cb.result;
}

CollisionBaseObject::ClassifyPlanesResult CollisionRenderObject::ClassifyToPlanes(DAVA::Plane* planes, size_t numPlanes)
{
    if (btShape == nullptr)
        return CollisionBaseObject::ClassifyPlanesResult::Outside;

    for (size_t i = 0; i < numPlanes; ++i)
    {
        if (ClassifyBoundingBoxToPlane(boundingBox, TransformPlaneToLocalSpace(planes[i])) == ClassifyPlaneResult::Behind)
        {
            return CollisionBaseObject::ClassifyPlanesResult::Outside;
        }
    }

    btBvhTriangleMeshShape* shape = static_cast<btBvhTriangleMeshShape*>(btShape);
    ClassifyTrianglesToMultiplePlanesCallback cb(planes, numPlanes, btTriangles->getNumTriangles());
    btTriangles->InternalProcessAllTriangles(&cb, shape->getLocalAabbMin(), shape->getLocalAabbMax());
    return (cb.trianglesBehind == btTriangles->getNumTriangles()) ? CollisionBaseObject::ClassifyPlanesResult::Outside :
                                                                    CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects;
}