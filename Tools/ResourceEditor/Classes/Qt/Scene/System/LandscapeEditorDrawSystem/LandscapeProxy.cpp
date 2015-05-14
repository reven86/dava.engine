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

LandscapeProxy::LandscapeProxy(Landscape* landscape, Entity* node)
:   tilemaskImageCopy(NULL)
,	tilemaskWasChanged(0)
,	fullTiledTexture(NULL)
,   displayingTexture(0)
,	mode(MODE_ORIGINAL_LANDSCAPE)
,	cursorTexture(NULL)
{
	DVASSERT(landscape != NULL);

	tilemaskTextures[TILEMASK_SPRITE_SOURCE] = NULL;
	tilemaskTextures[TILEMASK_SPRITE_DESTINATION] = NULL;

	baseLandscape = SafeRetain(landscape);
	landscapeNode = SafeRetain(node);
	for (int32 i = 0; i < TEXTURE_TYPES_COUNT; ++i)
	{
		texturesToBlend[i] = NULL;
		texturesEnabled[i] = false;
	}
	    

	customLandscape = new CustomLandscape();
#if RHI_COMPLETE_EDITOR
    customLandscape->Create();
	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));
#endif // RHI_COMPLETE_EDITOR
	customLandscape->SetAABBox(baseLandscape->GetBoundingBox());

    customLandscape->SetReflectionVisible(baseLandscape->GetReflectionVisible());
    customLandscape->SetRefractionVisible(baseLandscape->GetRefractionVisible());

    displayingTexture = Texture::CreateFBO(2048, 2048, FORMAT_RGBA8888);

}

LandscapeProxy::~LandscapeProxy()
{
	SafeRelease(landscapeNode);
	SafeRelease(baseLandscape);
	SafeRelease(displayingTexture);
	SafeRelease(customLandscape);
	SafeRelease(tilemaskImageCopy);
	SafeRelease(tilemaskTextures[TILEMASK_SPRITE_SOURCE]);
	SafeRelease(tilemaskTextures[TILEMASK_SPRITE_DESTINATION]);
	SafeRelease(fullTiledTexture);

    SafeRelease(cursorTexture);
	
}

void LandscapeProxy::SetMode(LandscapeProxy::eLandscapeMode mode)
{
	if (mode == this->mode)
	{
		return;
	}

	this->mode = mode;

	landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	landscapeNode->AddComponent(new RenderComponent(GetRenderObject()));
}

void LandscapeProxy::SetRenderer(LandscapeRenderer *renderer)
{
	customLandscape->SetRenderer(renderer);
}

LandscapeRenderer* LandscapeProxy::GetRenderer()
{
	return customLandscape->GetRenderer();
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
#if RHI_COMPLETE_EDITOR
	FilePath texturePathname = baseLandscape->GetTextureName(Landscape::TEXTURE_TILE_MASK);
	baseLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
	baseLandscape->SetTextureName(Landscape::TEXTURE_TILE_MASK, texturePathname);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetNotPassableTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE]);
	texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetNotPassableTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetCustomColorsTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS]);
	texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetCustomColorsTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetVisibilityCheckToolTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL]);
	texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetVisibilityCheckToolTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetRulerToolTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_RULER_TOOL]);
	texturesToBlend[TEXTURE_TYPE_RULER_TOOL] = SafeRetain(texture);

	UpdateDisplayedTexture();
}

void LandscapeProxy::SetRulerToolTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_RULER_TOOL] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::UpdateDisplayedTexture()
{
#if RHI_COMPLETE_EDITOR
    RenderHelper::Instance()->Set2DRenderTarget(displayingTexture);
    RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
    RenderManager::Instance()->SetColor(Color::White);
    RenderHelper::Instance()->DrawTexture(fullTiledTexture, RenderState::RENDERSTATE_2D_OPAQUE);

	Texture* notPassableTexture = texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE];
	if (notPassableTexture && texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE])
    {
        RenderHelper::Instance()->DrawTexture(notPassableTexture, RenderState::RENDERSTATE_2D_BLEND);
	}

	Texture* customColorsTexture = texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS];
	if (customColorsTexture && texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS])
	{
		RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, .5f);
        RenderHelper::Instance()->DrawTexture(customColorsTexture, RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(Color::White);
	}
	
	Texture* visibilityCheckToolTexture = texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL];
	if (visibilityCheckToolTexture && texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL])
	{
        RenderHelper::Instance()->DrawTexture(visibilityCheckToolTexture, RenderState::RENDERSTATE_2D_BLEND);
	}

	Texture* rulerToolTexture = texturesToBlend[TEXTURE_TYPE_RULER_TOOL];
	if (rulerToolTexture && texturesEnabled[TEXTURE_TYPE_RULER_TOOL])
	{
        RenderHelper::Instance()->DrawTexture(rulerToolTexture, RenderState::RENDERSTATE_2D_BLEND);
	}

    RenderManager::Instance()->SetRenderTarget(0);
    RenderSystem2D::Instance()->Setup2DMatrices();

	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, displayingTexture);
	customLandscape->UpdateTextureState();
#endif // RHI_COMPLETE_EDITOR
}

RenderObject* LandscapeProxy::GetRenderObject()
{
	switch (mode)
	{
		case MODE_CUSTOM_LANDSCAPE:
			return customLandscape;

		case MODE_ORIGINAL_LANDSCAPE:
			return baseLandscape;

		default:
			return NULL;
	}
}

void LandscapeProxy::SetHeightmap(DAVA::Heightmap *heightmap)
{
	switch (mode)
	{
		case MODE_CUSTOM_LANDSCAPE:
			customLandscape->SetHeightmap(heightmap);
			break;

		case MODE_ORIGINAL_LANDSCAPE:
			baseLandscape->SetHeightmap(heightmap);
			break;

		default:
			break;
	}
}

void LandscapeProxy::CursorEnable()
{
#if RHI_COMPLETE_EDITOR
	customLandscape->CursorEnable();
	baseLandscape->CursorEnable();
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::CursorDisable()
{
#if RHI_COMPLETE_EDITOR
	customLandscape->CursorDisable();
	baseLandscape->CursorDisable();
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetCursorTexture(Texture* texture)
{
    if(cursorTexture != texture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(texture);
    }
    
#if RHI_COMPLETE_EDITOR
    customLandscape->GetCursor()->SetCursorTexture(texture);
    baseLandscape->GetCursor()->SetCursorTexture(texture);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetBigTextureSize(float32 size)
{
#if RHI_COMPLETE_EDITOR
	customLandscape->GetCursor()->SetBigTextureSize(size);
	baseLandscape->GetCursor()->SetBigTextureSize(size);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetCursorScale(float32 scale)
{
#if RHI_COMPLETE_EDITOR
	customLandscape->GetCursor()->SetScale(scale);
	baseLandscape->GetCursor()->SetScale(scale);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
#if RHI_COMPLETE_EDITOR
	customLandscape->GetCursor()->SetPosition(position);
	baseLandscape->GetCursor()->SetPosition(position);
#endif // RHI_COMPLETE_EDITOR
}

void LandscapeProxy::UpdateFullTiledTexture(bool force)
{
#if RHI_COMPLETE_EDITOR
	if (force || mode == MODE_CUSTOM_LANDSCAPE)
	{
		SafeRelease(fullTiledTexture);

		RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
		RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
		RenderManager::Instance()->FlushState();
//		baseLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->GenerateMipmaps();

		eLandscapeMode oldMode = mode;
		SetMode(MODE_ORIGINAL_LANDSCAPE);
		fullTiledTexture = baseLandscape->CreateLandscapeTexture();
		SetMode(oldMode);

		UpdateDisplayedTexture();
	}
#endif // RHI_COMPLETE_EDITOR
}

Vector3 LandscapeProxy::PlacePoint(const Vector3& point)
{
	Vector3 landscapePoint;
	if (mode == MODE_ORIGINAL_LANDSCAPE)
	{
		baseLandscape->PlacePoint(point, landscapePoint);
	}
	else if (mode == MODE_CUSTOM_LANDSCAPE)
	{
		customLandscape->PlacePoint(point, landscapePoint);
	}

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
#if RHI_COMPLETE_EDITOR
	if (tilemaskTextures[TILEMASK_SPRITE_SOURCE] == NULL
		|| tilemaskTextures[TILEMASK_SPRITE_DESTINATION] == NULL)
	{
		SafeRelease(tilemaskTextures[TILEMASK_SPRITE_SOURCE]);
		SafeRelease(tilemaskTextures[TILEMASK_SPRITE_DESTINATION]);

        uint32 texSize = (uint32)GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->GetWidth();
		tilemaskTextures[TILEMASK_SPRITE_SOURCE] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);
        tilemaskTextures[TILEMASK_SPRITE_DESTINATION] = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);

        RenderHelper::Instance()->Set2DRenderTarget(tilemaskTextures[TILEMASK_SPRITE_SOURCE]);
        RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
        RenderHelper::Instance()->Set2DRenderTarget(tilemaskTextures[TILEMASK_SPRITE_DESTINATION]);
        RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

        RenderManager::Instance()->SetRenderTarget(0);
	}
#endif // RHI_COMPLETE_EDITOR
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
