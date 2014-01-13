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
#include "../SceneEditor2.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "../SceneEditor/EditorConfig.h"
#include "../../../Commands2/CustomColorsCommands2.h"
#include "../SceneSignals.h"
#include "SceneEditor/EditorSettings.h"

CustomColorsSystem::CustomColorsSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(120)
,	drawColor(Color(0.f, 0.f, 0.f, 0.f))
,	toolImageSprite(NULL)
,	prevCursorPos(Vector2(-1.f, -1.f))
,	originalImage(NULL)
,	colorIndex(0)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.tex");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	SetColor(colorIndex);
	
	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
}

CustomColorsSystem::~CustomColorsSystem()
{
	SafeRelease(cursorTexture);
	SafeRelease(toolImageSprite);
}

bool CustomColorsSystem::IsLandscapeEditingEnabled() const
{
	return enabled;
}

LandscapeEditorDrawSystem::eErrorType CustomColorsSystem::IsCanBeEnabled()
{
	return drawSystem->VerifyLandscape();
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

	landscapeSize = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	FilePath filePath = GetCurrentSaveFileName();
	if (!filePath.IsEmpty())
	{
		LoadTexture(filePath, false);
	}

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);
	
	Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetSprite()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(customColorsTexture);
	drawSystem->GetLandscapeProxy()->SetCustomColorsTextureEnabled(true);
	
	if (!toolImageSprite)
	{
		CreateToolImage(512, "~res:/LandscapeEditor/Tools/customcolorsbrush/circle.tex");
	}
	
	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool CustomColorsSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	FinishEditing();

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();
	
	drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(NULL);
	drawSystem->GetLandscapeProxy()->SetCustomColorsTextureEnabled(false);
	
	enabled = false;

	if (drawSystem->GetCustomColorsProxy()->GetChangesCount())
	{
		SceneSignals::Instance()->EmitCustomColorsTextureShouldBeSaved(((SceneEditor2 *) GetScene()));
	}

	return !enabled;
}

void CustomColorsSystem::Update(DAVA::float32 timeElapsed)
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

void CustomColorsSystem::ProcessUIEvent(DAVA::UIEvent *event)
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
					StoreOriginalState();
					editingIsEnabled = true;
				}
				break;
				
			case UIEvent::PHASE_DRAG:
				break;
				
			case UIEvent::PHASE_ENDED:
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

void CustomColorsSystem::UpdateCursorPosition()
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
	else
	{
		// hide cursor
		drawSystem->SetCursorPosition(DAVA::Vector2(-100, -100));
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
	
	SafeRelease(toolImageSprite);
	toolImageSprite = Sprite::CreateFromTexture(toolTexture, 0.f, 0.f, sideSize, sideSize);
	toolImageSprite->GetTexture()->GeneratePixelesation();
	
	SafeRelease(toolTexture);
	
	return NULL;
}

void CustomColorsSystem::UpdateBrushTool(float32 timeElapsed)
{
	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	
	Sprite* colorSprite = drawSystem->GetCustomColorsProxy()->GetSprite();
	
	RenderManager::Instance()->SetRenderTarget(colorSprite);
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	
	RenderManager::Instance()->SetColor(drawColor);

	Vector2 spriteSize = Vector2(cursorSize, cursorSize);
	Vector2 spritePos = cursorPosition - spriteSize / 2.f;
	
	toolImageSprite->SetScaleSize(spriteSize.x, spriteSize.y);
	toolImageSprite->SetPosition(spritePos.x, spritePos.y);
	toolImageSprite->Draw();
	
	RenderManager::Instance()->RestoreRenderTarget();
	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	RenderManager::Instance()->SetColor(Color::White);
	
	drawSystem->GetLandscapeProxy()->SetCustomColorsTexture(colorSprite->GetTexture());
	
	Rect updatedRect;
	updatedRect.SetCenter(spritePos);
	updatedRect.SetSize(spriteSize);
	AddRectToAccumulator(updatedRect);
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
	originalImage = drawSystem->GetCustomColorsProxy()->GetSprite()->GetTexture()->CreateImageFromMemory();
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

	Sprite* customColorsSprite = drawSystem->GetCustomColorsProxy()->GetSprite();
	Texture* customColorsTexture = customColorsSprite->GetTexture();

	Image* image = customColorsTexture->CreateImageFromMemory();
	ImageLoader::Save(image, filePath);
	SafeRelease(image);

	StoreSaveFileName(filePath);
	drawSystem->GetCustomColorsProxy()->ResetChanges();
}

void CustomColorsSystem::LoadTexture(const DAVA::FilePath &filePath, bool createUndo /* = true */)
{
	if(filePath.IsEmpty())
		return;

	Vector<Image*> images = ImageLoader::CreateFromFile(filePath);
	if(images.empty())
		return;

	Image* image = images.front();
	if(image)
	{
		Texture* texture = Texture::CreateFromData(image->GetPixelFormat(),
												   image->GetData(),
												   image->GetWidth(),
												   image->GetHeight(),
												   false);
		Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

		if (createUndo)
		{
			StoreOriginalState();
		}
		RenderManager::Instance()->SetRenderTarget(drawSystem->GetCustomColorsProxy()->GetSprite());
		sprite->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		AddRectToAccumulator(Rect(Vector2(0.f, 0.f), Vector2(texture->GetWidth(), texture->GetHeight())));

		SafeRelease(sprite);
		SafeRelease(texture);
		for_each(images.begin(), images.end(), SafeRelease<Image>);

		StoreSaveFileName(filePath);

		if (createUndo)
		{
			CreateUndoPoint();
		}
	}
}

void CustomColorsSystem::StoreSaveFileName(const FilePath& filePath)
{
	KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
	if (customProps)
	{
		customProps->SetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, GetRelativePathToProjectPath(filePath));
	}
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

	return absolutePath.GetRelativePathname(EditorSettings::Instance()->GetProjectPath());
}

FilePath CustomColorsSystem::GetAbsolutePathFromProjectPath(const String& relativePath)
{
	if(relativePath.empty())
		return FilePath();
	
	return (EditorSettings::Instance()->GetProjectPath() + relativePath);
}

int32 CustomColorsSystem::GetBrushSize()
{
	return cursorSize;
}

int32 CustomColorsSystem::GetColor()
{
	return colorIndex;
}