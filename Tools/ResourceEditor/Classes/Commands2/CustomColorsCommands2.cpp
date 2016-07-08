#include "Commands2/CustomColorsCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"
#include "../Qt/Main/QtUtils.h"

ModifyCustomColorsCommand::ModifyCustomColorsCommand(DAVA::Image* originalImage, DAVA::Image* currentImage, CustomColorsProxy* customColorsProxy_,
                                                     const DAVA::Rect& updatedRect_, bool shouldClear)
    : Command2(CMDID_CUSTOM_COLORS_MODIFY, "Custom Colors Modification")
    , shouldClearTexture(shouldClear)
{
    const DAVA::Vector2 topLeft(floorf(updatedRect_.x), floorf(updatedRect_.y));
    const DAVA::Vector2 bottomRight(ceilf(updatedRect_.x + updatedRect_.dx), ceilf(updatedRect_.y + updatedRect_.dy));

    updatedRect = DAVA::Rect(topLeft, bottomRight - topLeft);

    customColorsProxy = SafeRetain(customColorsProxy_);

    undoImage = DAVA::Image::CopyImageRegion(originalImage, updatedRect);
    redoImage = DAVA::Image::CopyImageRegion(currentImage, updatedRect);
}

ModifyCustomColorsCommand::~ModifyCustomColorsCommand()
{
    SafeRelease(undoImage);
    SafeRelease(redoImage);
    SafeRelease(customColorsProxy);
}

void ModifyCustomColorsCommand::Undo()
{
    ApplyImage(undoImage, true);
    customColorsProxy->DecrementChanges();
}

void ModifyCustomColorsCommand::Redo()
{
    ApplyImage(redoImage, false);
    customColorsProxy->IncrementChanges();
}

void ModifyCustomColorsCommand::ApplyImage(DAVA::Image* image, bool disableBlend)
{
    DAVA::ScopedPtr<DAVA::Texture> fboTexture(DAVA::Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false));

    DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;

    auto material = disableBlend ? DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL : customColorsProxy->GetBrushMaterial();

    DAVA::Texture* proxy = customColorsProxy->GetTexture();
    desc.colorAttachment = proxy->handle;
    desc.depthAttachment = proxy->handleDepthStencil;
    desc.width = proxy->GetWidth();
    desc.height = proxy->GetHeight();
    desc.clearTarget = shouldClearTexture;
    desc.transformVirtualToPhysical = false;

    DAVA::RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    DAVA::RenderSystem2D::Instance()->DrawTexture(fboTexture, material, DAVA::Color::White, updatedRect);
    DAVA::RenderSystem2D::Instance()->EndRenderTargetPass();

    customColorsProxy->UpdateRect(updatedRect);
}

DAVA::Entity* ModifyCustomColorsCommand::GetEntity() const
{
    return nullptr;
}
