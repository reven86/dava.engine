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



#include "../SceneEditor2.h"
#include "LandscapeEditorDrawSystem.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "LandscapeEditorDrawSystem/NotPassableTerrainProxy.h"
#include "LandscapeEditorDrawSystem/RulerToolProxy.h"
#include "LandscapeEditor/LandscapeRenderer.h"

LandscapeEditorDrawSystem::LandscapeEditorDrawSystem(Scene* scene)
:	SceneSystem(scene)
,	customDrawRequestCount(0)
,	landscapeProxy(NULL)
,	heightmapProxy(NULL)
,	landscapeNode(NULL)
,	baseLandscape(NULL)
,	cursorTexture(NULL)
,	notPassableTerrainProxy(NULL)
,	customColorsProxy(NULL)
,	visibilityToolProxy(NULL)
,	rulerToolProxy(NULL)
{
}

LandscapeEditorDrawSystem::~LandscapeEditorDrawSystem()
{
	SafeRelease(baseLandscape);
	SafeRelease(landscapeProxy);
	SafeRelease(heightmapProxy);
	SafeRelease(customColorsProxy);
	SafeRelease(visibilityToolProxy);
	SafeRelease(rulerToolProxy);
}

LandscapeProxy* LandscapeEditorDrawSystem::GetLandscapeProxy()
{
	return landscapeProxy;
}

HeightmapProxy* LandscapeEditorDrawSystem::GetHeightmapProxy()
{
	return heightmapProxy;
}

CustomColorsProxy* LandscapeEditorDrawSystem::GetCustomColorsProxy()
{
	return customColorsProxy;
}

VisibilityToolProxy* LandscapeEditorDrawSystem::GetVisibilityToolProxy()
{
	return visibilityToolProxy;
}

RulerToolProxy* LandscapeEditorDrawSystem::GetRulerToolProxy()
{
	return rulerToolProxy;
}

bool LandscapeEditorDrawSystem::EnableCustomDraw()
{
	if (customDrawRequestCount != 0)
	{
		++customDrawRequestCount;
		return true;
	}

	if (!Init())
	{
		return false;
	}

	landscapeProxy->SetMode(LandscapeProxy::MODE_CUSTOM_LANDSCAPE);
	landscapeProxy->SetHeightmap(heightmapProxy);

	AABBox3 landscapeBoundingBox = baseLandscape->GetBoundingBox();
	LandscapeRenderer* landscapeRenderer = new LandscapeRenderer(heightmapProxy, landscapeBoundingBox);
	landscapeProxy->SetRenderer(landscapeRenderer);
	
	landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	landscapeNode->AddComponent(new RenderComponent(landscapeProxy->GetRenderObject()));
	
	++customDrawRequestCount;

	return true;
}

void LandscapeEditorDrawSystem::DisableCustomDraw()
{
	if (customDrawRequestCount == 0)
	{
		return;
	}
	
	--customDrawRequestCount;
	
	if (customDrawRequestCount == 0)
	{
		landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
		landscapeNode->AddComponent(new RenderComponent(baseLandscape));
		
		UpdateBaseLandscapeHeightmap();
	}
}

bool LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled()
{
	if (!notPassableTerrainProxy)
	{
		return false;
	}
	
	return notPassableTerrainProxy->IsEnabled();
}

bool LandscapeEditorDrawSystem::IsNotPassableTerrainCanBeEnabled()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);

	bool canBeEnabled = true;
	canBeEnabled &= !(scene->visibilityToolSystem->IsLandscapeEditingEnabled());
//	canBeEnabled &= !(scene->heightmapEditorSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->tilemaskEditorSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->rulerToolSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->customColorsSystem->IsLandscapeEditingEnabled());
//	canBeEnabled &= !(scene->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled());
	
	return canBeEnabled;
}

bool LandscapeEditorDrawSystem::EnableNotPassableTerrain()
{
	if (!notPassableTerrainProxy)
	{
		notPassableTerrainProxy = new NotPassableTerrainProxy();
	}
	
	if (notPassableTerrainProxy->IsEnabled())
	{
		return true;
	}

	if (!IsNotPassableTerrainCanBeEnabled())
	{
		return false;
	}

	if (!EnableCustomDraw())
	{
		return false;
	}

	notPassableTerrainProxy->Enable();
	notPassableTerrainProxy->UpdateTexture(heightmapProxy,
										   landscapeProxy->GetLandscapeBoundingBox(),
										   Rect(0.f, 0.f, (float32)(heightmapProxy->Size() - 1), (float32)(heightmapProxy->Size() - 1)));
	
	landscapeProxy->SetNotPassableTexture(notPassableTerrainProxy->GetTexture());
	landscapeProxy->SetNotPassableTextureEnabled(true);

	return true;
}

void LandscapeEditorDrawSystem::DisableNotPassableTerrain()
{
	if (!notPassableTerrainProxy || !notPassableTerrainProxy->IsEnabled())
	{
		return;
	}
	
	notPassableTerrainProxy->Disable();
	landscapeProxy->SetNotPassableTexture(NULL);
	landscapeProxy->SetNotPassableTextureEnabled(false);
	
	DisableCustomDraw();
}

void LandscapeEditorDrawSystem::EnableCursor(int32 landscapeSize)
{
	landscapeProxy->CursorEnable();
	landscapeProxy->SetBigTextureSize((float32)landscapeSize);
}

void LandscapeEditorDrawSystem::DisableCursor()
{
	landscapeProxy->CursorDisable();
}

void LandscapeEditorDrawSystem::SetCursorTexture(Texture* cursorTexture)
{
	SafeRelease(this->cursorTexture);
	this->cursorTexture = SafeRetain(cursorTexture);
	
	landscapeProxy->SetCursorTexture(cursorTexture);
}

void LandscapeEditorDrawSystem::SetCursorSize(uint32 cursorSize)
{
	this->cursorSize = cursorSize;
	landscapeProxy->SetCursorScale((float32)cursorSize);
	UpdateCursorPosition();
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
	cursorPosition = cursorPos;// - Vector2(cursorSize / 2.f, cursorSize / 2.f);
	UpdateCursorPosition();
}

void LandscapeEditorDrawSystem::UpdateCursorPosition()
{
	Vector2 p = cursorPosition - Vector2(cursorSize / 2.f, cursorSize / 2.f);
	landscapeProxy->SetCursorPosition(p);
}

void LandscapeEditorDrawSystem::Update(DAVA::float32 timeElapsed)
{
	if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
	{
		Rect changedRect = heightmapProxy->GetChangedRect();
		
		if (landscapeProxy)
		{
			landscapeProxy->GetRenderer()->RebuildVertexes(changedRect);
		}
		
		if (notPassableTerrainProxy && notPassableTerrainProxy->IsEnabled())
		{
			notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), changedRect);
			landscapeProxy->SetNotPassableTexture(notPassableTerrainProxy->GetTexture());
		}
		
		if (customDrawRequestCount == 0)
		{
			UpdateBaseLandscapeHeightmap();
		}
		
		heightmapProxy->ResetHeightmapChanged();
	}
	
	if (customColorsProxy &&  customColorsProxy->IsSpriteChanged())
	{
		if (landscapeProxy)
		{
			landscapeProxy->SetCustomColorsTexture(customColorsProxy->GetSprite()->GetTexture());
		}
		customColorsProxy->ResetSpriteChanged();
	}

	if (visibilityToolProxy && visibilityToolProxy->IsSpriteChanged())
	{
		if (landscapeProxy)
		{
			landscapeProxy->SetVisibilityCheckToolTexture(visibilityToolProxy->GetSprite()->GetTexture());
		}
		visibilityToolProxy->ResetSpriteChanged();
	}

	if (rulerToolProxy && rulerToolProxy->IsSpriteChanged())
	{
		if (rulerToolProxy)
		{
			landscapeProxy->SetRulerToolTexture(rulerToolProxy->GetSprite()->GetTexture());
		}
		rulerToolProxy->ResetSpriteChanged();
	}
}

void LandscapeEditorDrawSystem::UpdateBaseLandscapeHeightmap()
{
	Heightmap* h = new Heightmap();
	heightmapProxy->Clone(h);
	
	baseLandscape->SetHeightmap(h);
	
	SafeRelease(h);
}

float32 LandscapeEditorDrawSystem::GetTextureSize()
{
	return (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();
}

Vector3 LandscapeEditorDrawSystem::GetLandscapeSize()
{
	AABBox3 transformedBox;
	baseLandscape->GetBoundingBox().GetTransformedBox(*baseLandscape->GetWorldTransformPtr(), transformedBox);
	
	Vector3 landSize = transformedBox.max - transformedBox.min;
	return landSize;
}

float32 LandscapeEditorDrawSystem::GetLandscapeMaxHeight()
{
	Vector3 landSize = GetLandscapeSize();
	return landSize.z;
}

float32 LandscapeEditorDrawSystem::GetHeightAtPoint(const Vector2& point)
{
	Heightmap *heightmap = GetHeightmapProxy();
	int32 x = (int32)point.x;
	int32 y = (int32)point.y;

	DVASSERT_MSG((x >= 0 && x < heightmap->Size()) && (y >= 0 && y < heightmap->Size()),
				 "Point must be in heightmap coordinates");

	int32 index = x + y * heightmap->Size();
	float32 height = heightmap->Data()[index];
	float32 maxHeight = GetLandscapeMaxHeight();

	height *= maxHeight;
	height /= Heightmap::MAX_VALUE;

	return height;
}

float32 LandscapeEditorDrawSystem::GetHeightAtTexturePoint(const Vector2& point)
{
	float32 textureSize = (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));

	if (textureRect.PointInside(point))
	{
		return GetHeightAtPoint(TexturePointToHeightmapPoint(point));
	}

	return 0.f;
}

Vector2 LandscapeEditorDrawSystem::HeightmapPointToTexturePoint(const Vector2& point)
{
	float32 heightmapSize = (float32)GetHeightmapProxy()->Size();
	float32 textureSize = (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	Rect heightmapRect(Vector2(0.f, 0.f), Vector2(heightmapSize, heightmapSize));
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));

	return TranslatePoint(point, heightmapRect, textureRect);
}

Vector2 LandscapeEditorDrawSystem::TexturePointToHeightmapPoint(const Vector2& point)
{
	float32 heightmapSize = (float32)GetHeightmapProxy()->Size();
	float32 textureSize = (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	Rect heightmapRect(Vector2(0.f, 0.f), Vector2(heightmapSize, heightmapSize));
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));

	return TranslatePoint(point, textureRect, heightmapRect);
}

Vector2 LandscapeEditorDrawSystem::TexturePointToLandscapePoint(const Vector2& point)
{
	AABBox3 boundingBox = GetLandscapeProxy()->GetLandscapeBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
					 (boundingBox.max - boundingBox.min).y);

	float32 textureSize = (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	Rect landRect(landPos, landSize);
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));

	return TranslatePoint(point, textureRect, landRect);
}

Vector2 LandscapeEditorDrawSystem::LandscapePointToTexturePoint(const Vector2& point)
{
	AABBox3 boundingBox = GetLandscapeProxy()->GetLandscapeBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
					 (boundingBox.max - boundingBox.min).y);

	float32 textureSize = (float32)baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	Rect landRect(landPos, landSize);
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));

	return TranslatePoint(point, landRect, textureRect);
}

Vector2 LandscapeEditorDrawSystem::TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect)
{
	DVASSERT(fromRect.dx != 0.f && fromRect.dy != 0.f);

	Vector2 origRectSize = fromRect.GetSize();
	Vector2 destRectSize = toRect.GetSize();

	Vector2 scale(destRectSize.x / origRectSize.x,
				  destRectSize.y / origRectSize.y);

	Vector2 relPos = point - fromRect.GetPosition();
	Vector2 newRelPos(relPos.x * scale.x,
					  relPos.y * scale.y);

	Vector2 newPos = newRelPos + toRect.GetPosition();

	return newPos;
}

KeyedArchive* LandscapeEditorDrawSystem::GetLandscapeCustomProperties()
{
	return landscapeNode->GetCustomProperties();
}

bool LandscapeEditorDrawSystem::EnableTilemaskEditing()
{
	if (!Init())
	{
		return false;
	}

	landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);

	landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	landscapeNode->AddComponent(new RenderComponent(landscapeProxy->GetRenderObject()));

	return true;
}

void LandscapeEditorDrawSystem::DisableTilemaskEditing()
{
}

bool LandscapeEditorDrawSystem::Init()
{
	if (!landscapeNode)
	{
		landscapeNode = EditorScene::GetLandscapeNode(GetScene());
	}
	if (!baseLandscape)
	{
		baseLandscape = SafeRetain(GetLandscape(landscapeNode));
	}
	if (!baseLandscape)
	{
		return false;
	}
	if (baseLandscape && !landscapeProxy)
	{
		landscapeProxy = new LandscapeProxy(baseLandscape);
	}
	if (!heightmapProxy)
	{
		heightmapProxy = new HeightmapProxy(baseLandscape->GetHeightmap()->Clone(NULL));
	}
	if (!customColorsProxy)
	{
		customColorsProxy = new CustomColorsProxy((int32)GetTextureSize());
	}
	if (!visibilityToolProxy)
	{
		visibilityToolProxy = new VisibilityToolProxy((int32)GetTextureSize());
	}
	if (!rulerToolProxy)
	{
		rulerToolProxy = new RulerToolProxy((int32)GetTextureSize());
	}

	return true;
}
