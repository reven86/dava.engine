#include "CustomColorsProxy.h"
#include "Deprecated/EditorConfig.h"

#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/REGlobal.h"

#include "Render/Texture.h"
#include "Render/Material/NMaterial.h"

#include "Settings/Settings.h"
#include "Settings/SettingsManager.h"

using namespace DAVA;

CustomColorsProxy::CustomColorsProxy(int32 _size)
    : changedRect(Rect())
    , spriteChanged(false)
    , textureLoaded(false)
    , size(_size)
    , changes(0)
    , brushMaterial(new NMaterial())
{
    Texture::FBODescriptor fboDesc;
    fboDesc.width = size;
    fboDesc.height = size;
    fboDesc.textureType = rhi::TextureType::TEXTURE_TYPE_2D;
    fboDesc.format = PixelFormat::FORMAT_RGBA8888;
    fboDesc.needDepth = false;
    fboDesc.needPixelReadback = true;
    customColorsRenderTarget = Texture::CreateFBO(fboDesc);

    // clear texture, to initialize frame buffer object
    // using PRIORITY_SERVICE_2D + 1 to ensure it will be cleared before drawing existing image into render target
    RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_SERVICE_2D + 1, DAVA::Color::Clear, rhi::Viewport(0, 0, size, size));

    brushMaterial->SetMaterialName(FastName("CustomColorsMaterial"));
    brushMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/CustomColors.material"));
    brushMaterial->PreBuildMaterial(RenderSystem2D::RENDER_PASS_NAME);
}

void CustomColorsProxy::ResetLoadedState(bool isLoaded)
{
    textureLoaded = isLoaded;
}

bool CustomColorsProxy::IsTextureLoaded() const
{
    return textureLoaded;
}

CustomColorsProxy::~CustomColorsProxy()
{
    SafeRelease(customColorsRenderTarget);
}

Texture* CustomColorsProxy::GetTexture()
{
    return customColorsRenderTarget;
}

void CustomColorsProxy::ResetTargetChanged()
{
    spriteChanged = false;
}

bool CustomColorsProxy::IsTargetChanged()
{
    return spriteChanged;
}

Rect CustomColorsProxy::GetChangedRect()
{
    if (IsTargetChanged())
    {
        return changedRect;
    }

    return Rect();
}

void CustomColorsProxy::UpdateRect(const DAVA::Rect& rect)
{
    DAVA::Rect bounds(0.f, 0.f, (float32)size, (float32)size);
    changedRect = rect;
    bounds.ClampToRect(changedRect);

    spriteChanged = true;
}

int32 CustomColorsProxy::GetChangesCount() const
{
    return changes;
}

void CustomColorsProxy::ResetChanges()
{
    changes = 0;
}

void CustomColorsProxy::IncrementChanges()
{
    ++changes;
}

void CustomColorsProxy::DecrementChanges()
{
    --changes;
}

void CustomColorsProxy::UpdateSpriteFromConfig()
{
    if (NULL == customColorsRenderTarget)
    {
        return;
    }

    rhi::Viewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = viewport.height = size;

    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data);

    Vector<Color> customColors = data->GetEditorConfig()->GetColorPropertyValues("LandscapeCustomColors");
    if (customColors.empty())
    {
        RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_CLEAR, Color::Clear, viewport);
    }
    else
    {
        DAVA::uint32 defaultColorIndex = SettingsManager::GetValue(Settings::Scene_DefaultCustomColorIndex).AsUInt32();
        defaultColorIndex = Min(defaultColorIndex, static_cast<DAVA::uint32>(customColors.size() - 1));
        Color color = customColors[defaultColorIndex];
        RenderHelper::CreateClearPass(customColorsRenderTarget->handle, rhi::HTexture(), PRIORITY_CLEAR, color, viewport);
    }
}

NMaterial* CustomColorsProxy::GetBrushMaterial() const
{
    return brushMaterial.get();
}