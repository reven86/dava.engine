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
#include "LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Deprecated/LandscapeRenderer.h"

#include "Commands2/InspMemberModifyCommand.h"

#include "Scene3D/Systems/RenderUpdateSystem.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

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
,   grassEditorProxy(NULL)
{
	const DAVA::RenderStateData default3dState = DAVA::RenderManager::Instance()->GetRenderStateData(DAVA::RenderState::RENDERSTATE_3D_BLEND);
	DAVA::RenderStateData noBlendStateData;
	memcpy(&noBlendStateData, &default3dState, sizeof(noBlendStateData));
	
	noBlendStateData.sourceFactor = DAVA::BLEND_ONE;
	noBlendStateData.destFactor = DAVA::BLEND_ZERO;
	
	noBlendDrawState = DAVA::RenderManager::Instance()->CreateRenderState(noBlendStateData);
}

LandscapeEditorDrawSystem::~LandscapeEditorDrawSystem()
{
	SafeRelease(baseLandscape);
	SafeRelease(landscapeProxy);
	SafeRelease(heightmapProxy);
	SafeRelease(customColorsProxy);
	SafeRelease(visibilityToolProxy);
	SafeRelease(rulerToolProxy);
    SafeRelease(grassEditorProxy);
	SafeRelease(cursorTexture);

	SafeDelete(notPassableTerrainProxy);

	RenderManager::Instance()->ReleaseRenderState(noBlendDrawState);
}

LandscapeProxy* LandscapeEditorDrawSystem::GetLandscapeProxy()
{
	return landscapeProxy;
}

HeightmapProxy* LandscapeEditorDrawSystem::GetHeightmapProxy()
{
	return heightmapProxy;
}

GrassEditorProxy* LandscapeEditorDrawSystem::GetGrassEditorProxy()
{
    return grassEditorProxy;
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

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableCustomDraw()
{
	if (customDrawRequestCount != 0)
	{
		++customDrawRequestCount;
		return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}

	eErrorType initError = Init();
	if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return initError;
	}

	GetLandscapeProxy()->UpdateFullTiledTexture(true);
	landscapeProxy->SetMode(LandscapeProxy::MODE_CUSTOM_LANDSCAPE);
	landscapeProxy->SetHeightmap(heightmapProxy);

	AABBox3 landscapeBoundingBox = baseLandscape->GetBoundingBox();
	LandscapeRenderer* landscapeRenderer = new LandscapeRenderer(heightmapProxy, landscapeBoundingBox);
	landscapeProxy->SetRenderer(landscapeRenderer);
	landscapeRenderer->Release();

	++customDrawRequestCount;

	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
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
		landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
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

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::IsNotPassableTerrainCanBeEnabled()
{
	return VerifyLandscape();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableNotPassableTerrain()
{
	if (!notPassableTerrainProxy)
	{
		notPassableTerrainProxy = new NotPassableTerrainProxy();
	}
	
	if (notPassableTerrainProxy->IsEnabled())
	{
		return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}

	eErrorType canBeEnabledError = IsNotPassableTerrainCanBeEnabled();
	if (canBeEnabledError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return canBeEnabledError;
	}

	eErrorType enableCustomDrawError = EnableCustomDraw();
	if (enableCustomDrawError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}

	notPassableTerrainProxy->Enable();
	notPassableTerrainProxy->UpdateTexture(heightmapProxy,
										   landscapeProxy->GetLandscapeBoundingBox(),
										   GetHeightmapRect());
	
	landscapeProxy->SetNotPassableTexture(notPassableTerrainProxy->GetTexture());
	landscapeProxy->SetNotPassableTextureEnabled(true);
    
	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
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
	if (landscapeProxy)
	{
		landscapeProxy->SetCursorScale((float32)cursorSize);
		UpdateCursorPosition();
	}
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
	cursorPosition = cursorPos;// - Vector2(cursorSize / 2.f, cursorSize / 2.f);
	UpdateCursorPosition();
}

void LandscapeEditorDrawSystem::UpdateCursorPosition()
{
    Vector2 p = cursorPosition;
    if(cursorSize & 0x1)
    {
        p = p - Vector2((cursorSize - 1) / 2.f, (cursorSize - 1) / 2.f);
    }
    else
    {
        p = p - Vector2(cursorSize / 2.f, cursorSize / 2.f);
    }
	 
	landscapeProxy->SetCursorPosition(p);
}

void LandscapeEditorDrawSystem::Process(DAVA::float32 timeElapsed)
{
	if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
	{
		Rect changedRect = heightmapProxy->GetChangedRect();
		
		if (landscapeProxy)
		{
			LandscapeRenderer* renderer = landscapeProxy->GetRenderer();
			if (renderer)
			{
				renderer->RebuildVertexes(changedRect);
			}
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
	
	if (customColorsProxy && customColorsProxy->IsTargetChanged())
	{
		if (landscapeProxy)
		{
			landscapeProxy->SetCustomColorsTexture(customColorsProxy->GetTexture());
		}
		customColorsProxy->ResetTargetChanged();
	}

	if (visibilityToolProxy && visibilityToolProxy->IsTextureChanged())
	{
		if (landscapeProxy)
		{
			landscapeProxy->SetVisibilityCheckToolTexture(visibilityToolProxy->GetTexture());
		}
		visibilityToolProxy->ResetTextureChanged();
	}

	if (rulerToolProxy && rulerToolProxy->IsTextureChanged())
	{
		if (rulerToolProxy)
		{
			landscapeProxy->SetRulerToolTexture(rulerToolProxy->GetTexture());
		}
		rulerToolProxy->ResetTextureChanged();
	}
}

void LandscapeEditorDrawSystem::UpdateBaseLandscapeHeightmap()
{
	Heightmap* h = new Heightmap();
	heightmapProxy->Clone(h);
	
	baseLandscape->SetHeightmap(h);
	
	SafeRelease(h);
    
    GetScene()->foliageSystem->SyncFoliageWithLandscape();
}

float32 LandscapeEditorDrawSystem::GetTextureSize(Landscape::eTextureLevel level)
{
	float32 size = 0.f;
	if (level == Landscape::TEXTURE_TILE_FULL)
	{
		level = Landscape::TEXTURE_TILE_MASK;
	}
	Texture* texture = baseLandscape->GetTexture(level);
	if (texture)
	{
		size = (float32)texture->GetWidth();
	}
	return size;
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

Rect LandscapeEditorDrawSystem::GetTextureRect(Landscape::eTextureLevel level)
{
	float32 textureSize = GetTextureSize(level);
	return Rect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));
}

Rect LandscapeEditorDrawSystem::GetHeightmapRect()
{
	float32 heightmapSize = (float32)GetHeightmapProxy()->Size();
	return Rect(Vector2(0.f, 0.f), Vector2(heightmapSize, heightmapSize));
}

Rect LandscapeEditorDrawSystem::GetLandscapeRect()
{
	AABBox3 boundingBox = GetLandscapeProxy()->GetLandscapeBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
					 (boundingBox.max - boundingBox.min).y);

	return Rect(landPos, landSize);
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

float32 LandscapeEditorDrawSystem::GetHeightAtTexturePoint(Landscape::eTextureLevel level, const Vector2& point)
{
	if (GetTextureRect(level).PointInside(point))
	{
		return GetHeightAtPoint(TexturePointToHeightmapPoint(level, point));
	}

	return 0.f;
}

Vector2 LandscapeEditorDrawSystem::HeightmapPointToTexturePoint(Landscape::eTextureLevel level, const Vector2& point)
{
	return TranslatePoint(point, GetHeightmapRect(), GetTextureRect(level));
}

Vector2 LandscapeEditorDrawSystem::TexturePointToHeightmapPoint(Landscape::eTextureLevel level, const Vector2& point)
{
	return TranslatePoint(point, GetTextureRect(level), GetHeightmapRect());
}

Vector2 LandscapeEditorDrawSystem::TexturePointToLandscapePoint(Landscape::eTextureLevel level, const Vector2& point)
{
	return TranslatePoint(point, GetTextureRect(level), GetLandscapeRect());
}

Vector2 LandscapeEditorDrawSystem::LandscapePointToTexturePoint(Landscape::eTextureLevel level, const Vector2& point)
{
	return TranslatePoint(point, GetLandscapeRect(), GetTextureRect(level));
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
					  toRect.dy - 1.0f - relPos.y * scale.y);

	Vector2 newPos = newRelPos + toRect.GetPosition();

	return newPos;
}

KeyedArchive* LandscapeEditorDrawSystem::GetLandscapeCustomProperties()
{
	return GetOrCreateCustomProperties(landscapeNode)->GetArchive();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableTilemaskEditing()
{
	eErrorType initError = Init();
	if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return initError;
	}

	landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableTilemaskEditing()
{}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::Init()
{
	if (!heightmapProxy)
	{
		Heightmap* heightmap = baseLandscape->GetHeightmap();
		if (heightmap == NULL || heightmap->Size() == 0)
		{
			return LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
		}
		heightmapProxy = new HeightmapProxy(baseLandscape->GetHeightmap()->Clone(NULL));
	}
	if (!customColorsProxy)
	{
		customColorsProxy = new CustomColorsProxy((int32)GetTextureSize(Landscape::TEXTURE_TILE_FULL));
	}
	if (!visibilityToolProxy)
	{
		visibilityToolProxy = new VisibilityToolProxy((int32)GetTextureSize(Landscape::TEXTURE_TILE_FULL));
	}
	if (!rulerToolProxy)
	{
		rulerToolProxy = new RulerToolProxy((int32)GetTextureSize(Landscape::TEXTURE_TILE_FULL));
	}
    if(!grassEditorProxy)
    {
        grassEditorProxy = new GrassEditorProxy(NULL);
    }

	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::InitLandscape(Entity* landscapeEntity, Landscape* landscape)
{
	DeinitLandscape();

	if (!landscapeEntity || !landscape)
	{
		return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
	}

	landscapeNode = landscapeEntity;
	baseLandscape = SafeRetain(landscape);
	landscapeProxy = new LandscapeProxy(baseLandscape, landscapeNode);

	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DeinitLandscape()
{
	landscapeNode = NULL;
	SafeRelease(landscapeProxy);
	SafeRelease(baseLandscape);
}

void LandscapeEditorDrawSystem::ClampToTexture(Landscape::eTextureLevel level, Rect& rect)
{
	GetTextureRect(level).ClampToRect(rect);
}

void LandscapeEditorDrawSystem::ClampToHeightmap(Rect& rect)
{
	GetHeightmapRect().ClampToRect(rect);
}

void LandscapeEditorDrawSystem::AddEntity(DAVA::Entity * entity)
{
	Landscape* landscape = GetLandscape(entity);
	if (landscape != NULL)
	{
        entity->SetLocked(true);
        
		InitLandscape(entity, landscape);
	}
}

void LandscapeEditorDrawSystem::RemoveEntity(DAVA::Entity * entity)
{
	if (entity == landscapeNode)
	{
		SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());

		bool needRemoveBaseLandscape = sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL
																   & ~SceneEditor2::LANDSCAPE_TOOL_TILEMAP_EDITOR);

		sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

		if (needRemoveBaseLandscape)
		{
			sceneEditor->renderUpdateSystem->RemoveEntity(entity);
		}

		DeinitLandscape();

		Entity* entity = FindLandscapeEntity(sceneEditor);
		if (entity != NULL)
		{
			InitLandscape(entity, GetLandscape(entity));
		}
	}
}

void LandscapeEditorDrawSystem::SaveTileMaskTexture()
{
	if (!baseLandscape)
	{
		return;
	}

	if (!GetLandscapeProxy()->IsTilemaskChanged())
	{
		return;
	}

	Texture* texture = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK);

	if (texture)
	{
		FilePath texturePathname = baseLandscape->GetTextureName(Landscape::TEXTURE_TILE_MASK);

		if (texturePathname.IsEmpty())
		{
			return;
		}

		texturePathname.ReplaceExtension(".png");

		//eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
		//eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
		//RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
		
		Image *image = texture->CreateImageFromMemory(noBlendDrawState);
		//RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);

		if(image)
		{
            ImageSystem::Instance()->Save(texturePathname, image);
			SafeRelease(image);
		}

		TextureDescriptorUtils::CreateDescriptorIfNeed(texturePathname);

		GetLandscapeProxy()->ResetTilemaskChanged();
	}
}

void LandscapeEditorDrawSystem::ResetTileMaskTexture()
{
	if (!baseLandscape)
	{
		return;
	}

	FilePath filePath = baseLandscape->GetTextureName(Landscape::TEXTURE_TILE_MASK);
	baseLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, "");
	baseLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, filePath);
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::VerifyLandscape() const
{
	//landscape initialization should be handled by AddEntity/RemoveEntity methods
	if (!landscapeNode || !baseLandscape || !landscapeProxy)
	{
		return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
	}

//	Texture* t = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_FULL);
//	if (t == NULL || t->IsPinkPlaceholder())
//	{
//		landscapeProxy->UpdateFullTiledTexture(true);
//	}

	Texture* tileMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);
	if (tileMask == NULL || tileMask->IsPinkPlaceholder())
	{
		return LANDSCAPE_EDITOR_SYSTEM_TILE_MASK_TEXTURE_ABSENT;
	}
	
//	Texture* fullTiled = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_FULL);
//	if (fullTiled == NULL || fullTiled->IsPinkPlaceholder())
//	{
//		return LANDSCAPE_EDITOR_SYSTEM_FULL_TILED_TEXTURE_ABSENT;
//	}

	Texture* texTile0 = baseLandscape->GetTexture(Landscape::TEXTURE_TILE0);
	
	if ((texTile0 == NULL || texTile0->IsPinkPlaceholder()))
	{
		return LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE0_TEXTURE_ABSENT;
    }

	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

Landscape * LandscapeEditorDrawSystem::GetBaseLandscape() const
{
	return baseLandscape;
}

String LandscapeEditorDrawSystem::GetDescriptionByError(eErrorType error)
{
	String ret;
	switch (error)
	{
		case LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_TILE_MASK_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSETN;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_FULL_TILED_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_FULL_TILED_TEXTURE_ABSETN;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE0_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE0_ABSENT;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE1_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE1_ABSENT;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE2_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE2_ABSENT;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE3_TEXTURE_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE3_ABSENT;
			break;
		case LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT:
			ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
			break;
        case LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT:
            ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT;
            break;
			
		default:
			break;
	}
	return ret;
}

void LandscapeEditorDrawSystem::ProcessCommand(const Command2 *command, bool redo)
{
    if (command == NULL)
    {
        return;
    }

    switch(command->GetId())
    {
        case CMDID_INSP_MEMBER_MODIFY:
        {
            const InspMemberModifyCommand* cmd = static_cast<const InspMemberModifyCommand*>(command);
            if (String("heightmapPath") == cmd->member->Name())
            {
                if (heightmapProxy)
                {
                    baseLandscape->GetHeightmap()->Clone(heightmapProxy);
                    int32 size = heightmapProxy->Size();
                    heightmapProxy->UpdateRect(Rect(0.f, 0.f, (float32)size, (float32)size));
                }
            }
            break;
        }

        default:
            break;
    }
}
