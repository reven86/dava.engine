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
    
    if(success &&
       LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
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
    
    if(disabled)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }
	
	SceneSignals::Instance()->EmitTilemaskEditorToggled(sceneEditor);
}


ModifyTilemaskCommand::ModifyTilemaskCommand(LandscapeProxy* _landscapeProxy, const Rect& _updatedRect)
:	Command2(CMDID_TILEMASK_MODIFY, "Tile Mask Modification")
{
	updatedRect = Rect(floorf(_updatedRect.x), floorf(_updatedRect.y), ceilf(_updatedRect.dx), ceilf(_updatedRect.dy));
	landscapeProxy = SafeRetain(_landscapeProxy);
 
    texture[0] = texture[1] = nullptr;
    
	Image* originalMask = landscapeProxy->GetTilemaskImageCopy();
    
	undoImageMask = Image::CopyImageRegion(originalMask, updatedRect);
    
    Image* currentImageMask = landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK)->CreateImageFromMemory();
	redoImageMask = Image::CopyImageRegion(currentImageMask, updatedRect);
	SafeRelease(currentImageMask);
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
	SafeRelease(undoImageMask);
	SafeRelease(redoImageMask);
	SafeRelease(landscapeProxy);

    SafeRelease(texture[0]);
    SafeRelease(texture[1]);
}

void ModifyTilemaskCommand::Undo()
{
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE), 0);
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK), 1);

	landscapeProxy->DecreaseTilemaskChanges();

	Rect r = Rect(Vector2(0, 0), Vector2(undoImageMask->GetWidth(), undoImageMask->GetHeight()));
	Image* mask = landscapeProxy->GetTilemaskImageCopy();
	mask->InsertImage(undoImageMask, updatedRect.GetPosition(), r);

}

void ModifyTilemaskCommand::Redo()
{
	ApplyImageToTexture(redoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE), 0);
    ApplyImageToTexture(redoImageMask, landscapeProxy->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK), 1);

	landscapeProxy->IncreaseTilemaskChanges();

	Rect r = Rect(Vector2(0, 0), Vector2(redoImageMask->GetWidth(), redoImageMask->GetHeight()));
	Image* mask = landscapeProxy->GetTilemaskImageCopy();
	mask->InsertImage(redoImageMask, updatedRect.GetPosition(), r);
}

Entity* ModifyTilemaskCommand::GetEntity() const
{
	return NULL;
}

void ModifyTilemaskCommand::ApplyImageToTexture(Image* image, Texture * dstTex, int32 internalHandleIndex)
{
    SafeRelease(texture[internalHandleIndex]);

    texture[internalHandleIndex] = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
                                                           image->GetWidth(), image->GetHeight(), false);
    
    RenderSystem2D::Instance()->BeginRenderTargetPass(dstTex, false);
    RenderSystem2D::Instance()->DrawTexture(texture[internalHandleIndex], RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();
}


SetTileColorCommand::SetTileColorCommand(LandscapeProxy* landscapeProxy,
										 const FastName& level,
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
