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
    : LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
    , drawColor(Color(0.f, 0.f, 0.f, 0.f))
{
    curToolSize = 120;
    SetColor(colorIndex);
}

CustomColorsSystem::~CustomColorsSystem()
{
    if(toolImageTexture)
    {
        SafeRelease(toolImageTexture);
        rhi::ReleaseTextureSet(toolTextureSet);
    }
    
    if(loadedTexture)
    {
        SafeRelease(loadedTexture);
        rhi::ReleaseTextureSet(loadedTextureSet);
    }
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
    landscapeSize = drawSystem->GetTextureSize(Landscape::TEXTURE_COLOR);

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

	drawSystem->EnableCursor();
	drawSystem->SetCursorTexture(cursorTexture);
    SetBrushSize(curToolSize);
	
	Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetToolTexture(customColorsTexture, true);
	
	if (!toolImageTexture)
	{
		CreateToolImage("~res:/LandscapeEditor/Tools/customcolorsbrush/circle.tex");
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
    
	drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, true);
	enabled = false;
	
    if(toolImageTexture)
    {
        SafeRelease(toolImageTexture);
        rhi::ReleaseTextureSet(toolTextureSet);
    }
    
    if(loadedTexture)
    {
        SafeRelease(loadedTexture);
        rhi::ReleaseTextureSet(loadedTextureSet);
    }
    
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
			UpdateBrushTool();
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


void CustomColorsSystem::UpdateToolImage(bool force)
{
}

void CustomColorsSystem::CreateToolImage(const FilePath& filePath)
{
	Texture* toolTexture = Texture::CreateFromFile(filePath);
	if (!toolTexture)
	{
		return;
	}
	
    if(toolImageTexture)
    {
        SafeRelease(toolImageTexture);
        rhi::ReleaseTextureSet(toolTextureSet);
    }
    
    toolImageTexture = toolTexture;
    toolImageTexture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
    
    rhi::TextureSetDescriptor desc;
    desc.fragmentTextureCount = 1;
    desc.fragmentTexture[0] = toolImageTexture->handle;
    toolTextureSet = rhi::AcquireTextureSet(desc);
}

void CustomColorsSystem::UpdateBrushTool()
{
	Texture* colorTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
	
	Vector2 spriteSize = Vector2(cursorSize, cursorSize) * landscapeSize;
	Vector2 spritePos = cursorPosition * landscapeSize - spriteSize / 2.f;

    Rect updatedRect;
    updatedRect.SetPosition(spritePos);
    updatedRect.SetSize(spriteSize);
    AddRectToAccumulator(updatedRect);

    glLoadIdentity();

    auto brushMaterial = drawSystem->GetCustomColorsProxy()->GetBrushMaterial();
    RenderSystem2D::Instance()->BeginRenderTargetPass(colorTexture, false);
    RenderSystem2D::Instance()->DrawTexture(toolTextureSet, brushMaterial, drawColor, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();
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
    drawSystem->ClampToTexture(Landscape::TEXTURE_COLOR, r);

	return r;
}

void CustomColorsSystem::SetBrushSize(int32 brushSize, bool updateDrawSystem /*= true*/)
{
	if (brushSize > 0)
	{
        curToolSize = brushSize;
		cursorSize = (float32)brushSize / landscapeSize;
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
	originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory();
	ResetAccumulatorRect();
}

void CustomColorsSystem::CreateUndoPoint()
{
	Rect updatedRect = GetUpdatedRect();
	if (updatedRect.dx > 0 || updatedRect.dy > 0)
	{
		SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
		DVASSERT(scene);

		scene->Exec(new ModifyCustomColorsCommand(originalImage, ScopedPtr<Image>(drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory())
                                                  , drawSystem->GetCustomColorsProxy(), updatedRect));
	}

	SafeRelease(originalImage);
}

void CustomColorsSystem::SaveTexture(const DAVA::FilePath &filePath)
{
	if(filePath.IsEmpty())
		return;

    Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

	Image* image = customColorsTexture->CreateImageFromMemory();
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
    if (CouldApplyImage(image, filePath.GetFilename()))
    {
        AddRectToAccumulator(Rect(Vector2(0.f, 0.f), Vector2(image->GetWidth(), image->GetHeight())));
        
		if (createUndo)
        {
            originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory();
            
            SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
            
            scene->BeginBatch("Load custom colors texture");
            StoreSaveFileName(filePath);
            scene->Exec(new ModifyCustomColorsCommand(originalImage, image,
                                                      drawSystem->GetCustomColorsProxy(), GetUpdatedRect()));
            scene->EndBatch();
            
            SafeRelease(originalImage);
		}
        else
        {
            if(loadedTexture)
            {
                SafeRelease(loadedTexture);
                rhi::ReleaseTextureSet(loadedTextureSet);
            }
            
            loadedTexture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),  image->GetWidth(), image->GetHeight(), false);
            
            rhi::TextureSetDescriptor desc;
            desc.fragmentTextureCount = 1;
            desc.fragmentTexture[0] = loadedTexture->handle;
            loadedTextureSet = rhi::AcquireTextureSet(desc);
            
            Texture * target = drawSystem->GetCustomColorsProxy()->GetTexture();

            auto brushMaterial = drawSystem->GetCustomColorsProxy()->GetBrushMaterial();
            RenderSystem2D::Instance()->BeginRenderTargetPass(target, false);
            RenderSystem2D::Instance()->DrawTexture(loadedTextureSet, brushMaterial, Color::White);
            RenderSystem2D::Instance()->EndRenderTargetPass();
        }
	}

    for_each(images.begin(), images.end(), SafeRelease<Image>);
    return true;
}

bool CustomColorsSystem::CouldApplyImage(Image* image, const String& imageName) const
{
    if (image == nullptr)
    {
        return false;
    }

    if (image->GetPixelFormat() != FORMAT_RGBA8888)
    {
        Logger::Error("[CustomColorsSystem] %s has wrong format (%s). We need RGBA888", imageName.c_str(), GlobalEnumMap<PixelFormat>::Instance()->ToString(image->GetPixelFormat()));
        return false;
    }

    const Texture* oldTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
    if (oldTexture != nullptr)
    {
        const Size2i imageSize(image->GetWidth(), image->GetHeight());
        const Size2i textureSize(oldTexture->GetWidth(), oldTexture->GetHeight());

        if (imageSize != textureSize)
        {
            Logger::Error("[CustomColorsSystem] %s has wrong size (%d x %d). We need (%d x %d)", imageName.c_str(), imageSize.dx, imageSize.dy, textureSize.dx, textureSize.dy);
            return false;
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
	return curToolSize;
}

int32 CustomColorsSystem::GetColor()
{
	return colorIndex;
}