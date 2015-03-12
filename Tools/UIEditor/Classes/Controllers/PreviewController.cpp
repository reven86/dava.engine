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


#include "PreviewController.h"
#include "DefaultScreen.h"

#define DEFAULT_GFX_EXTENSION "Gfx"

namespace DAVA {

int32 ScreenParamsConverter::ConvertDiagonalToDPI(const Vector2& screenSize, float32 diagonal)
{
    if (FLOAT_EQUAL(diagonal, 0.0f))
    {
        return 0;
    }

    float32 diagonalInPixels = sqrt(screenSize.SquareLength());
    return diagonalInPixels / diagonal;
}

float32 ScreenParamsConverter::ConverDPIToDiagonal(const Vector2& screenSize, int32 dpi)
{
    if (dpi == 0)
    {
        return 0.0f;
    }

    float32 diagonalInPixels = sqrt(screenSize.SquareLength());
    return diagonalInPixels / (float32)dpi;
}

static const PreviewSettingsData predefinedPreviewSettings[] =
{
    {-1, "iPad 2", Vector2(1024, 768), 132, true, 0},
    {-2, "iPad Retina (iPad 3/4/Air)", Vector2(2048, 1536), 264, true, 1},
    {-3, "iPad mini", Vector2(1024, 768), 163, true, 2},
    {-4, "iPad mini Retina", Vector2(2048, 1536), 326, true, 3},
    {-5, "iPhone 4S", Vector2(960, 640), 326, true, 4},
    {-6, "iPhone 5/5C/5S", Vector2(1136, 640), 326, true, 5},
    {-7, "iPhone 6", Vector2(1134, 750), 326, true, 6},
    {-8, "iPhone 6 Plus", Vector2(1920, 1080), 401, true, 7},
    {-9, "Nexus 7 (2012)", Vector2(1280, 800), 216, true, 8},
    {-10, "Nexus 7 (2013)", Vector2(1920, 1080), 323, true, 9},
};

#define PREVIEW_SETTINGS_NODE "previewSettings"
#define PREVIEW_SETTINGS_FORMAT_NODE "previewSettings%i"
    
#define PREVIEW_SETTINGS_POSITION_NODE "position"
#define PREVIEW_SETTINGS_DEVICE_NODE "device"
#define PREVIEW_SETTINGS_WIDTH_NODE "width"
#define PREVIEW_SETTINGS_HEIGHT_NODE "height"
#define PREVIEW_SETTINGS_DPI_NODE "dpi"

#define EMPTY_PREVIEW_SETTINGS_ID -99
    
int32 PreviewController::nextId = 0;
    

    
PreviewController::PreviewController() :
    previewEnabled(false),
    isDirty(false),
    activePreviewSettingsID(EMPTY_PREVIEW_SETTINGS_ID),
    isApplyPreviewScale(false)
{
    LoadPredefinedPreviewSettings();
}

PreviewController::~PreviewController()
{
    previewSettings.clear();
}

void PreviewController::EnablePreview(bool applyScale)
{
    isApplyPreviewScale = applyScale;
    previewEnabled = true;
}

const PreviewTransformData& PreviewController::SetPreviewMode(const PreviewSettingsData& data,
                                                              const Vector2& virtualScreenSize,
                                                              uint32 screenDPI)
{
    activePreviewSettingsID = data.id;
    currentTransformData = CalculateTransform(data, virtualScreenSize, screenDPI);
    previewEnabled = true;
    
    return currentTransformData;
}

void PreviewController::DisablePreview()
{
    if (!previewEnabled)
    {
        return;
    }

    previewEnabled = false;
    activePreviewSettingsID = EMPTY_PREVIEW_SETTINGS_ID;
}
    
const PreviewTransformData& PreviewController::GetTransformData() const
{
    return currentTransformData;
}

bool previewSettingsDataSortFn(const PreviewSettingsData & a, const PreviewSettingsData & b)
{
    return a.positionInList < b.positionInList;
}

void PreviewController::LoadPreviewSettings(const YamlNode* rootNode)
{
    previewSettings.clear();
    LoadPredefinedPreviewSettings();

    const YamlNode* previewSettingsNode = rootNode->Get(PREVIEW_SETTINGS_NODE);
    if (!previewSettingsNode)
    {
        return;
    }
    
    List<PreviewSettingsData> customSettings;
    int32 settingsCount = previewSettingsNode->GetCount();
    for (int32 i = 0; i < settingsCount; i++)
	{
		const YamlNode* settingsNode = previewSettingsNode->Get(i);
		if (!settingsNode)
        {
			continue;
        }

        const YamlNode* positionNode = settingsNode->Get(PREVIEW_SETTINGS_POSITION_NODE);
        const YamlNode* deviceNode = settingsNode->Get(PREVIEW_SETTINGS_DEVICE_NODE);
        const YamlNode* widthNode = settingsNode->Get(PREVIEW_SETTINGS_WIDTH_NODE);
        const YamlNode* heigthNode = settingsNode->Get(PREVIEW_SETTINGS_HEIGHT_NODE);
        const YamlNode* dpiNode = settingsNode->Get(PREVIEW_SETTINGS_DPI_NODE);
        
        if (!positionNode || !deviceNode || !widthNode || !heigthNode || !dpiNode)
        {
            continue;
        }
        
        PreviewSettingsData newItem;

        newItem.id = nextId++;
        newItem.deviceName = deviceNode->AsString();
        newItem.screenSize.x = widthNode->AsInt32();
        newItem.screenSize.y = heigthNode->AsInt32();
        newItem.dpi = dpiNode->AsInt32();
        newItem.positionInList = positionNode->AsInt32();
        newItem.isPredefined = false;

        customSettings.push_back(newItem);
    }
    
    customSettings.sort(previewSettingsDataSortFn);
    previewSettings.insert(previewSettings.end(), customSettings.begin(), customSettings.end());
    
    // At this moment no changes were made.
    SetDirty(false);
}

void PreviewController::LoadPredefinedPreviewSettings()
{
    // Predefined preview settings aren't stored in the Yaml.
    int32 predefinedItemsCount = COUNT_OF(predefinedPreviewSettings);
    for (int32 i = 0; i < predefinedItemsCount; i ++)
    {
        previewSettings.push_back(predefinedPreviewSettings[i]);
    }
}

void PreviewController::SavePreviewSettings(YamlNode* rootNode)
{
    bool customSettingsFound = false;
    for (List<PreviewSettingsData>::iterator iter = previewSettings.begin(); iter != previewSettings.end(); iter ++)
    {
        if (!iter->isPredefined)
        {
            customSettingsFound = true;
            break;
        }
    }
    
    // Have to save non-predefined settings only.
    if (!customSettingsFound)
    {
        return;
    }
    
    YamlNode* settingsNode = new YamlNode(YamlNode::TYPE_MAP);
    rootNode->SetNodeToMap(PREVIEW_SETTINGS_NODE, settingsNode);
    
    int32 position = 0;
    for (List<PreviewSettingsData>::iterator iter = previewSettings.begin(); iter != previewSettings.end(); iter ++)
    {
        if (iter->isPredefined)
        {
            continue;
        }

        YamlNode* setting = new YamlNode(YamlNode::TYPE_MAP);
        String nodeName = Format(PREVIEW_SETTINGS_FORMAT_NODE, position);
        settingsNode->AddNodeToMap(nodeName, setting);
        
        setting->Set(PREVIEW_SETTINGS_POSITION_NODE, position);
        setting->Set(PREVIEW_SETTINGS_DEVICE_NODE, iter->deviceName);
        setting->Set(PREVIEW_SETTINGS_WIDTH_NODE, iter->screenSize.x);
        setting->Set(PREVIEW_SETTINGS_HEIGHT_NODE, iter->screenSize.y);
        setting->Set(PREVIEW_SETTINGS_DPI_NODE, (int32)iter->dpi);
        position ++;
    }
    
    // The settings are in sync.
    SetDirty(false);
}

const List<PreviewSettingsData>& PreviewController::GetPreviewSettings() const
{
    return previewSettings;
}

PreviewSettingsData PreviewController::GetPreviewSettingsData(int32 id) const
{
    for (List<PreviewSettingsData>::const_iterator iter = previewSettings.begin(); iter != previewSettings.end(); iter ++)
    {
        if (iter->id == id)
        {
            return *iter;
        }
    }
    
    DVASSERT(false);
    return PreviewSettingsData();
}

PreviewSettingsData PreviewController::GetActivePreviewSettingsData() const
{
    return GetPreviewSettingsData(GetActivePreviewSettingsID());
}

void PreviewController::AddPreviewSettingsData(const PreviewSettingsData& data)
{
    PreviewSettingsData newData = data;
    newData.id = nextId ++;
    previewSettings.push_back(newData);
    
    // Preview Settings list is now "dirty" and needs to be saved.
    SetDirty(true);
}

void PreviewController::RemovePreviewSettingsData(int32 id)
{
    for (List<PreviewSettingsData>::iterator iter = previewSettings.begin(); iter != previewSettings.end(); iter ++)
    {
        if (iter->id == id)
        {
            previewSettings.erase(iter);
            SetDirty(true);
            break;
        }
    }
}

PreviewTransformData PreviewController::CalculateTransform(const PreviewSettingsData& settings,
                                                           const Vector2& virtualScreenSize,
                                                           uint32 screenDPI)
{
	float32 virtualScreenWidth = virtualScreenSize.x;
	float32 virtualScreenHeight = virtualScreenSize.y;
    float32 scaleFactor = 1.0f;

	float32 w = virtualScreenWidth / settings.screenSize.x;
	float32 h = virtualScreenHeight / settings.screenSize.y;

	if(w > h)
	{
        virtualScreenHeight = settings.screenSize.y * w;
        scaleFactor = 1.0f / w;
    }
	else
	{
        virtualScreenWidth = settings.screenSize.x * h;
        scaleFactor = 1.0f / h;
    }
    
    PreviewTransformData transformData;
    transformData.screenSize.x = floorf(virtualScreenWidth);
    transformData.screenSize.y = floorf(virtualScreenHeight);
    transformData.zoomLevel = scaleFactor;

    // Now apply the DPI, if requested.
    if (isApplyPreviewScale)
    {
        transformData.zoomLevel *= ((float32)screenDPI / (float32)settings.dpi);
    }

    return transformData;
}
    
bool PreviewController::HasUnsavedChanges() const
{
    return isDirty;
}
    
void PreviewController::SetDirty(bool value)
{
    isDirty = value;
}

void PreviewController::MakeScreenshot(const String& fileName, DefaultScreen* screen)
{
    if (!screen || !screen->GetScreenControl())
    {
        return;
    }

    ScreenControl* screenControl = screen->GetScreenControl();
    Rect rawScreenRect = Rect(Vector2(0.0f, 0.0f), screenControl->GetSize());
    Rect scaledScreenRect = rawScreenRect;
    scaledScreenRect.SetSize(rawScreenRect.GetSize() * screen->GetScale());

    RenderSystem2D::Instance()->Flush();

    Rect textureRect = scaledScreenRect;
    textureRect.SetSize(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(textureRect.GetSize()));
    ScopedPtr<Texture> screenshot(Texture::CreateFBO((int32)ceilf(textureRect.dx), (int32)ceilf(textureRect.dy),
                                                  FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER));
    
    Rect oldViewport = RenderManager::Instance()->GetViewport();

    RenderManager::Instance()->SetRenderTarget(screenshot);
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)screenshot->GetWidth(), (float32)screenshot->GetHeight()));
    RenderManager::Instance()->ClearWithColor(0.0f, 0.0f, 0.0f, 1.0f);

    RenderSystem2D::Instance()->Setup2DMatrices();

    // The clipping rectangle defines on scale and preview mode.
    Rect clipRect = rawScreenRect;
    clipRect.SetSize(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(clipRect.GetSize()));
    if (IsPreviewEnabled())
    {
        clipRect.SetSize(GetTransformData().screenSize);
        if (GetTransformData().zoomLevel > 1.0f)
        {
            clipRect.SetSize(clipRect.GetSize() * GetTransformData().zoomLevel);
        }
    }
    else if (screen->GetScale().x > 1.0f || screen->GetScale().y > 1.0f)
    {
        clipRect = textureRect;
    }

    RenderSystem2D::Instance()->PushClip();
    RenderSystem2D::Instance()->SetClip(clipRect);

    // Draw the screen with the scale requested, but without any offset.
    Matrix4 wt = Matrix4::MakeScale(Vector3(screen->GetScale().x, screen->GetScale().y, 1.f));
    RenderManager::SetDynamicParam(PARAM_VIEW, &wt, UPDATE_SEMANTIC_ALWAYS);
    
    screen->GetScreenControl()->SetScreenshotMode(true);
    screen->GetScreenControl()->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());
    screen->GetScreenControl()->SetScreenshotMode(false);

    RenderSystem2D::Instance()->PopClip();

    RenderSystem2D::Instance()->Flush();

    RenderManager::Instance()->SetRenderTarget(0);
    RenderManager::Instance()->SetViewport(oldViewport);

    RenderSystem2D::Instance()->Setup2DMatrices();

    ScopedPtr<Image> image(screenshot->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND));
    
    // Resulting texture is square. Resize its canvas to the
    // texture size and then resize the image to the original
    // size without virtual to physical factor.
    image->ResizeCanvas(textureRect.dx, textureRect.dy);
    image->ResizeImage(scaledScreenRect.dx, scaledScreenRect.dy);
    ImageSystem::Instance()->Save(fileName, image);
}

};