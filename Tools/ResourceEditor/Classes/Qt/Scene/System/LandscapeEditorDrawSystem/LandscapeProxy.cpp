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



#include "LandscapeProxy.h"
#include "CustomLandscape.h"

LandscapeProxy::LandscapeProxy(Landscape* landscape, Entity* node)
:	displayingTexture(0)
,	mode(MODE_ORIGINAL_LANDSCAPE)
,	tilemaskWasChanged(0)
,	tilemaskImageCopy(NULL)
,	fullTiledTexture(NULL)
,	fullTiledTextureState(InvalidUniqueHandle)
,	cursorTexture(NULL)
{
	DVASSERT(landscape != NULL);

	tilemaskSprites[TILEMASK_SPRITE_SOURCE] = NULL;
	tilemaskSprites[TILEMASK_SPRITE_DESTINATION] = NULL;

	baseLandscape = SafeRetain(landscape);
	landscapeNode = SafeRetain(node);
	for (int32 i = 0; i < TEXTURE_TYPES_COUNT; ++i)
	{
		texturesToBlend[i] = NULL;
		texturesEnabled[i] = false;
	}
	
    DAVA::RenderStateData noBlendStateData;
    DAVA::RenderManager::Instance()->GetRenderStateData(DAVA::RenderState::RENDERSTATE_2D_BLEND, noBlendStateData);
    
	noBlendStateData.sourceFactor = DAVA::BLEND_ONE;
	noBlendStateData.destFactor = DAVA::BLEND_ZERO;
	
	noBlendDrawState = DAVA::RenderManager::Instance()->CreateRenderState(noBlendStateData);

	customLandscape = new CustomLandscape();
	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));
	customLandscape->SetAABBox(baseLandscape->GetBoundingBox());
}

LandscapeProxy::~LandscapeProxy()
{
	SafeRelease(landscapeNode);
	SafeRelease(baseLandscape);
	SafeRelease(displayingTexture);
	SafeRelease(customLandscape);
	SafeRelease(tilemaskImageCopy);
	SafeRelease(tilemaskSprites[TILEMASK_SPRITE_SOURCE]);
	SafeRelease(tilemaskSprites[TILEMASK_SPRITE_DESTINATION]);
	SafeRelease(fullTiledTexture);

    SafeRelease(cursorTexture);

	if (fullTiledTextureState != InvalidUniqueHandle)
	{
		RenderManager::Instance()->ReleaseTextureState(fullTiledTextureState);
	}

	RenderManager::Instance()->ReleaseRenderState(noBlendDrawState);
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

void LandscapeProxy::SetDisplayingTexture(DAVA::Texture *texture)
{
	SafeRelease(displayingTexture);
	displayingTexture = SafeRetain(texture);
}

AABBox3 LandscapeProxy::GetLandscapeBoundingBox()
{
	return baseLandscape->GetBoundingBox();
}

Texture* LandscapeProxy::GetLandscapeTexture(Landscape::eTextureLevel level)
{
	return baseLandscape->GetTexture(level);
}

Color LandscapeProxy::GetLandscapeTileColor(Landscape::eTextureLevel level)
{
	return baseLandscape->GetTileColor(level);
}

void LandscapeProxy::SetLandscapeTileColor(Landscape::eTextureLevel level, const Color& color)
{
	baseLandscape->SetTileColor(level, color);
}

void LandscapeProxy::SetTilemaskTexture(Texture* texture)
{
	FilePath texturePathname = baseLandscape->GetTextureName(Landscape::TEXTURE_TILE_MASK);
	baseLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
	baseLandscape->SetTextureName(Landscape::TEXTURE_TILE_MASK, texturePathname);
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
	//RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
	//RenderManager::Instance()->SetTextureState(fullTiledTextureState);
	//RenderManager::Instance()->FlushState();

	int32 fullTiledWidth = fullTiledTexture->GetWidth();
	int32 fullTiledHeight = fullTiledTexture->GetHeight();
	Sprite* fullTiledSprite = Sprite::CreateFromTexture(fullTiledTexture, 0, 0,
														(float32)fullTiledWidth, (float32)fullTiledHeight);

	Sprite *dstSprite = Sprite::CreateAsRenderTarget((float32)fullTiledWidth, (float32)fullTiledHeight, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
    Sprite::DrawState drawState;
    drawState.SetPosition(0.f, 0.f);
	fullTiledSprite->Draw(&drawState);
	SafeRelease(fullTiledSprite);

	Texture* notPassableTexture = texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE];
	Sprite* notPassableSprite = NULL;
	if (notPassableTexture && texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE])
	{
		RenderManager::Instance()->SetColor(Color::White);
		notPassableSprite = Sprite::CreateFromTexture(notPassableTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
        
        drawState.Reset();
		notPassableSprite->Draw(&drawState);
	}
	SafeRelease(notPassableSprite);
	
	Texture* customColorsTexture = texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS];
	Sprite* customColorsSprite = NULL;
	if (customColorsTexture && texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS])
	{
		RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, .5f);
		customColorsSprite = Sprite::CreateFromTexture(customColorsTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
        
        drawState.Reset();
		customColorsSprite->Draw(&drawState);
		RenderManager::Instance()->SetColor(Color::White);
	}
	SafeRelease(customColorsSprite);
	
	Texture* visibilityCheckToolTexture = texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL];
	Sprite* visibilityCheckToolSprite = NULL;
	if (visibilityCheckToolTexture && texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL])
	{
		RenderManager::Instance()->SetColor(Color::White);
		visibilityCheckToolSprite = Sprite::CreateFromTexture(visibilityCheckToolTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
        
        drawState.Reset();
		visibilityCheckToolSprite->Draw(&drawState);
	}
	SafeRelease(visibilityCheckToolSprite);

	Texture* rulerToolTexture = texturesToBlend[TEXTURE_TYPE_RULER_TOOL];
	Sprite* rulerToolSprite = NULL;
	if (rulerToolTexture && texturesEnabled[TEXTURE_TYPE_RULER_TOOL])
	{
		RenderManager::Instance()->SetColor(Color::White);
		rulerToolSprite = Sprite::CreateFromTexture(rulerToolTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
        
        drawState.Reset();
		rulerToolSprite->Draw(&drawState);
	}
	SafeRelease(rulerToolSprite);

	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(displayingTexture);
	displayingTexture = SafeRetain(dstSprite->GetTexture());
	
	SafeRelease(dstSprite);

//	displayingTexture->GenerateMipmaps();
	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, displayingTexture);
	customLandscape->UpdateTextureState();
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
	customLandscape->CursorEnable();
	baseLandscape->CursorEnable();
}

void LandscapeProxy::CursorDisable()
{
	customLandscape->CursorDisable();
	baseLandscape->CursorDisable();
}

void LandscapeProxy::SetCursorTexture(Texture* texture)
{
    if(cursorTexture != texture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(texture);
    }
    
    customLandscape->GetCursor()->SetCursorTexture(texture);
    baseLandscape->GetCursor()->SetCursorTexture(texture);
}

void LandscapeProxy::SetBigTextureSize(float32 size)
{
	customLandscape->GetCursor()->SetBigTextureSize(size);
	baseLandscape->GetCursor()->SetBigTextureSize(size);
}

void LandscapeProxy::SetCursorScale(float32 scale)
{
	customLandscape->GetCursor()->SetScale(scale);
	baseLandscape->GetCursor()->SetScale(scale);
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
	customLandscape->GetCursor()->SetPosition(position);
	baseLandscape->GetCursor()->SetPosition(position);
}

void LandscapeProxy::UpdateFullTiledTexture(bool force)
{
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

		TextureStateData textureStateData;
		textureStateData.SetTexture(0, fullTiledTexture);
		UniqueHandle uniqueHandle = RenderManager::Instance()->CreateTextureState(textureStateData);
		if (fullTiledTextureState != InvalidUniqueHandle)
		{
			RenderManager::Instance()->ReleaseTextureState(fullTiledTextureState);
		}
		fullTiledTextureState = uniqueHandle;

		UpdateDisplayedTexture();
	}
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
	SafeRelease(tilemaskImageCopy);

    RenderManager::Instance()->SetColor(Color::White);
	tilemaskImageCopy = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory(noBlendDrawState);
}

Image* LandscapeProxy::GetTilemaskImageCopy()
{
	return tilemaskImageCopy;
}

void LandscapeProxy::InitTilemaskSprites()
{
	if (tilemaskSprites[TILEMASK_SPRITE_SOURCE] == NULL
		|| tilemaskSprites[TILEMASK_SPRITE_DESTINATION] == NULL)
	{
		SafeRelease(tilemaskSprites[TILEMASK_SPRITE_SOURCE]);
		SafeRelease(tilemaskSprites[TILEMASK_SPRITE_DESTINATION]);

		float32 texSize = (float32)GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->GetWidth();
		tilemaskSprites[TILEMASK_SPRITE_SOURCE] = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
		tilemaskSprites[TILEMASK_SPRITE_DESTINATION] = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	}
}

Sprite* LandscapeProxy::GetTilemaskSprite(int32 number)
{
	if (number >= 0 && number < TILEMASK_SPRITES_COUNT)
	{
		return tilemaskSprites[number];
	}

	return NULL;
}

void LandscapeProxy::SwapTilemaskSprites()
{
	Sprite* temp = tilemaskSprites[TILEMASK_SPRITE_SOURCE];
	tilemaskSprites[TILEMASK_SPRITE_SOURCE] = tilemaskSprites[TILEMASK_SPRITE_DESTINATION];
	tilemaskSprites[TILEMASK_SPRITE_DESTINATION] = temp;
}
