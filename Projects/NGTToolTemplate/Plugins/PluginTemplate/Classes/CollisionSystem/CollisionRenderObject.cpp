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


#include "CollisionSystem/CollisionRenderObject.h"


CollisionRenderObject::CollisionRenderObject(DAVA::Entity *entity, btCollisionWorld *word, DAVA::RenderObject *renderObject)
	: CollisionBaseObject(entity, word)
	, btTriangles(NULL)
	, btShape(NULL)
{
	if(NULL != renderObject && NULL != word)
	{
		bool anyPolygonAdded = false;
		DAVA::Matrix4 curEntityTransform = entity->GetWorldTransform();

		DAVA::AABBox3 commonBox;

        int maxVertexCount = 0;
        int bestLodIndex = 0;
        int curSwitchIndex = renderObject->GetSwitchIndex();

        // search for best lod index
        for(DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
        {
            int batchLodIndex;
            int batchSwitchIndex;

            DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);
            int vertexCount = batch->GetPolygonGroup()->GetVertexCount();
            if(vertexCount > maxVertexCount && curSwitchIndex == batchSwitchIndex)
            {
                bestLodIndex = batchLodIndex;
                maxVertexCount = vertexCount;
            }
        }

		for(DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
		{
            int batchLodIndex;
            int batchSwitchIndex;

            DAVA::RenderBatch* batch = renderObject->GetRenderBatch(i, batchLodIndex, batchSwitchIndex);
            if(batchLodIndex == bestLodIndex && batchSwitchIndex == curSwitchIndex)
            {
			    DAVA::PolygonGroup* pg = batch->GetPolygonGroup();

			    if(NULL != pg)
			    {
				    // is this the first polygon in cycle
				    if(!anyPolygonAdded)
				    {
					    anyPolygonAdded = true;
					    btTriangles = new btTriangleMesh();
				    }

				    for(int i = 0; i < pg->indexCount; i += 3 )
				    {
					    DAVA::uint16 index0 = pg->indexArray[i];
					    DAVA::uint16 index1 = pg->indexArray[i+1];
					    DAVA::uint16 index2 = pg->indexArray[i+2];

					    DAVA::Vector3 v;
					    pg->GetCoord(index0, v);
					    v = v * curEntityTransform;
					    btVector3 vertex0(v.x, v.y, v.z);

					    pg->GetCoord(index1, v);
					    v = v * curEntityTransform;
					    btVector3 vertex1(v.x, v.y, v.z);

					    pg->GetCoord(index2, v);
					    v = v * curEntityTransform;
					    btVector3 vertex2(v.x, v.y, v.z);

					    btTriangles->addTriangle(vertex0, vertex1, vertex2, false);
				    }

				    // save original bbox
				    boundingBox.AddAABBox(pg->GetBoundingBox());
			    }
            }
		}

		if(anyPolygonAdded)
		{
			// increase bbox a a little bit
			boundingBox.AddPoint(boundingBox.min - DAVA::Vector3(0.5f, 0.5f, 0.5f));
			boundingBox.AddPoint(boundingBox.max + DAVA::Vector3(0.5f, 0.5f, 0.5f));

			btObject = new btCollisionObject();
			btShape = new btBvhTriangleMeshShape(btTriangles, true, true);

			btObject->setCollisionShape(btShape);
			btWord->addCollisionObject(btObject);
		}
	}
}

CollisionRenderObject::~CollisionRenderObject()
{
	if(NULL != btObject)
	{
		btWord->removeCollisionObject(btObject);
		DAVA::SafeDelete(btObject);
		DAVA::SafeDelete(btShape);
		DAVA::SafeDelete(btTriangles);
	}
}
