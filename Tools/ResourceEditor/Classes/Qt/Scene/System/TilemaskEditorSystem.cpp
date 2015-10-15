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

#include <QApplication>

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
:	LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
,	curToolSize(0)
,	toolImage(NULL)
,	toolImageTexture(NULL)
,	tileTextureNum(0)
,	drawingType(TILEMASK_DRAW_NORMAL)
,	strength(0.25f)
,	toolImagePath("")
,	toolImageIndex(0)
,	copyPasteFrom(-1.f, -1.f)
,	copyPasteTo(-1.f, -1.f)
,	editingIsEnabled(false)
,	stencilTexture(NULL)
,	toolTexture(NULL)
,	toolSpriteUpdated(false)
,	needCreateUndo(false)
,	textureLevel(Landscape::TEXTURE_TILE_MASK)
{
    cursorSize = 120;
    
    tileMaskEditorShader = SafeRetain(ShaderCache::Instance()->Get(FastName("~res:/Materials/Shaders/Landscape/tilemask-editor"), FastNameSet()));
    tileMaskCopyPasteShader = SafeRetain(ShaderCache::Instance()->Get(FastName("~res:/Materials/Shaders/Landscape/tilemask-editor-copypaste"), FastNameSet()));

    spriteRenderObject = new RenderDataObject();
    spriteVertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    spriteTexCoordStream = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
	SafeRelease(tileMaskEditorShader);
	SafeRelease(tileMaskCopyPasteShader);
	SafeRelease(toolImage);
	SafeRelease(toolImageTexture);
	SafeRelease(toolTexture);
	SafeRelease(stencilTexture);
    SafeRelease(spriteRenderObject);
}

LandscapeEditorDrawSystem::eErrorType TilemaskEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}
	
	LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
	if ( canBeEnabledError!= LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return canBeEnabledError;
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = drawSystem->EnableTilemaskEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enablingError;
	}

	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	landscapeSize = drawSystem->GetTextureSize(textureLevel);
	copyPasteFrom = Vector2(-1.f, -1.f);

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);

	drawSystem->GetLandscapeProxy()->InitTilemaskImageCopy();
	
	InitSprites();

	Texture* srcSprite = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_SOURCE);
    Texture* dstSprite = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_DESTINATION);

	srcSprite->SetMinMagFilter(Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);
	dstSprite->SetMinMagFilter(Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	FinishEditing();

	drawSystem->GetLandscapeProxy()->UpdateFullTiledTexture(true);
	
	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
	drawSystem->DisableCursor();
	drawSystem->DisableTilemaskEditing();
	
	enabled = false;
	return !enabled;
}

void TilemaskEditorSystem::Process(float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		if (prevCursorPos != cursorPosition)
		{
			prevCursorPos = cursorPosition;

			Vector2 toolSize = Vector2(cursorSize, cursorSize);
			Vector2 toolPos = cursorPosition - toolSize / 2.f;
            Rect toolRect(toolPos, toolSize);

			if (activeDrawingType == TILEMASK_DRAW_NORMAL)
			{
                RenderHelper::Instance()->Set2DRenderTarget(toolTexture);
                RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
                RenderHelper::Instance()->DrawTexture(toolImageTexture, RenderState::RENDERSTATE_2D_OPAQUE, toolRect);

                RenderManager::Instance()->SetRenderTarget(0);

				toolSpriteUpdated = true;
			}
			else if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
			{
				if (copyPasteFrom == Vector2(-1.f, -1.f) || copyPasteTo == Vector2(-1.f, -1.f))
				{
					return;
				}

				Texture * dstTex = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_SOURCE);

				Vector2 posTo = toolPos;
				Vector2 deltaPos = cursorPosition - copyPasteTo;
				Vector2 posFrom = copyPasteFrom + deltaPos - toolSize / 2.f;

				Rect dstRect = Rect(posTo, toolSize);
				Vector2 spriteDeltaPos = dstRect.GetPosition() - posFrom;
                Rect textureRect = Rect(spriteDeltaPos, Vector2((float32)dstTex->GetWidth(), (float32)dstTex->GetHeight()));
				textureRect.ClampToRect(dstRect);
				if (dstRect.dx == 0.f || dstRect.dy == 0.f)
				{
					return;
				}


                RenderHelper::Instance()->Set2DRenderTarget(toolTexture);
                RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
                RenderManager::Instance()->SetClip(dstRect);
                RenderHelper::Instance()->DrawTexture(dstTex, RenderState::RENDERSTATE_2D_OPAQUE, textureRect);

                RenderManager::Instance()->SetClip(Rect(0.f, 0.f, -1.f, -1.f));

                RenderHelper::Instance()->Set2DRenderTarget(stencilTexture);
                RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
                RenderHelper::Instance()->DrawTexture(toolImageTexture, RenderState::RENDERSTATE_2D_OPAQUE, toolRect);
                
                RenderManager::Instance()->SetRenderTarget(0);

				toolSpriteUpdated = true;
			}

            RenderSystem2D::Instance()->Setup2DMatrices();
            
			AddRectToAccumulator(toolRect);
		}
	}
}

void TilemaskEditorSystem::Input(UIEvent* event)
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
        case UIEvent::Phase::BEGAN:
                if (isIntersectsLandscape && !needCreateUndo)
				{
					if (drawingType == TILEMASK_DRAW_COPY_PASTE)
					{
						int32 curKeyModifiers = QApplication::keyboardModifiers();
						if (curKeyModifiers & Qt::AltModifier)
						{
							copyPasteFrom = cursorPosition;
							copyPasteTo = Vector2(-1.f, -1.f);
							return;
						}
						else
						{
							if (copyPasteFrom == Vector2(-1.f, -1.f))
							{
								return;
							}
							copyPasteTo = cursorPosition;
						}
					}

					UpdateToolImage();
					ResetAccumulatorRect();
					editingIsEnabled = true;
					activeDrawingType = drawingType;
				}
				break;

        case UIEvent::Phase::DRAG:
                break;

        case UIEvent::Phase::ENDED:
                FinishEditing();
				break;
		}
	}
}

void TilemaskEditorSystem::FinishEditing()
{
	if (editingIsEnabled)
	{
		needCreateUndo = true;
		editingIsEnabled = false;
	}
	prevCursorPos = Vector2(-1.f, -1.f);
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
	Texture* srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_SOURCE);
    Texture* dstTexture = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_DESTINATION);

    RenderHelper::Instance()->Set2DRenderTarget(dstTexture);
    RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
    
	Shader* shader = tileMaskEditorShader;
	if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
	{
		shader = tileMaskCopyPasteShader;
	}

    spriteTempVertices[0] = spriteTempVertices[4] = 0.f;
    spriteTempVertices[5] = spriteTempVertices[7] = 0.f;
    spriteTempVertices[1] = spriteTempVertices[3] = (float32)srcTexture->GetHeight();
    spriteTempVertices[2] = spriteTempVertices[6] = (float32)srcTexture->GetWidth();
    
    spriteTempCoords[0] = spriteTempCoords[4] = 0.f;
    spriteTempCoords[5] = spriteTempCoords[7] = 0.f;
    spriteTempCoords[1] = spriteTempCoords[3] = 1.f;
    spriteTempCoords[2] = spriteTempCoords[6] = 1.f;

    spriteVertexStream->Set(TYPE_FLOAT, 2, 0, spriteTempVertices);
    spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, spriteTempCoords);

	TextureStateData textureStateData;
	textureStateData.SetTexture(0, srcTexture);
	textureStateData.SetTexture(1, toolTexture);
	if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
	{
		textureStateData.SetTexture(2, stencilTexture);
	}
	UniqueHandle textureState = RenderManager::Instance()->CreateTextureState(textureStateData);

    RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->SetRenderData(spriteRenderObject);
    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_OPAQUE);
	RenderManager::Instance()->SetTextureState(textureState);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	
	int32 tex0 = shader->FindUniformIndexByName(DAVA::FastName("texture0"));
	shader->SetUniformValueByIndex(tex0, 0);
	int32 tex1 = shader->FindUniformIndexByName(DAVA::FastName("texture1"));
	shader->SetUniformValueByIndex(tex1, 1);

	if (activeDrawingType == TILEMASK_DRAW_NORMAL)
	{
		int32 colorType = (int32)tileTextureNum;
		int32 colorTypeUniform = shader->FindUniformIndexByName(DAVA::FastName("colorType"));
		shader->SetUniformValueByIndex(colorTypeUniform, colorType);
		int32 intensityUniform = shader->FindUniformIndexByName(DAVA::FastName("intensity"));
		shader->SetUniformValueByIndex(intensityUniform, strength);
	}
	else if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
	{
		int32 tex2 = shader->FindUniformIndexByName(DAVA::FastName("texture2"));
		shader->SetUniformValueByIndex(tex2, 2);
	}

	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    
	RenderManager::Instance()->SetColor(Color::White);

	RenderManager::Instance()->ReleaseTextureState(textureState);

	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(dstTexture);
	drawSystem->GetLandscapeProxy()->SwapTilemaskSprites();

    RenderManager::Instance()->SetRenderTarget(0);
}

Image* TilemaskEditorSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	Texture * dstTex = Texture::CreateFBO(sideSize, sideSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);
	Texture * srcTex = Texture::CreateFromFile(filePath);

    RenderHelper::Instance()->Set2DRenderTarget(dstTex);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.0f);
	RenderManager::Instance()->SetColor(Color::White);
    RenderHelper::Instance()->DrawTexture(srcTex, RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetRenderTarget(0);
	
	SafeRelease(toolImageTexture);
    toolImageTexture = SafeRetain(dstTex);
    Image * retImage = dstTex->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
	
	SafeRelease(srcTex);
    SafeRelease(dstTex);
	
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
	Rect r = updatedRectAccumulator;
	drawSystem->ClampToTexture(textureLevel, r);

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

Color TilemaskEditorSystem::GetTileColor(int32 index)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return Color::Black;
	}

	Landscape::eTextureLevel level = (Landscape::eTextureLevel)(Landscape::TEXTURE_TILE0 + index);
	Color color = drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(level);
	
	return color;
}

void TilemaskEditorSystem::SetTileColor(int32 index, const Color& color)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return;
	}

	Landscape::eTextureLevel level = (Landscape::eTextureLevel)(Landscape::TEXTURE_TILE0 + index);
	Color curColor = drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(level);

	if (curColor != color)
	{
		SceneEditor2* scene = (SceneEditor2*)(GetScene());
		scene->Exec(new SetTileColorCommand(drawSystem->GetLandscapeProxy(), level, color));
	}
}

MetaObjModifyCommand* TilemaskEditorSystem::CreateTileColorCommand(Landscape::eTextureLevel level, const Color& color)
{
	Landscape* landscape = drawSystem->GetBaseLandscape();
	const InspMember* inspMember = landscape->GetTypeInfo()->Member(FastName("tileColor"));
	const InspColl* inspColl = inspMember->Collection();
	void* object = inspMember->Data(landscape);

	InspColl::Iterator it = inspColl->Begin(object);

	int32 i = level;
	while (i--)
	{
		it = inspColl->Next(it);
	}

	return new MetaObjModifyCommand(inspColl->ItemType(), inspColl->ItemPointer(it), VariantType(color));
}

void TilemaskEditorSystem::CreateMaskTexture()
{
	CreateMaskFromTexture(drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK));
}

void TilemaskEditorSystem::CreateMaskFromTexture(Texture* texture)
{
    Texture* tilemaskTexture = drawSystem->GetLandscapeProxy()->GetTilemaskTexture(LandscapeProxy::TILEMASK_SPRITE_SOURCE);

	if(texture)
	{   
        RenderHelper::Instance()->Set2DRenderTarget(tilemaskTexture);
        RenderHelper::Instance()->DrawTexture(texture, RenderState::RENDERSTATE_2D_OPAQUE);
        RenderManager::Instance()->SetRenderTarget(0);
	}
	
    tilemaskTexture->GenerateMipmaps();
    drawSystem->GetLandscapeProxy()->SetTilemaskTexture(tilemaskTexture);
}

void TilemaskEditorSystem::Draw()
{
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    if (!IsLandscapeEditingEnabled())
	{
		return;
	}

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
    RenderManager::Instance()->SetViewport(oldViewport);
}

void TilemaskEditorSystem::CreateUndoPoint()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyTilemaskCommand(drawSystem->GetLandscapeProxy(), GetUpdatedRect()));
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

void TilemaskEditorSystem::InitSprites()
{
	float32 texSize = drawSystem->GetTextureSize(textureLevel);

	if (toolTexture == NULL)
	{
		toolTexture = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);
	}

	if (stencilTexture == NULL)
	{
        stencilTexture = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);
	}

	drawSystem->GetLandscapeProxy()->InitTilemaskSprites();
	CreateMaskTexture();
}

void TilemaskEditorSystem::SetDrawingType(eTilemaskDrawType type)
{
	if (type >= TILEMASK_DRAW_NORMAL && type < TILEMASK_DRAW_TYPES_COUNT)
	{
		drawingType = type;
	}
}

TilemaskEditorSystem::eTilemaskDrawType TilemaskEditorSystem::GetDrawingType()
{
	return drawingType;
}
