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

LandscapeProxy::LandscapeProxy(Landscape* landscape)
:	displayingTexture(0)
,	mode(MODE_CUSTOM_LANDSCAPE)
{
	baseLandscape = SafeRetain(landscape);
	for (int32 i = 0; i < TEXTURE_TYPES_COUNT; ++i)
	{
		texturesToBlend[i] = NULL;
		texturesEnabled[i] = false;
	}

	customLandscape = new CustomLandscape();
	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));
}

LandscapeProxy::~LandscapeProxy()
{
	SafeRelease(baseLandscape);
	SafeRelease(displayingTexture);
	SafeRelease(customLandscape);
}

void LandscapeProxy::SetMode(LandscapeProxy::eLandscapeMode mode)
{
	if (mode == this->mode)
	{
		return;
	}

	this->mode = mode;
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

void LandscapeProxy::SetTilemaskTexture(Texture* texture)
{
	baseLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
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
	RenderManager::Instance()->LockNonMain();

	Texture* fullTiledTexture = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);

	int32 fullTiledWidth = fullTiledTexture->GetWidth();
	int32 fullTiledHeight = fullTiledTexture->GetHeight();
	Sprite* fullTiledSprite = Sprite::CreateFromTexture(fullTiledTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);

	Sprite *dstSprite = Sprite::CreateAsRenderTarget((float32)fullTiledWidth, (float32)fullTiledHeight, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
	fullTiledSprite->SetPosition(0.f, 0.f);
	fullTiledSprite->Draw();
	SafeRelease(fullTiledSprite);
	
	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	
	Texture* notPassableTexture = texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE];
	Sprite* notPassableSprite = NULL;
	if (notPassableTexture && texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE])
	{
		RenderManager::Instance()->SetColor(Color::White());
		notPassableSprite = Sprite::CreateFromTexture(notPassableTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
		notPassableSprite->SetPosition(0.f, 0.f);
		notPassableSprite->Draw();
	}
	SafeRelease(notPassableSprite);
	
	Texture* customColorsTexture = texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS];
	Sprite* customColorsSprite = NULL;
	if (customColorsTexture && texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS])
	{
		RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, .5f);
		customColorsSprite = Sprite::CreateFromTexture(customColorsTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
		customColorsSprite->SetPosition(0.f, 0.f);
		customColorsSprite->Draw();
		RenderManager::Instance()->SetColor(Color::White());
	}
	SafeRelease(customColorsSprite);
	
	Texture* visibilityCheckToolTexture = texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL];
	Sprite* visibilityCheckToolSprite = NULL;
	if (visibilityCheckToolTexture && texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL])
	{
		RenderManager::Instance()->SetColor(Color::White());
		visibilityCheckToolSprite = Sprite::CreateFromTexture(visibilityCheckToolTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
		visibilityCheckToolSprite->SetPosition(0.f, 0.f);
		visibilityCheckToolSprite->Draw();
	}
	SafeRelease(visibilityCheckToolSprite);

	Texture* rulerToolTexture = texturesToBlend[TEXTURE_TYPE_RULER_TOOL];
	Sprite* rulerToolSprite = NULL;
	if (rulerToolTexture && texturesEnabled[TEXTURE_TYPE_RULER_TOOL])
	{
		RenderManager::Instance()->SetColor(Color::White());
		rulerToolSprite = Sprite::CreateFromTexture(rulerToolTexture, 0, 0, (float32)fullTiledWidth, (float32)fullTiledHeight);
		rulerToolSprite->SetPosition(0.f, 0.f);
		rulerToolSprite->Draw();
	}
	SafeRelease(rulerToolSprite);

	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(displayingTexture);
	displayingTexture = SafeRetain(dstSprite->GetTexture());
	
	SafeRelease(dstSprite);
	
	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	
	RenderManager::Instance()->UnlockNonMain();

	customLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, displayingTexture);
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
	customLandscape->SetCursorTexture(texture);
	baseLandscape->SetCursorTexture(texture);
}

void LandscapeProxy::SetBigTextureSize(float32 size)
{
	customLandscape->SetBigTextureSize(size);
	baseLandscape->SetBigTextureSize(size);
}

void LandscapeProxy::SetCursorScale(float32 scale)
{
	customLandscape->SetCursorScale(scale);
	baseLandscape->SetCursorScale(scale);
}

void LandscapeProxy::SetCursorPosition(const Vector2& position)
{
	customLandscape->SetCursorPosition(position);
	baseLandscape->SetCursorPosition(position);
}

void LandscapeProxy::UpdateFullTiledTexture(bool force)
{
	if (force || mode == MODE_CUSTOM_LANDSCAPE)
	{
		uint32 state = RenderManager::Instance()->GetState();
		baseLandscape->UpdateFullTiledTexture();
		RenderManager::Instance()->SetState(state);

		UpdateDisplayedTexture();
	}
}

Vector3 LandscapeProxy::PlacePoint(const Vector3& point)
{
	Vector3 landscapePoint;
	bool res = baseLandscape->PlacePoint(point, landscapePoint);
	return landscapePoint;
}
