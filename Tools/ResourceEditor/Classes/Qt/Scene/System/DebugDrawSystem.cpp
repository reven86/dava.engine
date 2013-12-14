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



#include "Scene/System/DebugDrawSystem.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Deprecated/EditorConfig.h"

using namespace DAVA;

float32 DebugDrawSystem::HANGING_OBJECTS_HEIGHT = 0.2f;

DebugDrawSystem::DebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, objectType(ResourceEditor::ESOT_NONE)
    , objectTypeColor(Color::White())
	, hangingObjectsModeEnabled(false)
{
	SceneEditor2 *sc = (SceneEditor2 *)GetScene();

	collSystem = sc->collisionSystem;
	selSystem = sc->selectionSystem;

	DVASSERT(NULL != collSystem);
	DVASSERT(NULL != selSystem);
	
	debugDrawState = DAVA::RenderManager::Instance()->Derive3DRenderState(DAVA::RenderStateData::STATE_BLEND |
																		  DAVA::RenderStateData::STATE_COLORMASK_ALL |
																		  DAVA::RenderStateData::STATE_DEPTH_TEST);
}


DebugDrawSystem::~DebugDrawSystem()
{ }

void DebugDrawSystem::SetRequestedObjectType(ResourceEditor::eSceneObjectType _objectType)
{
	objectType = _objectType;

	if(ResourceEditor::ESOT_NONE != objectType)
	{
        const Vector<Color> & colors = EditorConfig::Instance()->GetColorPropertyValues("CollisionTypeColor");
        if((uint32)objectType < (uint32)colors.size())
        {
            objectTypeColor = colors[objectType];
        }
        else
        {
            objectTypeColor = Color(1.f, 0, 0, 1.f);
        }
	}
}


ResourceEditor::eSceneObjectType DebugDrawSystem::GetRequestedObjectType() const
{
	return objectType;
}

void DebugDrawSystem::Draw()
{
	DAVA::RenderManager::Instance()->SetRenderState(debugDrawState);
	DAVA::RenderManager::Instance()->FlushState();
	
	Draw(GetScene());
}

void DebugDrawSystem::Draw(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		DrawObjectBoxesByType(entity);
		DrawUserNode(entity);
		DrawLightNode(entity);
		DrawSoundNode(entity);
		DrawHangingObjects(entity);
        DrawStaticOcclusionComponent(entity);

		for(int32 i = 0; i < entity->GetChildrenCount(); ++i)
		{
			Draw(entity->GetChild(i));
		}
	}
}

void DebugDrawSystem::DrawObjectBoxesByType(DAVA::Entity *entity)
{
	KeyedArchive * customProperties = entity->GetCustomProperties();
	if(customProperties && customProperties->IsKeyExists("CollisionType") && (customProperties->GetInt32("CollisionType", 0) == objectType))
	{
		DrawEntityBox(entity, objectTypeColor);
	}
}

void DebugDrawSystem::DrawUserNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT))
	{
		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
		Matrix4 finalMatrix = entity->GetWorldTransform() * prevMatrix;

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

		AABBox3 worldBox = selSystem->GetSelectionAABox(entity);
		DAVA::float32 delta = worldBox.GetSize().Length() / 4;

		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.5f, 0.5f, 1.0f, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.2f, 0.2f, 0.8f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);

		// axises
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(delta, 0, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0.7f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, delta, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0, 0.7f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, delta));

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
}


void DebugDrawSystem::DrawStaticOcclusionComponent(DAVA::Entity *entity)
{
    StaticOcclusionComponent * staticOcclusionComponent = 0;
	if((staticOcclusionComponent = (StaticOcclusionComponent*)entity->GetComponent(DAVA::Component::STATIC_OCCLUSION_COMPONENT)) != 0)
	{
		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
		Matrix4 finalMatrix = entity->GetWorldTransform() * prevMatrix;
        
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
        
		AABBox3 worldBox = staticOcclusionComponent->GetBoundingBox();
		DAVA::float32 delta = worldBox.GetSize().Length() / 4;
        
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.9f, 0.5f, 1.0f, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.9f, 0.2f, 0.8f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
        
		// axises
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(delta, 0, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0.7f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, delta, 0));
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0, 0.7f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, delta));
        
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
}

void DebugDrawSystem::DrawLightNode(DAVA::Entity *entity)
{
	DAVA::Light *light = GetLight(entity);
	if(NULL != light)
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		if(light->GetType() == Light::TYPE_DIRECTIONAL)
		{
			DAVA::Vector3 center = worldBox.GetCenter();
			DAVA::Vector3 direction = light->GetDirection();

			direction.Normalize();
			direction = direction * worldBox.GetSize().x;

			center -= (direction / 2);

			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 1.0f));
			DAVA::RenderHelper::Instance()->DrawArrow(center + direction, center, direction.Length() / 2);
		}
		else if(light->GetType() == Light::TYPE_POINT)
		{
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 0.3f));
			DAVA::RenderHelper::Instance()->FillDodecahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2);
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 1.0f));
			DAVA::RenderHelper::Instance()->DrawDodecahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2);
		}
		else
		{
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 0.3f));
			DAVA::RenderHelper::Instance()->FillBox(worldBox);
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 0, 1.0f));
			DAVA::RenderHelper::Instance()->DrawBox(worldBox);
		}
	}
}

void DebugDrawSystem::DrawSoundNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::SOUND_COMPONENT))
	{
		AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0.3f, 0.8f, 0.3f));
		DAVA::RenderHelper::Instance()->FillBox(worldBox);
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0.3f, 0.8f, 1.0f));
		DAVA::RenderHelper::Instance()->DrawBox(worldBox);
	}
}

void DebugDrawSystem::DrawEntityBox( DAVA::Entity *entity, const DAVA::Color &color )
{
	AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

	DAVA::RenderManager::Instance()->SetColor(color);
	DAVA::RenderHelper::Instance()->DrawBox(worldBox);
}

void DebugDrawSystem::EnableHangingObjectsMode( bool enabled )
{
	hangingObjectsModeEnabled = enabled;
}

bool DebugDrawSystem::HangingObjectsModeEnabled() const
{
	return hangingObjectsModeEnabled;
}

void DebugDrawSystem::DrawHangingObjects( DAVA::Entity *entity )
{
	if(!hangingObjectsModeEnabled)
		return;

	if (entity->GetParent() != GetScene())
		return;

	if(IsObjectHanging(entity))
	{
		DrawEntityBox(entity, Color(1.f, 0.f, 0.f, 1.f));
	}
}

bool DebugDrawSystem::IsObjectHanging(Entity * entity)
{
	AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());
	if(worldBox.IsEmpty() && worldBox.min.x == worldBox.max.x && worldBox.min.y == worldBox.max.y && worldBox.min.z == worldBox.max.z) 
		return false;

	float32 xStep = Max((worldBox.max.x - worldBox.min.x) / 10.f, 1.f);
	float32 yStep = Max((worldBox.max.y - worldBox.min.y) / 10.f, 1.f);

	for(float32 y = worldBox.min.y; y <= worldBox.max.y; y += yStep)
	{
		for(float32 x = worldBox.min.x; x <= worldBox.max.x; x += xStep)
		{
			Vector3 landscapePoint = GetLandscapePointAtCoordinates(Vector2(x, y));
			if((worldBox.min.z - landscapePoint.z) > HANGING_OBJECTS_HEIGHT)
			{
				return true;
			}
		}
	}

	return false;
}


Vector3 DebugDrawSystem::GetLandscapePointAtCoordinates(const Vector2 & centerXY)
{
	LandscapeEditorDrawSystem *landSystem = ((SceneEditor2 *)GetScene())->landscapeEditorDrawSystem;
	LandscapeProxy* landscape = landSystem->GetLandscapeProxy();

	if(landscape)
	{
		return landscape->PlacePoint(Vector3(centerXY));
	}

	return Vector3();
}
