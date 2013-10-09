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



#include "HeightmapNode.h"
#include "../bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "../EditorScene.h"
#include "Render/Highlevel/Heightmap.h"

#include "Render/Highlevel/LandscapeCursor.h"

HeightmapNode::HeightmapNode(EditorScene * _scene, Landscape *_land)
    :   Entity()
    ,   position(0,0,0)
    ,   rotation(0)
{
    SetName("HeightmapNode");
    
    editorScene = _scene;
    SetVisible(false);

    land = _land;
    DVASSERT(land);
    
    //Get LandSize;
    Vector3 landSize;
    AABBox3 transformedBox;
    Matrix4 * worldTransformPtr = land->GetWorldTransformPtr();
    
    land->GetBoundingBox().GetTransformedBox(*worldTransformPtr, transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    heightmap = land->GetHeightmap();
    sizeInMeters = landSize.x;
    areaScale = sizeInMeters / heightmap->Size();
    maxHeight = landSize.z;
    heightScale = maxHeight / 65535.f;
    float32 minHeight = 0.0f;
    
    
    uint16 *dt = heightmap->Data();
    size.x = (float32)heightmap->Size();
    size.y = (float32)heightmap->Size();
    hmap.resize(heightmap->Size() * heightmap->Size());

	for (int32 y = 0; y < heightmap->Size(); ++y)
	{
		for (int32 x = 0; x < heightmap->Size(); ++x)
		{
            int32 index = x + (y) * heightmap->Size();
            float32 mapValue = dt[index] * heightScale;
            SetValueToMap(x, y, mapValue, transformedBox);
		}
	}

	bool flipQuadEdges = true;

    colShape = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size()
                                             , &hmap.front(), heightScale, minHeight, maxHeight
                                             , 2, PHY_FLOAT
                                             , flipQuadEdges);

    colShape->setLocalScaling(btVector3(areaScale, areaScale, 1.0f));
    
    // Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
    btScalar	mass(0.0f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0,0,0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass,localInertia);
    
    startTransform.setOrigin(btVector3( btScalar(position.x),
                                       btScalar(position.y),
                                       btScalar(maxHeight/2.0f + position.z)));
    
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    motionSate = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionSate,colShape,localInertia);
    rbInfo.m_friction = 0.9f;
    body = new btRigidBody(rbInfo);
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
    collisionObject->setCollisionShape(colShape);
    editorScene->landCollisionWorld->addCollisionObject(collisionObject);
	editorScene->landCollisionWorld->updateAabbs();
}

HeightmapNode::~HeightmapNode()
{
	if(editorScene->landCollisionWorld)
	{
        editorScene->landCollisionWorld->removeCollisionObject(collisionObject);
	}
    
    
    SafeDelete(body);
    
    SafeDelete(collisionObject);
    
    SafeDelete(motionSate);
    
    SafeDelete(colShape);
}


void HeightmapNode::SetValueToMap(int16 x, int16 y, float32 height, const AABBox3 &box)
{
    int32 index = x + y * heightmap->Size();

    hmap[index] = height;
}

void HeightmapNode::UpdateHeightmapRect(const Rect &rect)
{
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(*land->GetWorldTransformPtr(), transformedBox);
    
    int32 x = (int32)rect.x;
    int32 y = (int32)rect.y;
    int32 endX = (int32)(rect.x + rect.dx);
    int32 endY = (int32)(rect.y + rect.dy);

    uint16 *dt = heightmap->Data();
    for (int32 yy = y; yy < endY; ++yy)
    {
		for (int32 xx = x; xx < endX; ++xx)
		{
            float32 mapValue = dt[(xx + (yy) * heightmap->Size())] * heightScale;
            SetValueToMap(xx, yy, mapValue, transformedBox);
		}
	}
}

float32 HeightmapNode::GetAreaScale()
{
    return areaScale;
}

const Vector3 &HeightmapNode::GetSize()
{
    return size;
}

float32 HeightmapNode::GetSizeInMeters()
{
    return sizeInMeters;
}

