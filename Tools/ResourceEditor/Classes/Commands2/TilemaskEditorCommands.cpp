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
	this->updatedRect = updatedRect;
	this->landscapeProxy = SafeRetain(landscapeProxy);

	Image* originalMask = landscapeProxy->GetTilemaskImageCopy();

	undoImageMask = Image::CopyImageRegion(originalMask, updatedRect);

	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	Image* currentImageMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory();
	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);

	redoImageMask = Image::CopyImageRegion(currentImageMask, updatedRect);
	SafeRelease(currentImageMask);
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
	SafeRelease(undoImageMask);
	SafeRelease(redoImageMask);
	SafeRelease(landscapeProxy);
}

void ModifyTilemaskCommand::Undo()
{
	ApplyImageToSprite(undoImageMask, landscapeProxy->GetTilemaskSprite(LandscapeProxy::TILEMASK_SPRITE_SOURCE));

	Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

	Sprite* sprite;
	sprite = ApplyImageToTexture(undoImageMask, maskTexture);
	sprite->GetTexture()->GenerateMipmaps();
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
	ApplyImageToSprite(redoImageMask, landscapeProxy->GetTilemaskSprite(LandscapeProxy::TILEMASK_SPRITE_SOURCE));

	Texture* maskTexture = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK);

	Sprite* sprite;
	sprite = ApplyImageToTexture(redoImageMask, maskTexture);
	sprite->GetTexture()->GenerateMipmaps();
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

void ModifyTilemaskCommand::ApplyImageToSprite(Image* image, Sprite* dstSprite)
{
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	RenderManager::Instance()->ClipPush();
	RenderManager::Instance()->SetClip(updatedRect);
	
	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	Sprite* srcSprite = Sprite::CreateFromTexture(texture, 0, 0, image->GetWidth(), image->GetHeight());
	
	srcSprite->SetPosition(updatedRect.x, updatedRect.y);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	srcSprite->Draw();

	dstSprite->GetTexture()->GenerateMipmaps();

	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	
	RenderManager::Instance()->ClipPop();
	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(texture);
	SafeRelease(srcSprite);
}
