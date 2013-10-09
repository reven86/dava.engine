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



#include "HangingObjectsHelper.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "LandscapeEditor/LandscapesController.h"

#define HEIGHT_DENSITY 2.0f

namespace DAVA
{

Vector3 HangingObjectsHelper::GetLandscapePointAtCoordinates(const Vector2& centerXY, SceneData *sceneData)
{
	Landscape* workingLandscape = sceneData->GetLandscapesController()->GetCurrentLandscape();

	AABBox3 boundingBox = workingLandscape->GetBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,(boundingBox.max - boundingBox.min).y);
	
	Rect landRect(landPos, landSize);

	Texture * tex = workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);
	Vector2 landscapeSize(tex->GetWidth(),tex->GetHeight());
	Rect textRect(Vector2(0, 0), landscapeSize);

	Vector3 landscapePoint(centerXY);
	
	workingLandscape->PlacePoint(landscapePoint, landscapePoint);

	return landscapePoint;
}

void HangingObjectsHelper::GetLowerPoint(const Vector2& candidate, Vector3& currentLower, SceneData *sceneData)
{
	Vector3 landscapePoint = GetLandscapePointAtCoordinates(candidate, sceneData);
	currentLower = currentLower.z > landscapePoint.z ? landscapePoint : currentLower;
}

Vector3 HangingObjectsHelper::GetLowestPointFromRect(Rect& rect, float density, SceneData *sceneData)
{
	Vector3 testPoint = GetLandscapePointAtCoordinates(Vector2( rect.x, rect.y), sceneData);

	for(float i = rect.x; i < rect.dx; i+=density)
	{
		for(float j = rect.y; j < rect.dy; j+=density)
		{
			GetLowerPoint(Vector2(i,j), testPoint, sceneData);
		}
	}

	// loops to test extremes sides (__|)
	for(float i = rect.dx; i > rect.x; i-=density)
	{
		GetLowerPoint(Vector2(i, rect.dy), testPoint, sceneData);
	}

	for(float j = rect.dy; j > rect.y; j-=density)
	{
		GetLowerPoint(Vector2(rect.dx, j), testPoint, sceneData);
	}

	return testPoint;
}

void HangingObjectsHelper::ProcessHangingObjectsUpdate(float value, bool isEnabled)
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		
		List<Entity*> renderComponents;
		
		sceneData->GetScene()->FindComponentsByTypeRecursive(Component::RENDER_COMPONENT, renderComponents);
		
		for(List<Entity*>::const_iterator it = renderComponents.begin(); it != renderComponents.end(); ++it)
		{
			Entity* entity = (*it);

			RenderComponent * renderComponent = cast_if_equal<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
			if(NULL == renderComponent)
			{
				continue;
			}

			RenderObject* renderObject = renderComponent->GetRenderObject();
	
			if(RenderObject::TYPE_MESH != renderObject->GetType())
			{
				continue;
			}

			entity->SetDebugFlags(entity->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_RED_AABBOX));

			if(!isEnabled)
			{
				continue;
			}

			AABBox3 bBox = renderObject->GetWorldBoundingBox();

			Rect buttomSide(Vector2(bBox.min.x, bBox.min.y), Vector2(bBox.max.x, bBox.max.y));

			Vector3 landscapePoint = GetLowestPointFromRect(buttomSide, HEIGHT_DENSITY, sceneData);

			if((bBox.min.z - landscapePoint.z) > value)
			{
				entity->SetDebugFlags(entity->GetDebugFlags() | (DebugRenderComponent::DEBUG_DRAW_RED_AABBOX));
			}
		}
	}
}



};
