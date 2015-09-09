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

#include "Commands2/InspMemberModifyCommand.h"
#include "Commands2/InspDynamicModifyCommand.h"

#include "Scene3D/Systems/RenderUpdateSystem.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "Scene/SceneHelper.h"

LandscapeEditorDrawSystem::LandscapeEditorDrawSystem(Scene* scene)
:	SceneSystem(scene)
,	landscapeNode(nullptr)
,	baseLandscape(nullptr)
,	landscapeProxy(nullptr)
,	heightmapProxy(nullptr)
,	notPassableTerrainProxy(nullptr)
,	customColorsProxy(nullptr)
,	visibilityToolProxy(nullptr)
,	rulerToolProxy(nullptr)
,	customDrawRequestCount(0)
,   sourceTilemaskPath("")
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

	SafeDelete(notPassableTerrainProxy);	
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

	landscapeProxy->SetMode(LandscapeProxy::MODE_CUSTOM_LANDSCAPE);
	landscapeProxy->SetHeightmap(heightmapProxy);

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
    eErrorType canBeEnabledError = IsNotPassableTerrainCanBeEnabled();
    if (canBeEnabledError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }
    
	if (!notPassableTerrainProxy)
	{
		notPassableTerrainProxy = new NotPassableTerrainProxy(baseLandscape->GetHeightmap()->Size());
	}
	
	if (notPassableTerrainProxy->IsEnabled())
	{
		return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}

	eErrorType enableCustomDrawError = EnableCustomDraw();
	if (enableCustomDrawError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}
    
    Rect2i updateRect = Rect2i(0, 0, GetHeightmapProxy()->Size(), GetHeightmapProxy()->Size());
	notPassableTerrainProxy->Enable();
	notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), updateRect);
	
    landscapeProxy->SetToolTexture(notPassableTerrainProxy->GetTexture());
    
	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableNotPassableTerrain()
{
	if (!notPassableTerrainProxy || !notPassableTerrainProxy->IsEnabled())
	{
		return;
	}
	
	notPassableTerrainProxy->Disable();
	landscapeProxy->SetToolTexture(nullptr);
    
	DisableCustomDraw();
}

void LandscapeEditorDrawSystem::EnableCursor()
{
	landscapeProxy->CursorEnable();
}

void LandscapeEditorDrawSystem::DisableCursor()
{
	landscapeProxy->CursorDisable();
}

void LandscapeEditorDrawSystem::SetCursorTexture(Texture* cursorTexture)
{
	landscapeProxy->SetCursorTexture(cursorTexture);
}

void LandscapeEditorDrawSystem::SetCursorSize(float32 cursorSize)
{
	if (landscapeProxy)
	{
		landscapeProxy->SetCursorSize(cursorSize);
	}
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
    if (landscapeProxy)
    {
        landscapeProxy->SetCursorPosition(cursorPos);
    }
}

void LandscapeEditorDrawSystem::Process(DAVA::float32 timeElapsed)
{
	if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
	{
		Rect changedRect = heightmapProxy->GetChangedRect();
        Rect2i updateRect = Rect2i((int32)changedRect.x, (int32)changedRect.y, (int32)changedRect.dx, (int32)changedRect.dy);
        baseLandscape->UpdatePart(heightmapProxy, updateRect);
		
		if (notPassableTerrainProxy && notPassableTerrainProxy->IsEnabled())
		{
			notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), updateRect);
		}
		
		if (customDrawRequestCount == 0)
		{
			UpdateBaseLandscapeHeightmap();
		}
		
		heightmapProxy->ResetHeightmapChanged();
	}
	
	if (customColorsProxy && customColorsProxy->IsTargetChanged())
	{
		customColorsProxy->ResetTargetChanged();
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

float32 LandscapeEditorDrawSystem::GetTextureSize(const FastName& level)
{
	float32 size = 0.f;
	Texture* texture = baseLandscape->GetMaterial()->GetEffectiveTexture(level);
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

Rect LandscapeEditorDrawSystem::GetTextureRect(const FastName& level)
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

float32 LandscapeEditorDrawSystem::GetHeightAtHeightmapPoint(const Vector2& point)
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

float32 LandscapeEditorDrawSystem::GetHeightAtTexturePoint(const FastName& level, const Vector2& point)
{
    if (GetTextureRect(level).PointInside(point))
    {
        return GetHeightAtHeightmapPoint(TexturePointToHeightmapPoint(level, point));
    }
    
    return 0.f;
}

float32 LandscapeEditorDrawSystem::GetHeightAtNormalizedPoint(const Vector2& point)
{
	if (point.x >= 0.f && point.x <= 1.f && point.y >= 0.f && point.y <= 1.f)
	{
		return GetHeightAtHeightmapPoint(point * GetHeightmapProxy()->Size());
	}

	return 0.f;
}

Vector2 LandscapeEditorDrawSystem::HeightmapPointToTexturePoint(const FastName& level, const Vector2& point)
{
	return TranslatePoint(point, GetHeightmapRect(), GetTextureRect(level));
}

Vector2 LandscapeEditorDrawSystem::TexturePointToHeightmapPoint(const FastName& level, const Vector2& point)
{
	return TranslatePoint(point, GetTextureRect(level), GetHeightmapRect());
}

Vector2 LandscapeEditorDrawSystem::TexturePointToLandscapePoint(const FastName& level, const Vector2& point)
{
	return TranslatePoint(point, GetTextureRect(level), GetLandscapeRect());
}

Vector2 LandscapeEditorDrawSystem::LandscapePointToTexturePoint(const FastName& level, const Vector2& point)
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
		customColorsProxy = new CustomColorsProxy((int32)GetTextureSize(Landscape::TEXTURE_COLOR));
	}
	if (!visibilityToolProxy)
	{
        visibilityToolProxy = new VisibilityToolProxy((int32)GetTextureSize(Landscape::TEXTURE_COLOR));
	}
	if (!rulerToolProxy)
	{
        rulerToolProxy = new RulerToolProxy((int32)GetTextureSize(Landscape::TEXTURE_COLOR));
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
    
    UpdateTilemaskPathname();
    
	landscapeProxy = new LandscapeProxy(baseLandscape, landscapeNode);

	return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DeinitLandscape()
{
	landscapeNode = NULL;
	SafeRelease(landscapeProxy);
	SafeRelease(baseLandscape);
}

void LandscapeEditorDrawSystem::ClampToTexture(const FastName& level, Rect& rect)
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

    Texture* texture = GetLandscapeProxy()->GetTilemaskTexture();// baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);

	if (texture)
	{
		Image *image = texture->CreateImageFromMemory();

		if(image)
		{
            ImageSystem::Instance()->Save(sourceTilemaskPath, image);
			SafeRelease(image);
		}

		GetLandscapeProxy()->ResetTilemaskChanged();
	}
}

void LandscapeEditorDrawSystem::ResetTileMaskTexture()
{
	if (!baseLandscape)
	{
		return;
	}

    NMaterial * landscapeMaterial = baseLandscape->GetMaterial();
    while (landscapeMaterial)
    {
        if(landscapeMaterial->HasLocalTexture(Landscape::TEXTURE_TILEMASK))
            break;

        landscapeMaterial = landscapeMaterial->GetParent();
    }

    if(landscapeMaterial)
    {
        Texture * texture = Texture::CreateFromFile(sourceTilemaskPath);
        texture->Reload();
        landscapeMaterial->SetTexture(Landscape::TEXTURE_TILEMASK, texture);
        texture->Release();
    }
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

	Texture* tileMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK);
	if (tileMask == NULL || tileMask->IsPinkPlaceholder())
	{
		return LANDSCAPE_EDITOR_SYSTEM_TILE_MASK_TEXTURE_ABSENT;
	}
	
//	Texture* fullTiled = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_FULL);
//	if (fullTiled == NULL || fullTiled->IsPinkPlaceholder())
//	{
//		return LANDSCAPE_EDITOR_SYSTEM_FULL_TILED_TEXTURE_ABSENT;
//	}
    
	Texture* texTile0 = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILE);
	
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
			if (String("heightmapPath") == cmd->member->Name().c_str())
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
        case CMDID_INSP_DYNAMIC_MODIFY:
        {
            const InspDynamicModifyCommand* cmd = static_cast<const InspDynamicModifyCommand*>(command);
            if (DAVA::FastName("tileMask") == cmd->key)
            {
                UpdateTilemaskPathname();
            }
            break;
        }

        default:
            break;
    }
}

bool LandscapeEditorDrawSystem::UpdateTilemaskPathname()
{
    if(nullptr != baseLandscape)
    {
        auto texture = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
        if(nullptr != texture)
        {
            sourceTilemaskPath = texture->GetDescriptor()->GetSourceTexturePathname();
            return true;
        }
    }
    
    return false;
}
