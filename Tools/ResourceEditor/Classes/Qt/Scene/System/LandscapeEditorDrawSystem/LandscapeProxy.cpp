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


#include "Qt/Main/QtUtils.h"
#include "LandscapeProxy.h"
#include "CustomLandscape.h"

const FastName LandscapeProxy::LANDSCAPE_TEXTURE_TOOL("toolTexture");
const FastName LandscapeProxy::LANDSCAPE_TEXTURE_CURSOR("cursorTexture");
const FastName LandscapeProxy::LANSDCAPE_FLAG_CURSOR("LANDSCAPE_CURSOR");
const FastName LandscapeProxy::LANSDCAPE_FLAG_TOOL("LANDSCAPE_TOOL");
const FastName LandscapeProxy::LANDSCAPE_PARAM_CURSOR_COORD_SIZE("cursorCoordSize");

LandscapeProxy::LandscapeProxy(Landscape* landscape, Entity* node)
:   tilemaskImageCopy(NULL)
,	tilemaskWasChanged(0)
,	mode(MODE_ORIGINAL_LANDSCAPE)
,	cursorTexture(NULL)
{
	DVASSERT(landscape != NULL);

	tilemaskTextures[TILEMASK_SPRITE_SOURCE] = NULL;
	tilemaskTextures[TILEMASK_SPRITE_DESTINATION] = NULL;

	baseLandscape = SafeRetain(landscape);

    landscapeEditorMaterial = new NMaterial();
    landscapeEditorMaterial->SetFXName(FastName("~res:/Materials/Landscape.Tool.material"));
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_TOOL, 0);
    landscapeEditorMaterial->AddFlag(LANSDCAPE_FLAG_CURSOR, 0);
    landscapeEditorMaterial->AddProperty(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data, rhi::ShaderProp::TYPE_FLOAT4);
}

LandscapeProxy::~LandscapeProxy()
{
    SafeRelease(landscapeEditorMaterial);

	SafeRelease(baseLandscape);
	SafeRelease(tilemaskImageCopy);
	SafeRelease(tilemaskTextures[TILEMASK_SPRITE_SOURCE]);
	SafeRelease(tilemaskTextures[TILEMASK_SPRITE_DESTINATION]);

    SafeRelease(cursorTexture);
}

void LandscapeProxy::SetMode(LandscapeProxy::eLandscapeMode mode)
{
	this->mode = mode;

    switch (mode)
    {
    case LandscapeProxy::MODE_CUSTOM_LANDSCAPE:
    {
        landscapeEditorMaterial->SetParent(baseLandscape->GetMaterial());
        baseLandscape->SetMaterial(landscapeEditorMaterial);
    }
    break;
    case LandscapeProxy::MODE_ORIGINAL_LANDSCAPE:
    {
        baseLandscape->SetMaterial(landscapeEditorMaterial->GetParent());
        landscapeEditorMaterial->SetParent(nullptr);
    }
    break;
    default:
        break;
    }
}

const AABBox3 & LandscapeProxy::GetLandscapeBoundingBox()
{
	return baseLandscape->GetBoundingBox();
}

Texture* LandscapeProxy::GetLandscapeTexture(const FastName& level)
{
	return baseLandscape->GetMaterial()->GetEffectiveTexture(level);
}

Color LandscapeProxy::GetLandscapeTileColor(const FastName& level)
{
#if RHI_COMPLETE_EDITOR
	return baseLandscape->GetTileColor(level);
#else
    return Color();
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetLandscapeTileColor(const FastName& level, const Color& color)
{
#if RHI_COMPLETE_EDITOR
	baseLandscape->SetTileColor(level, color);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetTilemaskTexture(Texture* texture)
{
    FilePath texturePathname = baseLandscape->GetMaterial()->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK)->GetPathname();
    texture->SetPathname(texturePathname);
    baseLandscape->GetMaterial()->SetTexture(Landscape::TEXTURE_TILEMASK, texture);
}

void LandscapeProxy::SetToolTexture(Texture * texture)
{
    if (texture)
    {
        landscapeEditorMaterial->AddTexture(LANDSCAPE_TEXTURE_TOOL, texture);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 1);
    }
    else
    {
        landscapeEditorMaterial->RemoveTexture(LANDSCAPE_TEXTURE_TOOL);
        landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_TOOL, 0);
    }
}

void LandscapeProxy::SetHeightmap(DAVA::Heightmap *heightmap)
{
    baseLandscape->SetHeightmap(heightmap);
}

void LandscapeProxy::CursorEnable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 1);
}

void LandscapeProxy::CursorDisable()
{
    landscapeEditorMaterial->SetFlag(LANSDCAPE_FLAG_CURSOR, 0);
}

void LandscapeProxy::SetCursorTexture(Texture* texture)
{
    if(cursorTexture != texture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(texture);
    }
    
    landscapeEditorMaterial->SetTexture(LANDSCAPE_TEXTURE_CURSOR, texture);
}

void LandscapeProxy::SetCursorSize(float32 size)
{
    cursorCoordSize.z = size;
    cursorCoordSize.w = size;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
    cursorCoordSize.x = position.x;
    cursorCoordSize.y = position.y;

    landscapeEditorMaterial->SetPropertyValue(LANDSCAPE_PARAM_CURSOR_COORD_SIZE, cursorCoordSize.data);
}

Vector3 LandscapeProxy::PlacePoint(const Vector3& point)
{
	Vector3 landscapePoint;
    baseLandscape->PlacePoint(point, landscapePoint);

	return landscapePoint;
}

bool LandscapeProxy::IsTilemaskChanged()
{
	return (tilemaskWasChanged != 0);
}

void LandscapeProxy::ResetTilemaskChanged()
{
	tilemaskWasChanged = 0;
}

void LandscapeProxy::IncreaseTilemaskChanges()
{
	++tilemaskWasChanged;
}

void LandscapeProxy::DecreaseTilemaskChanges()
{
	--tilemaskWasChanged;
}

void LandscapeProxy::InitTilemaskImageCopy()
{
#if RHI_COMPLETE_EDITOR
	SafeRelease(tilemaskImageCopy);

    RenderManager::Instance()->SetColor(Color::White);
	tilemaskImageCopy = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory(noBlendDrawState);
#endif // RHI_COMPLETE_EDITOR
}

Image* LandscapeProxy::GetTilemaskImageCopy()
{
	return tilemaskImageCopy;
}

void LandscapeProxy::InitTilemaskSprites()
{
	if (tilemaskTextures[TILEMASK_SPRITE_SOURCE] == NULL || tilemaskTextures[TILEMASK_SPRITE_DESTINATION] == NULL)
	{
		SafeRelease(tilemaskTextures[TILEMASK_SPRITE_SOURCE]);
		SafeRelease(tilemaskTextures[TILEMASK_SPRITE_DESTINATION]);

        uint32 texSize = (uint32)GetLandscapeTexture(Landscape::TEXTURE_TILEMASK)->GetWidth();
		tilemaskTextures[TILEMASK_SPRITE_SOURCE] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, rhi::TEXTURE_TYPE_2D);
        tilemaskTextures[TILEMASK_SPRITE_DESTINATION] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, rhi::TEXTURE_TYPE_2D);
	}
}

Texture * LandscapeProxy::GetTilemaskTexture(int32 number)
{
	if (number >= 0 && number < TILEMASK_SPRITES_COUNT)
	{
		return tilemaskTextures[number];
	}

	return NULL;
}

void LandscapeProxy::SwapTilemaskSprites()
{
	Texture* temp = tilemaskTextures[TILEMASK_SPRITE_SOURCE];
	tilemaskTextures[TILEMASK_SPRITE_SOURCE] = tilemaskTextures[TILEMASK_SPRITE_DESTINATION];
	tilemaskTextures[TILEMASK_SPRITE_DESTINATION] = temp;
}
