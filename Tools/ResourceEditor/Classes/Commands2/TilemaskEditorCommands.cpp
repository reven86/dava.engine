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



#include "TilemaskEditorCommands.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ActionEnableTilemaskEditor::ActionEnableTilemaskEditor(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_TILEMASK_EDITOR_ENABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionEnableTilemaskEditor::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool enabled = sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled();
	if (enabled)
	{
		return;
	}
	
	sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	if (!success )
	{
		ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->tilemaskEditorSystem->EnableLandscapeEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
	}
	
	SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}

ActionDisableTilemaskEditor::ActionDisableTilemaskEditor(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_TILEMASK_EDITOR_DISABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionDisableTilemaskEditor::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool disabled = !sceneEditor->tilemaskEditorSystem->IsLandscapeEditingEnabled();
	if (disabled)
	{
		return;
	}
	
	disabled = sceneEditor->tilemaskEditorSystem->DisableLandscapeEdititing();
	if (!disabled)
	{
		ShowErrorDialog(ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR);
	}
	
	SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}


ModifyTilemaskCommand::ModifyTilemaskCommand(LandscapeProxy* landscapeProxy, const Rect& updatedRect)
:	Command2(CMDID_TILEMASK_MODIFY, "Tile Mask Modification")
{
    RenderManager::Instance()->SetColor(Color::White);
    
	this->updatedRect = updatedRect;
	this->landscapeProxy = SafeRetain(landscapeProxy);
	
	const DAVA::RenderStateData& default2dState = DAVA::RenderManager::Instance()->GetRenderStateData(DAVA::RenderState::RENDERSTATE_2D_BLEND);
	DAVA::RenderStateData noBlendStateData;
	memcpy(&noBlendStateData, &default2dState, sizeof(noBlendStateData));
	
	noBlendStateData.sourceFactor = DAVA::BLEND_ONE;
	noBlendStateData.destFactor = DAVA::BLEND_ZERO;
	
	noBlendDrawState = DAVA::RenderManager::Instance()->CreateRenderState(noBlendStateData);

	Image* originalMask = landscapeProxy->GetTilemaskImageCopy();

	undoImageMask = Image::CopyImageRegion(originalMask, updatedRect);

    RenderManager::Instance()->SetColor(Color::White);
	Image* currentImageMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory(noBlendDrawState);

	redoImageMask = Image::CopyImageRegion(currentImageMask, updatedRect);
	SafeRelease(currentImageMask);
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
	SafeRelease(undoImageMask);
	SafeRelease(redoImageMask);
	SafeRelease(landscapeProxy);

	RenderManager::Instance()->ReleaseRenderState(noBlendDrawState);
}

void ModifyTilemaskCommand::Undo()
{
    RenderManager::Instance()->Setup2DMatrices();
    
    RenderManager::Instance()->SetColor(Color::White);
    
    Sprite* srcSprite = landscapeProxy->GetTilemaskSprite(LandscapeProxy::TILEMASK_SPRITE_SOURCE);
	ApplyImageToSprite(undoImageMask, srcSprite);
    
	Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

	Sprite* sprite;
	sprite = ApplyImageToTexture(undoImageMask, maskTexture);
    
	landscapeProxy->SetTilemaskTexture(sprite->GetTexture());
	SafeRelease(sprite);

	landscapeProxy->UpdateFullTiledTexture();
	landscapeProxy->DecreaseTilemaskChanges();

	Rect r = Rect(Vector2(0, 0), Vector2(undoImageMask->GetWidth(), undoImageMask->GetHeight()));
	Image* mask = landscapeProxy->GetTilemaskImageCopy();
	mask->InsertImage(undoImageMask, updatedRect.GetPosition(), r);
}

void ModifyTilemaskCommand::Redo()
{
    RenderManager::Instance()->Setup2DMatrices();
    
	ApplyImageToSprite(redoImageMask, landscapeProxy->GetTilemaskSprite(LandscapeProxy::TILEMASK_SPRITE_SOURCE));

	Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);
    
	Sprite* sprite = NULL;
	sprite = ApplyImageToTexture(redoImageMask, maskTexture);
	landscapeProxy->SetTilemaskTexture(sprite->GetTexture());
	SafeRelease(sprite);

	landscapeProxy->UpdateFullTiledTexture();
	landscapeProxy->IncreaseTilemaskChanges();

	Rect r = Rect(Vector2(0, 0), Vector2(redoImageMask->GetWidth(), redoImageMask->GetHeight()));
	Image* mask = landscapeProxy->GetTilemaskImageCopy();
	mask->InsertImage(redoImageMask, updatedRect.GetPosition(), r);
}

Entity* ModifyTilemaskCommand::GetEntity() const
{
	return NULL;
}

Sprite* ModifyTilemaskCommand::ApplyImageToTexture(DAVA::Image *image, DAVA::Texture *texture)
{
	int32 width = texture->GetWidth();
	int32 height = texture->GetHeight();
    
	Sprite* resSprite = Sprite::CreateAsRenderTarget((float32)width, (float32)height, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(resSprite);
    
    RenderManager::Instance()->SetColor(Color::White);
    
	Sprite* s = Sprite::CreateFromTexture(texture, 0, 0, (float32)width, (float32)height);
    
    Sprite::DrawState drawState;
    drawState.SetRenderState(noBlendDrawState);
	drawState.SetPosition(0.f, 0.f);
	s->Draw(&drawState);
	SafeRelease(s);
    
    RenderManager::Instance()->Reset();
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->SetClip(updatedRect);
    
    RenderManager::Instance()->SetColor(Color::White);
    RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);

	Texture* t = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
										 image->GetWidth(), image->GetHeight(), false);
	s = Sprite::CreateFromTexture(t, 0, 0, (float32)t->GetWidth(), (float32)t->GetHeight());
    
    drawState.Reset();
	drawState.SetPosition(updatedRect.x, updatedRect.y);
    drawState.SetRenderState(noBlendDrawState);
	s->Draw(&drawState);
    
	SafeRelease(s);
	SafeRelease(t);
    
	RenderManager::Instance()->ClipPop();
	
    RenderManager::Instance()->RestoreRenderTarget();
    
	return resSprite;
}

void ModifyTilemaskCommand::ApplyImageToSprite(Image* image, Sprite* dstSprite)
{
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->SetClip(updatedRect);

    RenderManager::Instance()->SetColor(Color::White);
	
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	Sprite* srcSprite = Sprite::CreateFromTexture(texture, 0, 0, image->GetWidth(), image->GetHeight());
	
    Sprite::DrawState drawState;
    drawState.SetRenderState(noBlendDrawState);
	drawState.SetPosition(updatedRect.x, updatedRect.y);
	srcSprite->Draw(&drawState);
    
	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(texture);
	SafeRelease(srcSprite);
}


SetTileColorCommand::SetTileColorCommand(LandscapeProxy* landscapeProxy,
										 Landscape::eTextureLevel level,
										 const Color& color)
:	Command2(CMDID_SET_TILE_COLOR, "Set tile color")
,	level(level)
,	redoColor(color)
{
	this->landscapeProxy = SafeRetain(landscapeProxy);
	undoColor = landscapeProxy->GetLandscapeTileColor(level);
}

SetTileColorCommand::~SetTileColorCommand()
{
	SafeRelease(landscapeProxy);
}

void SetTileColorCommand::Undo()
{
	landscapeProxy->SetLandscapeTileColor(level, undoColor);
}

void SetTileColorCommand::Redo()
{
	landscapeProxy->SetLandscapeTileColor(level, redoColor);
}

Entity* SetTileColorCommand::GetEntity() const
{
	return NULL;
}
