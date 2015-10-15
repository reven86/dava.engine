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


#include "CustomColorsSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "Commands2/KeyedArchiveCommand.h"
#include "Commands2/CustomColorsCommands2.h"
#include "Scene/SceneSignals.h"
#include "Qt/Settings/SettingsManager.h"
#include "Deprecated/EditorConfig.h"
#include "Project/ProjectManager.h"

CustomColorsSystem::CustomColorsSystem(Scene* scene)
:	LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
,	curToolSize(0)
,	toolImageTexture(NULL)
,	drawColor(Color(0.f, 0.f, 0.f, 0.f))
,	colorIndex(0)
,	editingIsEnabled(false)
,	originalImage(NULL)
{
    cursorSize = 120;

    SetColor(colorIndex);
}

CustomColorsSystem::~CustomColorsSystem()
{
	SafeRelease(toolImageTexture);
}

LandscapeEditorDrawSystem::eErrorType CustomColorsSystem::EnableLandscapeEditing()
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

	LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
	if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}

    selectionSystem->SetLocked(true);
    modifSystem->SetLocked(true);
    landscapeSize = drawSystem->GetTextureSize(Landscape::TEXTURE_TILE_FULL);

	FilePath filePath = GetCurrentSaveFileName();
	if (!filePath.IsEmpty())
	{
        const bool isTextureLoaded = LoadTexture(filePath, false);
        drawSystem->GetCustomColorsProxy()->ResetLoadedState(isTextureLoaded);
	}
	else
	{
		drawSystem->GetCustomColorsProxy()->UpdateSpriteFromConfig();
	}

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);
	
	Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(customColorsTexture);
	drawSystem->GetLandscapeProxy()->SetCustomColorsTextureEnabled(true);
	
	if (!toolImageTexture)
	{
		CreateToolImage(512, "~res:/LandscapeEditor/Tools/customcolorsbrush/circle.tex");
	}
	
	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool CustomColorsSystem::ChangesPresent()
{
	if(drawSystem && drawSystem->GetCustomColorsProxy())
	{
		return drawSystem->GetCustomColorsProxy()->GetChangesCount() > 0;
	}
	return false;
}

bool CustomColorsSystem::DisableLandscapeEdititing( bool saveNeeded)
{
	if (!enabled)
	{
		return true;
	}
	
	if (drawSystem->GetCustomColorsProxy()->GetChangesCount() && saveNeeded)
	{	
		SceneSignals::Instance()->EmitCustomColorsTextureShouldBeSaved(((SceneEditor2 *) GetScene()));
	}
	FinishEditing();

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();
	
	drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(NULL);
	drawSystem->GetLandscapeProxy()->SetCustomColorsTextureEnabled(false);
	enabled = false;
	
	return !enabled;
}

void CustomColorsSystem::Process(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		if (prevCursorPos != cursorPosition)
		{
			UpdateBrushTool(timeElapsed);
			prevCursorPos = cursorPosition;
		}
	}
}

void CustomColorsSystem::Input(DAVA::UIEvent *event)
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
                if (isIntersectsLandscape)
				{
					UpdateToolImage();
					StoreOriginalState();
					editingIsEnabled = true;
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

void CustomColorsSystem::FinishEditing()
{
	if (editingIsEnabled)
	{
		CreateUndoPoint();
		editingIsEnabled = false;
	}
}


void CustomColorsSystem::UpdateToolImage(bool force)
{
}

Image* CustomColorsSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	Texture* toolTexture = Texture::CreateFromFile(filePath);
	if (!toolTexture)
	{
		return NULL;
	}
	
	SafeRelease(toolImageTexture);
    toolImageTexture = toolTexture;
	toolImageTexture->GeneratePixelesation();
	
	return NULL;
}

void CustomColorsSystem::UpdateBrushTool(float32 timeElapsed)
{
	Texture* colorTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
	
	Vector2 spriteSize = Vector2(cursorSize, cursorSize);
	Vector2 spritePos = cursorPosition - spriteSize / 2.f;

    Rect updatedRect;
    updatedRect.SetCenter(spritePos);
    updatedRect.SetSize(spriteSize);
    AddRectToAccumulator(updatedRect);

    RenderManager::Instance()->SetColor(drawColor);

    RenderHelper::Instance()->Set2DRenderTarget(colorTexture);
    RenderHelper::Instance()->DrawTexture(toolImageTexture, RenderState::RENDERSTATE_2D_BLEND, updatedRect);
	
    RenderManager::Instance()->SetRenderTarget(0);
	RenderManager::Instance()->SetColor(Color::White);
	
    drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(colorTexture);
}

void CustomColorsSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

void CustomColorsSystem::AddRectToAccumulator(const Rect &rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

Rect CustomColorsSystem::GetUpdatedRect()
{
	Rect r = updatedRectAccumulator;
	drawSystem->ClampToTexture(Landscape::TEXTURE_TILE_FULL, r);

	return r;
}

void CustomColorsSystem::SetBrushSize(int32 brushSize, bool updateDrawSystem /*= true*/)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;
		if(updateDrawSystem)
		{
			drawSystem->SetCursorSize(cursorSize);
		}
	}
}

void CustomColorsSystem::SetColor(int32 colorIndex)
{
	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	if (colorIndex >= 0 && colorIndex < (int32)customColors.size())
	{
		drawColor = customColors[colorIndex];
		this->colorIndex = colorIndex;
	}
}

void CustomColorsSystem::StoreOriginalState()
{
	DVASSERT(originalImage == NULL);
	originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
	ResetAccumulatorRect();
}

void CustomColorsSystem::CreateUndoPoint()
{
	Rect updatedRect = GetUpdatedRect();
	if (updatedRect.dx > 0 || updatedRect.dy > 0)
	{
		SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
		DVASSERT(scene);

		scene->Exec(new ModifyCustomColorsCommand(originalImage, drawSystem->GetCustomColorsProxy(), updatedRect));
	}

	SafeRelease(originalImage);
}

void CustomColorsSystem::SaveTexture(const DAVA::FilePath &filePath)
{
	if(filePath.IsEmpty())
		return;

    Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

	Image* image = customColorsTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    ImageSystem::Instance()->Save(filePath, image);
	SafeRelease(image);

	StoreSaveFileName(filePath);
	drawSystem->GetCustomColorsProxy()->ResetChanges();
}

bool CustomColorsSystem::LoadTexture( const DAVA::FilePath &filePath, bool createUndo /* = true */ )
{
	if(filePath.IsEmpty())
		return false;

    Vector<Image*> images;
    ImageSystem::Instance()->Load(filePath, images);
	if(images.empty())
		return false;

	Image* image = images.front();
	if(image)
	{
		Texture* texture = Texture::CreateFromData(image->GetPixelFormat(),
												   image->GetData(),
												   image->GetWidth(),
												   image->GetHeight(),
												   false);
		if (createUndo)
		{
			StoreOriginalState();
		}

        Texture * target = drawSystem->GetCustomColorsProxy()->GetTexture();

        RenderHelper::Instance()->Set2DRenderTarget(target);
        RenderHelper::Instance()->DrawTexture(texture, RenderState::RENDERSTATE_2D_BLEND);
        
        RenderManager::Instance()->SetRenderTarget(0);
		AddRectToAccumulator(Rect(Vector2(0.f, 0.f), Vector2(texture->GetWidth(), texture->GetHeight())));

		SafeRelease(texture);
		for_each(images.begin(), images.end(), SafeRelease<Image>);

		if (createUndo)
		{
			((SceneEditor2*)GetScene())->BeginBatch("Load custom colors texture");
			StoreSaveFileName(filePath);
			CreateUndoPoint();
			((SceneEditor2*)GetScene())->EndBatch();
		}
	}

    return true;
}

void CustomColorsSystem::StoreSaveFileName(const FilePath& filePath)
{
	Command2* command = CreateSaveFileNameCommand(GetRelativePathToProjectPath(filePath));
	if (command)
	{
		((SceneEditor2*)GetScene())->Exec(command);
	}
}

Command2* CustomColorsSystem::CreateSaveFileNameCommand(const String& filePath)
{
	KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
	bool keyExists = customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);

	Command2* command = NULL;
	if (keyExists)
	{
		String curPath = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
		if (curPath != filePath)
		{
			command = new KeyeadArchiveSetValueCommand(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP,
													   VariantType(filePath));
		}
	}
	else
	{
		command = new KeyedArchiveAddValueCommand(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP,
												  VariantType(filePath));
	}

	return command;
}

FilePath CustomColorsSystem::GetCurrentSaveFileName()
{
	String currentSaveName;

	KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
	if (customProps && customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
	{
		currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
	}

	return GetAbsolutePathFromProjectPath(currentSaveName);
}

FilePath CustomColorsSystem::GetScenePath()
{
	return ((SceneEditor2 *) GetScene())->GetScenePath().GetDirectory();
}

String CustomColorsSystem::GetRelativePathToScenePath(const FilePath &absolutePath)
{
	if(absolutePath.IsEmpty())
		return String();

	return absolutePath.GetRelativePathname(GetScenePath());
}

FilePath CustomColorsSystem::GetAbsolutePathFromScenePath(const String &relativePath)
{
	if(relativePath.empty())
		return FilePath();

	return (GetScenePath() + relativePath);
}

String CustomColorsSystem::GetRelativePathToProjectPath(const FilePath& absolutePath)
{
	if(absolutePath.IsEmpty())
		return String();

	return absolutePath.GetRelativePathname(ProjectManager::Instance()->CurProjectPath());
}

FilePath CustomColorsSystem::GetAbsolutePathFromProjectPath(const String& relativePath)
{
	if(relativePath.empty())
		return FilePath();
	
	return ProjectManager::Instance()->CurProjectPath() + relativePath;
}

int32 CustomColorsSystem::GetBrushSize()
{
	return cursorSize;
}

int32 CustomColorsSystem::GetColor()
{
	return colorIndex;
}