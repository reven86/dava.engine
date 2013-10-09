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



#include "TilemaskEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "../SceneEditor2.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Commands2/TilemaskEditorCommands.h"

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(120)
,	toolImage(NULL)
,	toolImageSprite(NULL)
,	strength(0.25f)
,	toolImagePath("")
,	tileTextureNum(0)
,	maskSprite(NULL)
,	oldMaskSprite(NULL)
,	toolSprite(NULL)
,	prevCursorPos(Vector2(-1.f, -1.f))
,	toolSpriteUpdated(false)
,	needCreateUndo(false)
,	originalMask(NULL)
,	toolImageIndex(0)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.png");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
	
    tileMaskEditorShader = new Shader();
	tileMaskEditorShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask-editor.shader");
	tileMaskEditorShader->Recompile();
	
	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
	SafeRelease(cursorTexture);
	SafeRelease(tileMaskEditorShader);
	SafeRelease(toolImage);
	SafeRelease(toolImageSprite);
}

bool TilemaskEditorSystem::IsCanBeEnabled()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	
	bool canBeEnabled = true;
	canBeEnabled &= !(scene->visibilityToolSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->heightmapEditorSystem->IsLandscapeEditingEnabled());
//	canBeEnabled &= !(scene->tilemaskEditorSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->rulerToolSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->customColorsSystem->IsLandscapeEditingEnabled());
	canBeEnabled &= !(scene->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled());
	
	return canBeEnabled;
}

bool TilemaskEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return true;
	}

	if (!IsCanBeEnabled())
	{
		return false;
	}

	if (!drawSystem->EnableTilemaskEditing())
	{
		return false;
	}

	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	landscapeSize = drawSystem->GetTextureSize();

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);
	
	CreateMaskTexture();
	
	enabled = true;
	return enabled;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	drawSystem->GetLandscapeProxy()->UpdateFullTiledTexture(true);
	
	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
	drawSystem->DisableCursor();
	drawSystem->DisableTilemaskEditing();
	
	enabled = false;
	return !enabled;
}

bool TilemaskEditorSystem::IsLandscapeEditingEnabled() const
{
	return enabled;
}

void TilemaskEditorSystem::Update(float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		if (prevCursorPos != cursorPosition)
		{
			Vector2 toolSize = Vector2(cursorSize, cursorSize);
			Vector2 toolPos = cursorPosition - toolSize / 2.f;

			RenderManager::Instance()->SetRenderTarget(toolSprite);
			toolImageSprite->SetScaleSize(toolSize.x, toolSize.y);
			toolImageSprite->SetPosition(toolPos.x, toolPos.y);
			toolImageSprite->Draw();
			RenderManager::Instance()->RestoreRenderTarget();

			toolSpriteUpdated = true;

			prevCursorPos = cursorPosition;

			Rect rect(toolPos, toolSize);
			AddRectToAccumulator(rect);
		}
	}
}

void TilemaskEditorSystem::ProcessUIEvent(UIEvent* event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	UpdateCursorPosition();
	
	if (event->tid == UIEvent::BUTTON_1)
	{
		Vector3 point;
		
		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
				if (isIntersectsLandscape)
				{
					UpdateToolImage();
					ResetAccumulatorRect();
					StoreOriginalState();
					editingIsEnabled = true;
				}
				break;
				
			case UIEvent::PHASE_DRAG:
				break;
				
			case UIEvent::PHASE_ENDED:
				if (editingIsEnabled)
				{
					needCreateUndo = true;
					editingIsEnabled = false;
				}
				break;
		}
	}
}

void TilemaskEditorSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;
		drawSystem->SetCursorSize(cursorSize);
	}
}

void TilemaskEditorSystem::SetStrength(float32 strength)
{
	if (strength >= 0)
	{
		this->strength = strength;
	}
}

void TilemaskEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
	this->toolImagePath = toolImagePath;
	this->toolImageIndex = index;
	UpdateToolImage(true);
}

void TilemaskEditorSystem::SetTileTexture(uint32 tileTexture)
{
	if (tileTexture >= GetTileTextureCount())
	{
		return;
	}
	
	tileTextureNum = tileTexture;
}

void TilemaskEditorSystem::UpdateCursorPosition()
{
	Vector3 landPos;
	isIntersectsLandscape = false;
	if (collisionSystem->LandRayTestFromCamera(landPos))
	{
		isIntersectsLandscape = true;
		Vector2 point(landPos.x, landPos.y);
		
		point.x = (float32)((int32)point.x);
		point.y = (float32)((int32)point.y);
		
		AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
		
		cursorPosition.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
		cursorPosition.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
		cursorPosition.x = (int32)cursorPosition.x;
		cursorPosition.y = (int32)cursorPosition.y;
		
		drawSystem->SetCursorPosition(cursorPosition);
	}
}

void TilemaskEditorSystem::UpdateToolImage(bool force)
{
	if (toolImage)
	{
		if (curToolSize != cursorSize || force)
		{
			SafeRelease(toolImage);
		}
	}
	
	if (!toolImage)
	{
		if (!toolImagePath.IsEmpty())
		{
			toolImage = CreateToolImage(32, toolImagePath);
			curToolSize = cursorSize;
		}
	}
}

void TilemaskEditorSystem::UpdateBrushTool()
{
	int32 colorType = (int32)tileTextureNum;
	
	RenderManager::Instance()->SetRenderTarget(maskSprite);
	
	srcBlendMode = RenderManager::Instance()->GetSrcBlend();
	dstBlendMode = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	
	RenderManager::Instance()->SetShader(tileMaskEditorShader);
	oldMaskSprite->PrepareSpriteRenderData(0);
	RenderManager::Instance()->SetRenderData(oldMaskSprite->spriteRenderObject);
	RenderManager::Instance()->SetTexture(oldMaskSprite->GetTexture(), 0);
	RenderManager::Instance()->SetTexture(toolSprite->GetTexture(), 1);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	
	int32 tex0 = tileMaskEditorShader->FindUniformIndexByName("texture0");
	tileMaskEditorShader->SetUniformValueByIndex(tex0, 0);
	int32 tex1 = tileMaskEditorShader->FindUniformIndexByName("texture1");
	tileMaskEditorShader->SetUniformValueByIndex(tex1, 1);
	int32 colorTypeUniform = tileMaskEditorShader->FindUniformIndexByName("colorType");
	tileMaskEditorShader->SetUniformValueByIndex(colorTypeUniform, colorType);
	int32 intensityUniform = tileMaskEditorShader->FindUniformIndexByName("intensity");
	
	tileMaskEditorShader->SetUniformValueByIndex(intensityUniform, strength);
	
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
	
	RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
	RenderManager::Instance()->RestoreRenderTarget();
	RenderManager::Instance()->SetColor(Color::White());
	
	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(maskSprite->GetTexture());
	Sprite * temp = oldMaskSprite;
	oldMaskSprite = maskSprite;
	maskSprite = temp;
	
	RenderManager::Instance()->SetRenderTarget(toolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	RenderManager::Instance()->RestoreRenderTarget();
}

Image* TilemaskEditorSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	RenderManager::Instance()->LockNonMain();
	
	Sprite *dstSprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);
	Texture *srcTex = Texture::CreateFromFile(filePath);
	Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)srcTex->GetWidth(), (float32)srcTex->GetHeight());
	
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	RenderManager::Instance()->SetColor(Color::White());
	
	srcSprite->SetScaleSize((float32)sideSize, (float32)sideSize);
	srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0f,
								   (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0f));
	srcSprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(toolImageSprite);
	toolImageSprite = SafeRetain(dstSprite);
	Image *retImage = dstSprite->GetTexture()->CreateImageFromMemory();
	
	SafeRelease(srcSprite);
	SafeRelease(srcTex);
	SafeRelease(dstSprite);
	
	RenderManager::Instance()->UnlockNonMain();
	
	return retImage;
}

void TilemaskEditorSystem::AddRectToAccumulator(const Rect& rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

void TilemaskEditorSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

Rect TilemaskEditorSystem::GetUpdatedRect()
{
	int32 textureSize = drawSystem->GetTextureSize();
	Rect r = updatedRectAccumulator;

	r.x = (float32)Clamp((int32)updatedRectAccumulator.x, 0, textureSize - 1);
	r.y = (float32)Clamp((int32)updatedRectAccumulator.y, 0, textureSize - 1);
	r.dx = Clamp((updatedRectAccumulator.x + updatedRectAccumulator.dx),
				 0.f, (float32)textureSize - 1.f) - updatedRectAccumulator.x;
	r.dy = Clamp((updatedRectAccumulator.y + updatedRectAccumulator.dy),
				 0.f, (float32)textureSize - 1.f) - updatedRectAccumulator.y;

	return r;
}

uint32 TilemaskEditorSystem::GetTileTextureCount() const
{
	return 4;
}

Texture* TilemaskEditorSystem::GetTileTexture(int32 index)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return NULL;
	}
	
	Landscape::eTextureLevel level = (Landscape::eTextureLevel)(Landscape::TEXTURE_TILE0 + index);
	Texture* texture = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(level);
	
	return texture;
}

void TilemaskEditorSystem::CreateMaskTexture()
{
	CreateMaskFromTexture(drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK));
}

void TilemaskEditorSystem::CreateMaskFromTexture(Texture* texture)
{
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
    SafeRelease(toolSprite);
	
	float32 texSize = 1024;
	if(texture)
	{
		texSize = (float32)texture->GetWidth();
	}
    
	maskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	oldMaskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	toolSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	
	if(texture)
	{
		RenderManager::Instance()->LockNonMain();
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
		
		Sprite *oldMask = Sprite::CreateFromTexture(texture, 0, 0,
													(float32)texture->GetWidth(), (float32)texture->GetHeight());
		
		RenderManager::Instance()->SetRenderTarget(oldMaskSprite);
		oldMask->SetPosition(0.f, 0.f);
		oldMask->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		
		RenderManager::Instance()->SetRenderTarget(maskSprite);
		oldMask->SetPosition(0.f, 0.f);
		oldMask->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		
		SafeRelease(oldMask);
		
		RenderManager::Instance()->UnlockNonMain();
	}
	
	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(oldMaskSprite->GetTexture());
}

void TilemaskEditorSystem::Draw()
{
	if (toolSpriteUpdated)
	{
		UpdateBrushTool();
		toolSpriteUpdated = false;
	}

	if (needCreateUndo)
	{
		CreateUndoPoint();
		needCreateUndo = false;
	}
}

void TilemaskEditorSystem::CreateUndoPoint()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyTilemaskCommand(originalMask, drawSystem->GetLandscapeProxy(), GetUpdatedRect()));

	SafeRelease(originalMask);
}

void TilemaskEditorSystem::StoreOriginalState()
{
	DVASSERT(originalMask == NULL);

	LandscapeProxy* lp = drawSystem->GetLandscapeProxy();
	originalMask = lp->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory();
}

int32 TilemaskEditorSystem::GetBrushSize()
{
	return cursorSize;
}

float32 TilemaskEditorSystem::GetStrength()
{
	return strength;
}

int32 TilemaskEditorSystem::GetToolImage()
{
	return toolImageIndex;
}

uint32 TilemaskEditorSystem::GetTileTextureIndex()
{
	return tileTextureNum;
}
