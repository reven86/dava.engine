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
#include "Scene3D/Components/ComponentHelpers.h"

#include "Deprecated/SceneValidator.h"

using namespace DAVA;

float32 DebugDrawSystem::HANGING_OBJECTS_HEIGHT = 0.001f;

DebugDrawSystem::DebugDrawSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, objectType(ResourceEditor::ESOT_NONE)
    , objectTypeColor(Color::White)
	, hangingObjectsModeEnabled(false)
    , switchesWithDifferentLodsEnabled(false)
{
	SceneEditor2 *sc = (SceneEditor2 *)GetScene();

	collSystem = sc->collisionSystem;
	selSystem = sc->selectionSystem;

	DVASSERT(NULL != collSystem);
    DVASSERT(NULL != selSystem);
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
	
	Draw(GetScene());
}

void DebugDrawSystem::Draw(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
        bool isSelected = selSystem->GetSelection().ContainsEntity(entity);

		DrawObjectBoxesByType(entity);
		DrawUserNode(entity);
		DrawLightNode(entity);
		DrawHangingObjects(entity);        
		DrawSwitchesWithDifferentLods(entity);
		DrawWindNode(entity);
        DrawSwitchesWithDifferentLods(entity);
        DrawSoundNode(entity);

        if(isSelected)
        {
            DrawSelectedSoundNode(entity);         
        }

		for(int32 i = 0; i < entity->GetChildrenCount(); ++i)
		{
			Draw(entity->GetChild(i));
		}
	}
}

void DebugDrawSystem::DrawObjectBoxesByType(DAVA::Entity *entity)
{
	bool drawBox = false;

	KeyedArchive * customProperties = GetCustomPropertiesArchieve(entity);
    if ( customProperties )
    {
        if ( customProperties->IsKeyExists( "CollisionType" ) )
        {
            drawBox = customProperties->GetInt32( "CollisionType", 0 ) == objectType;
        }
        else if ( objectType == ResourceEditor::ESOT_UNDEFINED_COLLISION && entity->GetParent() == GetScene() )
        {
            const bool skip =
            GetLight(entity) == NULL &&
            GetCamera(entity) == NULL &&
            GetLandscape(entity) == NULL;

            drawBox = skip;
        }
    }

    if ( drawBox )
	{
		DrawEntityBox(entity, objectTypeColor);
	}
}

void DebugDrawSystem::DrawUserNode(DAVA::Entity *entity)
{
	if(NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT))
	{
        RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox = selSystem->GetSelectionAABox(entity);
        DAVA::float32 delta = worldBox.GetSize().Length() / 4;

        drawer->DrawAABoxTransformed(worldBox, entity->GetWorldTransform(), DAVA::Color(0.5f, 0.5f, 1.0f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawAABoxTransformed(worldBox, entity->GetWorldTransform(), DAVA::Color(0.2f, 0.2f, 0.8f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);

        const Vector3 center = entity->GetWorldTransform().GetTranslationVector();
        const Vector3 xAxis = MultiplyVectorMat3x3(DAVA::Vector3(delta, 0.f, 0.f), entity->GetWorldTransform());
        const Vector3 yAxis = MultiplyVectorMat3x3(DAVA::Vector3(0.f, delta, 0.f), entity->GetWorldTransform());
        const Vector3 zAxis = MultiplyVectorMat3x3(DAVA::Vector3(0.f, 0.f, delta), entity->GetWorldTransform());

        // axises
        drawer->DrawLine(center, center + xAxis, DAVA::Color(0.7f, 0, 0, 1.0f));
        drawer->DrawLine(center, center + yAxis, DAVA::Color(0, 0.7f, 0, 1.0f));
        drawer->DrawLine(center, center + zAxis, DAVA::Color(0, 0, 0.7f, 1.0f));
    }
}

void DebugDrawSystem::DrawLightNode(DAVA::Entity *entity)
{
	DAVA::Light *light = GetLight(entity);
	if(NULL != light)
	{
        RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

		if(light->GetType() == Light::TYPE_DIRECTIONAL)
		{
			DAVA::Vector3 center = worldBox.GetCenter();
			DAVA::Vector3 direction = -light->GetDirection();

			direction.Normalize();
			direction = direction * worldBox.GetSize().x;

			center -= (direction / 2);

            drawer->DrawArrow(center + direction, center, direction.Length() / 2, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else if (light->GetType() == Light::TYPE_POINT)
        {
            drawer->DrawIcosahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2, DAVA::Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawIcosahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else
        {
            drawer->DrawAABox(worldBox, DAVA::Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawAABox(worldBox, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSoundNode(DAVA::Entity *entity)
{
    SettingsManager * settings = SettingsManager::Instance();

    if(!settings->GetValue(Settings::Scene_Sound_SoundObjectDraw).AsBool())
        return;

    DAVA::SoundComponent * sc = GetSoundComponent(entity);
    if(sc)
    {
        AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, settings->GetValue(Settings::Scene_Sound_SoundObjectBoxColor).AsColor(), RenderHelper::DRAW_SOLID_DEPTH);
    }
}

void DebugDrawSystem::DrawSelectedSoundNode(DAVA::Entity *entity)
{
    SettingsManager * settings = SettingsManager::Instance();

    if(!settings->GetValue(Settings::Scene_Sound_SoundObjectDraw).AsBool())
        return;

    DAVA::SoundComponent * sc = GetSoundComponent(entity);
    if(sc)
    {
        SceneEditor2 * sceneEditor = ((SceneEditor2 *)GetScene());

        Vector3 position = entity->GetWorldTransform().GetTranslationVector();

        uint32 fontHeight = 0;
        GraphicFont* debugTextFont = sceneEditor->textDrawSystem->GetFont();
        if(debugTextFont)
            fontHeight = debugTextFont->GetFontHeight();

        uint32 eventsCount = sc->GetEventsCount();
        for(uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent * sEvent = sc->GetSoundEvent(i);
            float32 distance = sEvent->GetMaxDistance();

            sceneEditor->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(position, distance,
                                                                              settings->GetValue(Settings::Scene_Sound_SoundObjectSphereColor).AsColor(), RenderHelper::DRAW_SOLID_DEPTH);

            sceneEditor->textDrawSystem->DrawText(sceneEditor->textDrawSystem->ToPos2d(position) - Vector2(0.f, fontHeight - 2.f) * i,
                                                  sEvent->GetEventName(), Color::White, TextDrawSystem::Align::Center);

            if(sEvent->IsDirectional())
            {
                sceneEditor->GetRenderSystem()->GetDebugDrawer()->DrawArrow(position, sc->GetLocalDirection(i), .25f, DAVA::Color(0.0f, 1.0f, 0.3f, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void DebugDrawSystem::DrawWindNode(DAVA::Entity *entity)
{
	WindComponent * wind = GetWindComponent(entity);
	if(wind)
	{
		const Matrix4 & worldMx = entity->GetWorldTransform();
		Vector3 worldPosition = worldMx.GetTranslationVector();

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(worldPosition, worldPosition + wind->GetDirection() * 3.f, .75f, DAVA::Color(1.0f, 0.5f, 0.2f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawEntityBox( DAVA::Entity *entity, const DAVA::Color &color )
{
    AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, color, RenderHelper::DRAW_WIRE_DEPTH);
}


void DebugDrawSystem::DrawHangingObjects( DAVA::Entity *entity )
{
	if(!hangingObjectsModeEnabled)
	{
        return;
    }

    if (entity->GetParent() != GetScene())
    {
		return;
    }
    
	if(IsObjectHanging(entity))
	{
		DrawEntityBox(entity, Color(1.f, 0.f, 0.f, 1.f));
	}
}


bool DebugDrawSystem::IsObjectHanging(Entity * entity)
{
	RenderObject *ro = GetRenderObject(entity);
	if(!ro || (ro->GetType() != RenderObject::TYPE_MESH && ro->GetType() != RenderObject::TYPE_RENDEROBJECT && ro->GetType() != RenderObject::TYPE_SPEED_TREE))
		return false;

	const AABBox3 & worldBox = ro->GetWorldBoundingBox();
	if(worldBox.IsEmpty() && worldBox.min.x == worldBox.max.x && worldBox.min.y == worldBox.max.y && worldBox.min.z == worldBox.max.z)
		return false;

	const Matrix4 & wt = entity->GetWorldTransform();
	Vector3 position, scale, orientation;
	wt.Decomposition(position, scale, orientation);

	Vector<Vector3> lowestVertexes;
	GetLowestVertexes(ro, lowestVertexes, scale);

	const uint32 count = lowestVertexes.size();
	if(count == 0) return false; // we can be in state when selected lod had been set less than lodLayerNumber

	bool isAllVertextesUnderLandscape = true;
	for(uint32 i = 0; i < count && isAllVertextesUnderLandscape; ++i)
	{
		Vector3 pos = lowestVertexes[i];
		pos = pos * wt;

		const Vector3 landscapePoint = GetLandscapePointAtCoordinates(Vector2(pos.x, pos.y));

		bool isVertexUnderLandscape = ((pos.z - landscapePoint.z) < DAVA::EPSILON);
		isAllVertextesUnderLandscape &= isVertexUnderLandscape;
	}

	return !isAllVertextesUnderLandscape;
}

void DebugDrawSystem::GetLowestVertexes(const DAVA::RenderObject *ro, DAVA::Vector<DAVA::Vector3> &vertexes, const DAVA::Vector3 & scale)
{
	const float32 minZ = GetMinimalZ(ro);
	const float32 vertexDelta = HANGING_OBJECTS_HEIGHT / scale.z;

	uint32 count = ro->GetActiveRenderBatchCount();
	for(uint32 i = 0; i < count; ++i)
	{
		RenderBatch *batch = ro->GetActiveRenderBatch(i);
		DVASSERT(batch);

		PolygonGroup *pg = batch->GetPolygonGroup();
		if(pg)
		{
			uint32 vertexCount = pg->GetVertexCount();
			for(uint32 v = 0; v < vertexCount; ++v)
			{
				Vector3 pos;
				pg->GetCoord(v, pos);

				if((pos.z - minZ) <= vertexDelta)   //accuracy of finding of lowest vertexes
				{
					vertexes.push_back(pos);
				}
			}
		}
	}
}

float32 DebugDrawSystem::GetMinimalZ(const DAVA::RenderObject *ro)
{
	float32 minZ = AABBOX_INFINITY;
	
	uint32 count = ro->GetActiveRenderBatchCount();
	for(uint32 i = 0; i < count; ++i)
	{
		RenderBatch *batch = ro->GetActiveRenderBatch(i);
		DVASSERT(batch);

		PolygonGroup *pg = batch->GetPolygonGroup();
		if(pg)
		{
			uint32 vertexCount = pg->GetVertexCount();
			for(uint32 v = 0; v < vertexCount; ++v)
			{
				Vector3 pos;
				pg->GetCoord(v, pos);

				if(pos.z < minZ)
				{
					minZ = pos.z;
				}
			}
		}
	}

	return minZ;
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

void DebugDrawSystem::DrawSwitchesWithDifferentLods( DAVA::Entity *entity )
{
    if(!switchesWithDifferentLodsEnabled) return;

    if(SceneValidator::IsEntityHasDifferentLODsCount(entity))
    {
        AABBox3 worldBox = selSystem->GetSelectionAABox(entity, entity->GetWorldTransform());

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, Color(1.0f, 0.f, 0.f, 1.f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}
