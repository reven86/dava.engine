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


#include "CollisionSystem/CollisionLandscape.h"
#include "Render/Highlevel/Heightmap.h"

CollisionLandscape::CollisionLandscape(DAVA::Entity *entity, btCollisionWorld *word, DAVA::Landscape *landscape)
	: CollisionBaseObject(entity, word)
{
	if(NULL != landscape && NULL != word)
	{
		DAVA::Heightmap *heightmap = landscape->GetHeightmap();
		if(NULL != heightmap && heightmap->Size() > 0)
		{
			DAVA::Vector3 landSize;
			DAVA::AABBox3 landBox = landscape->GetBoundingBox();

			//DAVA::AABBox3 landTransformedBox;
			//DAVA::Matrix4 landWorldTransform = entity->GetWorldTransform();
			// 
			//landBox.GetTransformedBox(landWorldTransform, landTransformedBox);
			//landSize = landTransformedBox.max - landTransformedBox.min;

			landSize = landBox.max - landBox.min;

			DAVA::float32 landWidth = landSize.x;
			DAVA::float32 landScaleW = landWidth / heightmap->Size();
			DAVA::float32 landHeight = landSize.z;
			DAVA::float32 landScaleH = landHeight / 65535.f;

			DAVA::uint16 *heightData = heightmap->Data();
			btHMap.resize(heightmap->Size() * heightmap->Size());

			for(DAVA::int32 y = 0; y < heightmap->Size(); ++y)
			{
				for (DAVA::int32 x = 0; x < heightmap->Size(); ++x)
				{
					DAVA::int32 heightIndex = x + y * heightmap->Size();
					btHMap[heightIndex] = heightData[heightIndex] * landScaleH;
				}
			}

			btTerrain = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size(),	&btHMap.front(), landScaleH, 0, landHeight, 2, PHY_FLOAT, true);
			btTerrain->setLocalScaling(btVector3(landScaleW, landScaleW, 1.0f));
			
			btTransform landTransform;
			landTransform.setIdentity();
			landTransform.setOrigin(btVector3(0, 0, landHeight / 2.0f));

			btObject = new btCollisionObject();
			btObject->setWorldTransform(landTransform);
			btObject->setCollisionShape(btTerrain);
			btWord->addCollisionObject(btObject);

			boundingBox = landBox;
		}
	}
}

CollisionLandscape::~CollisionLandscape()
{
	if(NULL != btObject)
	{
		btWord->removeCollisionObject(btObject);
		DAVA::SafeDelete(btObject);
		DAVA::SafeDelete(btTerrain);
	}
}
