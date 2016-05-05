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

CustomColorsSystem::CustomColorsSystem(DAVA::Scene* scene)
    : LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
{
    SetColor(colorIndex);
}

CustomColorsSystem::~CustomColorsSystem()
{
    SafeRelease(toolImageTexture);
    SafeRelease(loadedTexture);
}

LandscapeEditorDrawSystem::eErrorType CustomColorsSystem::EnableLandscapeEditing()
{
    if (enabled)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
    if (canBeEnabledError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
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
    landscapeSize = drawSystem->GetTextureSize(DAVA::Landscape::TEXTURE_COLOR);

    DAVA::FilePath filePath = GetCurrentSaveFileName();
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

    DAVA::Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
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
    if (drawSystem && drawSystem->GetCustomColorsProxy())
    {
        return drawSystem->GetCustomColorsProxy()->GetChangesCount() > 0;
    }
    return false;
}

bool CustomColorsSystem::DisableLandscapeEdititing(bool saveNeeded)
{
    if (!enabled)
    {
        return true;
    }

    if (drawSystem->GetCustomColorsProxy()->GetChangesCount() && saveNeeded)
    {
        SceneSignals::Instance()->EmitCustomColorsTextureShouldBeSaved(((SceneEditor2*)GetScene()));
    }
    FinishEditing();

    selectionSystem->SetLocked(false);
    modifSystem->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();

    drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, true);
    enabled = false;

    SafeRelease(toolImageTexture);
    SafeRelease(loadedTexture);

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

void CustomColorsSystem::Input(DAVA::UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    UpdateCursorPosition();

    if (event->mouseButton == DAVA::UIEvent::MouseButton::LEFT)
    {
        DAVA::Vector3 point;

        switch (event->phase)
        {
        case DAVA::UIEvent::Phase::BEGAN:
            if (isIntersectsLandscape)
            {
                UpdateToolImage();
                StoreOriginalState();
                editingIsEnabled = true;
            }
            break;

        case DAVA::UIEvent::Phase::DRAG:
            break;

        case DAVA::UIEvent::Phase::ENDED:
            FinishEditing();
            break;

        default:
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

void CustomColorsSystem::CreateToolImage(const DAVA::FilePath& filePath)
{
    DAVA::Texture* toolTexture = DAVA::Texture::CreateFromFile(filePath);
    if (!toolTexture)
    {
        return;
    }

    SafeRelease(toolImageTexture);

    toolImageTexture = toolTexture;
    toolImageTexture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}

void CustomColorsSystem::UpdateBrushTool()
{
    DAVA::Texture* colorTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

    DAVA::Vector2 spriteSize = DAVA::Vector2(cursorSize, cursorSize) * landscapeSize;
    DAVA::Vector2 spritePos = cursorPosition * landscapeSize - spriteSize / 2.f;

    DAVA::Rect updatedRect;
    updatedRect.SetPosition(spritePos);
    updatedRect.SetSize(spriteSize);
    AddRectToAccumulator(updatedRect);

    auto brushMaterial = drawSystem->GetCustomColorsProxy()->GetBrushMaterial();
    DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.priority = DAVA::PRIORITY_SERVICE_2D;
    desc.colorAttachment = colorTexture->handle;
    desc.depthAttachment = colorTexture->handleDepthStencil;
    desc.width = colorTexture->GetWidth();
    desc.height = colorTexture->GetHeight();
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = false;
    DAVA::RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    DAVA::RenderSystem2D::Instance()->DrawTexture(toolImageTexture, brushMaterial, drawColor, updatedRect);
    DAVA::RenderSystem2D::Instance()->EndRenderTargetPass();
}

void CustomColorsSystem::ResetAccumulatorRect()
{
    DAVA::float32 inf = std::numeric_limits<DAVA::float32>::infinity();
    updatedRectAccumulator = DAVA::Rect(inf, inf, -inf, -inf);
}

void CustomColorsSystem::AddRectToAccumulator(const DAVA::Rect& rect)
{
    updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

DAVA::Rect CustomColorsSystem::GetUpdatedRect()
{
    DAVA::Rect r = updatedRectAccumulator;
    drawSystem->ClampToTexture(DAVA::Landscape::TEXTURE_COLOR, r);

    return r;
}

void CustomColorsSystem::SetBrushSize(DAVA::int32 brushSize, bool updateDrawSystem /*= true*/)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<DAVA::float32>(brushSize) / landscapeSize;
        if (updateDrawSystem)
        {
            drawSystem->SetCursorSize(cursorSize);
        }
    }
}

void CustomColorsSystem::SetColor(DAVA::int32 colorIndex)
{
    DAVA::Vector<DAVA::Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
    if (colorIndex >= 0 && colorIndex < static_cast<DAVA::int32>(customColors.size()))
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
    DAVA::Rect updatedRect = GetUpdatedRect();
    if (updatedRect.dx > 0 || updatedRect.dy > 0)
    {
        SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
        DVASSERT(scene);

        DAVA::ScopedPtr<DAVA::Image> image(drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory());
        scene->Exec(Command2::Create<ModifyCustomColorsCommand>(originalImage, image, drawSystem->GetCustomColorsProxy(), updatedRect, false));
    }

    SafeRelease(originalImage);
}

void CustomColorsSystem::SaveTexture(const DAVA::FilePath& filePath)
{
    if (filePath.IsEmpty())
        return;

    DAVA::Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

    DAVA::Image* image = customColorsTexture->CreateImageFromMemory();
    DAVA::ImageSystem::Instance()->Save(filePath, image);
    DAVA::SafeRelease(image);

    StoreSaveFileName(filePath);
    drawSystem->GetCustomColorsProxy()->ResetChanges();
}

bool CustomColorsSystem::LoadTexture(const DAVA::FilePath& filePath, bool createUndo)
{
    if (filePath.IsEmpty())
        return false;

    DAVA::Vector<DAVA::Image*> images;
    DAVA::ImageSystem::Instance()->Load(filePath, images);
    if (images.empty())
        return false;

    DAVA::Image* image = images.front();
    if (CouldApplyImage(image, filePath.GetFilename()))
    {
        AddRectToAccumulator(DAVA::Rect(DAVA::Vector2(0.f, 0.f), DAVA::Vector2(image->GetWidth(), image->GetHeight())));

        if (createUndo)
        {
            DVASSERT(originalImage == nullptr);
            originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory();

            SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

            scene->BeginBatch("Load custom colors texture", 2);
            StoreSaveFileName(filePath);
            scene->Exec(Command2::Create<ModifyCustomColorsCommand>(originalImage, image, drawSystem->GetCustomColorsProxy(), GetUpdatedRect(), true));
            scene->EndBatch();

            SafeRelease(originalImage);
        }
        else
        {
            SafeRelease(loadedTexture);
            loadedTexture = DAVA::Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);

            DAVA::Texture* target = drawSystem->GetCustomColorsProxy()->GetTexture();

            auto brushMaterial = drawSystem->GetCustomColorsProxy()->GetBrushMaterial();
            DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;
            desc.priority = DAVA::PRIORITY_SERVICE_2D;
            desc.colorAttachment = target->handle;
            desc.depthAttachment = target->handleDepthStencil;
            desc.width = target->GetWidth();
            desc.height = target->GetHeight();
            desc.clearTarget = false;
            desc.transformVirtualToPhysical = false;
            DAVA::RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
            DAVA::RenderSystem2D::Instance()->DrawTexture(loadedTexture, DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, DAVA::Color::White);
            DAVA::RenderSystem2D::Instance()->EndRenderTargetPass();
        }
    }

    for_each(images.begin(), images.end(), DAVA::SafeRelease<DAVA::Image>);
    return true;
}

bool CustomColorsSystem::CouldApplyImage(DAVA::Image* image, const DAVA::String& imageName) const
{
    if (image == nullptr)
    {
        return false;
    }

    if (image->GetPixelFormat() != DAVA::FORMAT_RGBA8888)
    {
        DAVA::Logger::Error("[CustomColorsSystem] %s has wrong format (%s). We need RGBA888", imageName.c_str(), GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(image->GetPixelFormat()));
        return false;
    }

    const DAVA::Texture* oldTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
    if (oldTexture != nullptr)
    {
        const DAVA::Size2i imageSize(image->GetWidth(), image->GetHeight());
        const DAVA::Size2i textureSize(oldTexture->GetWidth(), oldTexture->GetHeight());

        if (imageSize != textureSize)
        {
            DAVA::Logger::Error("[CustomColorsSystem] %s has wrong size (%d x %d). We need (%d x %d)", imageName.c_str(), imageSize.dx, imageSize.dy, textureSize.dx, textureSize.dy);
            return false;
        }
    }

    return true;
}

void CustomColorsSystem::StoreSaveFileName(const DAVA::FilePath& filePath)
{
    Command2::Pointer command = CreateSaveFileNameCommand(GetRelativePathToProjectPath(filePath));
    if (command)
    {
        SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
        sc->Exec(std::move(command));
    }
}

Command2::Pointer CustomColorsSystem::CreateSaveFileNameCommand(const DAVA::String& filePath)
{
    DAVA::KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
    bool keyExists = customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);

    if (keyExists)
    {
        DAVA::String curPath = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
        if (curPath != filePath)
        {
            return Command2::Create<KeyeadArchiveSetValueCommand>(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, DAVA::VariantType(filePath));
        }
    }
    else
    {
        return Command2::Create<KeyedArchiveAddValueCommand>(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, DAVA::VariantType(filePath));
    }

    return Command2::CreateEmptyCommand();
}

DAVA::FilePath CustomColorsSystem::GetCurrentSaveFileName()
{
    DAVA::String currentSaveName;

    DAVA::KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
    if (customProps && customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
    {
        currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
    }

    return GetAbsolutePathFromProjectPath(currentSaveName);
}

DAVA::FilePath CustomColorsSystem::GetScenePath()
{
    return ((SceneEditor2*)GetScene())->GetScenePath().GetDirectory();
}

DAVA::String CustomColorsSystem::GetRelativePathToScenePath(const DAVA::FilePath& absolutePath)
{
    if (absolutePath.IsEmpty())
        return DAVA::String();

    return absolutePath.GetRelativePathname(GetScenePath());
}

DAVA::FilePath CustomColorsSystem::GetAbsolutePathFromScenePath(const DAVA::String& relativePath)
{
    if (relativePath.empty())
        return DAVA::FilePath();

    return (GetScenePath() + relativePath);
}

DAVA::String CustomColorsSystem::GetRelativePathToProjectPath(const DAVA::FilePath& absolutePath)
{
    if (absolutePath.IsEmpty())
        return DAVA::String();

    return absolutePath.GetRelativePathname(ProjectManager::Instance()->GetProjectPath());
}

DAVA::FilePath CustomColorsSystem::GetAbsolutePathFromProjectPath(const DAVA::String& relativePath)
{
    if (relativePath.empty())
        return DAVA::FilePath();

    return ProjectManager::Instance()->GetProjectPath() + relativePath;
}

DAVA::int32 CustomColorsSystem::GetBrushSize()
{
    return curToolSize;
}

DAVA::int32 CustomColorsSystem::GetColor()
{
    return colorIndex;
}