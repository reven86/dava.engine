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



#include "HeightmapEditorCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"

ModifyHeightmapCommand::ModifyHeightmapCommand(HeightmapProxy* heightmapProxy,
											   Heightmap* originalHeightmap,
											   const Rect& updatedRect)
:	Command2(CMDID_DRAW_HEIGHTMAP, "Height Map Change")
,	heightmapProxy(heightmapProxy)
{
	if (originalHeightmap && heightmapProxy)
	{
		this->updatedRect = updatedRect;
		undoRegion = GetHeightmapRegion(originalHeightmap);
		redoRegion = GetHeightmapRegion(heightmapProxy);
	}
}

ModifyHeightmapCommand::~ModifyHeightmapCommand()
{
	SafeDeleteArray(undoRegion);
	SafeDeleteArray(redoRegion);
}

Entity* ModifyHeightmapCommand::GetEntity() const
{
	return NULL;
}

void ModifyHeightmapCommand::Redo()
{
	ApplyHeightmapRegion(redoRegion);
}

void ModifyHeightmapCommand::Undo()
{
	ApplyHeightmapRegion(undoRegion);
}

uint16* ModifyHeightmapCommand::GetHeightmapRegion(Heightmap* heightmap)
{
	int32 size = heightmap->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;
	
	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);
	
	uint16* newData = new uint16[width * height];
	uint16* oldData = heightmap->Data();
	
	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = oldData + (yOffset + i) * size + xOffset;
		uint16* dst = newData + i * width;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	return newData;
}

void ModifyHeightmapCommand::ApplyHeightmapRegion(uint16* region)
{
	int32 size = heightmapProxy->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;
	
	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);
	
	uint16* data = heightmapProxy->Data();
	
	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = region + i * width;
		uint16* dst = data + (yOffset + i) * size + xOffset;
		memcpy(dst, src, sizeof(uint16) * width);
	}
	
	heightmapProxy->UpdateRect(updatedRect);
}


CopyPasteHeightmapCommand::CopyPasteHeightmapCommand(bool heightmapModified,
													 bool tilemaskModified,
													 HeightmapProxy* heightmapProxy,
													 Heightmap* originalHeightmap,
													 const Rect& heightmapUpdatedRect,
													 LandscapeProxy* landscapeProxy,
													 Image* originalTilemaskImage,
													 const Rect& tilemaskUpdatedRect)
:	Command2(CMDID_COPY_PASTE_HEIGHTMAP, "Height Map Copy/Paste")
,	heightmapProxy(heightmapProxy)
,	landscapeProxy(landscapeProxy)
,	heightmapModified(heightmapModified)
,	tilemaskModified(tilemaskModified)
,	undoRegion(NULL)
,	redoRegion(NULL)
,	undoImageMask(NULL)
,	redoImageMask(NULL)
,	heightmapUpdatedRect(heightmapUpdatedRect)
,	tilemaskUpdatedRect(tilemaskUpdatedRect)
,	firstRun(true)
{
	if (heightmapModified)
	{
		undoRegion = GetHeightmapRegion(originalHeightmap, heightmapUpdatedRect);
		redoRegion = GetHeightmapRegion(heightmapProxy, heightmapUpdatedRect);
	}

	if (tilemaskModified)
	{
		undoImageMask = Image::CopyImageRegion(originalTilemaskImage, tilemaskUpdatedRect);

		eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
		Image* currentImageMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory();
		redoImageMask = Image::CopyImageRegion(currentImageMask, tilemaskUpdatedRect);
		SafeRelease(currentImageMask);
		RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	}
}

CopyPasteHeightmapCommand::~CopyPasteHeightmapCommand()
{
	SafeDeleteArray(undoRegion);
	SafeDeleteArray(redoRegion);
	SafeRelease(undoImageMask);
	SafeRelease(redoImageMask);
}

uint16* CopyPasteHeightmapCommand::GetHeightmapRegion(Heightmap* heightmap, const Rect& updatedRect)
{
	int32 size = heightmap->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;

	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

	uint16* newData = new uint16[width * height];
	uint16* oldData = heightmap->Data();

	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = oldData + (yOffset + i) * size + xOffset;
		uint16* dst = newData + i * width;
		memcpy(dst, src, sizeof(uint16) * width);
	}

	return newData;
}

void CopyPasteHeightmapCommand::ApplyHeightmapRegion(uint16* region, const Rect& updatedRect)
{
	int32 size = heightmapProxy->Size();
	int32 width = (int32)updatedRect.dx;
	int32 height = (int32)updatedRect.dy;
	int32 xOffset = (int32)updatedRect.x;
	int32 yOffset = (int32)updatedRect.y;

	DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

	uint16* data = heightmapProxy->Data();

	for (int32 i = 0; i < height; ++i)
	{
		uint16* src = region + i * width;
		uint16* dst = data + (yOffset + i) * size + xOffset;
		memcpy(dst, src, sizeof(uint16) * width);
	}

	heightmapProxy->UpdateRect(updatedRect);
}

Sprite* CopyPasteHeightmapCommand::ApplyImageToTexture(Image* image, Texture* texture, const Rect& updatedRect)
{
	int32 width = texture->GetWidth();
	int32 height = texture->GetHeight();
	
	Sprite* resSprite = Sprite::CreateAsRenderTarget((float32)width, (float32)height, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(resSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	
	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	
	Sprite* s = Sprite::CreateFromTexture(texture, 0, 0, (float32)width, (float32)height);
	s->SetPosition(0.f, 0.f);
	s->Draw();
	SafeRelease(s);
	
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->SetClip(updatedRect);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	
	Texture* t = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
										 image->GetWidth(), image->GetHeight(), false);
	s = Sprite::CreateFromTexture(t, 0, 0, (float32)t->GetWidth(), (float32)t->GetHeight());
	s->SetPosition(updatedRect.x, updatedRect.y);
	s->Draw();
	SafeRelease(s);
	SafeRelease(t);
	
	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	RenderManager::Instance()->ResetColor();
	RenderManager::Instance()->RestoreRenderTarget();
	
	return resSprite;
}

void CopyPasteHeightmapCommand::Redo()
{
	if (firstRun)
	{
		firstRun = false;
		return;
	}

	if (heightmapModified)
	{
		ApplyHeightmapRegion(redoRegion, heightmapUpdatedRect);
	}

	if (tilemaskModified)
	{
		Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

		Sprite* sprite;
		sprite = ApplyImageToTexture(redoImageMask, maskTexture, tilemaskUpdatedRect);
		landscapeProxy->SetTilemaskTexture(sprite->GetTexture());
		SafeRelease(sprite);

		landscapeProxy->UpdateFullTiledTexture();
	}
}

void CopyPasteHeightmapCommand::Undo()
{
	if (heightmapModified)
	{
		ApplyHeightmapRegion(undoRegion, heightmapUpdatedRect);
	}

	if (tilemaskModified)
	{
		Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

		Sprite* sprite;
		sprite = ApplyImageToTexture(undoImageMask, maskTexture, tilemaskUpdatedRect);
		landscapeProxy->SetTilemaskTexture(sprite->GetTexture());
		SafeRelease(sprite);

		landscapeProxy->UpdateFullTiledTexture();
	}
}

Entity* CopyPasteHeightmapCommand::GetEntity() const
{
	return NULL;
}
