#include "TilemaskEditorCommands.h"
#include "Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

#include "Main/QtUtils.h"

ModifyTilemaskCommand::ModifyTilemaskCommand(LandscapeProxy* landscapeProxy_, const DAVA::Rect& updatedRect_)
    : RECommand(CMDID_TILEMASK_MODIFY, "Tile Mask Modification")
    , landscapeProxy(SafeRetain(landscapeProxy_))
{
    updatedRect = DAVA::Rect(std::floor(updatedRect_.x), std::floor(updatedRect_.y), std::ceil(updatedRect_.dx), std::ceil(updatedRect_.dy));

    undoImageMask = DAVA::Image::CopyImageRegion(landscapeProxy->GetTilemaskImageCopy(), updatedRect);

    DAVA::ScopedPtr<DAVA::Image> currentImageMask(landscapeProxy->GetLandscapeTexture(DAVA::Landscape::TEXTURE_TILEMASK)->CreateImageFromMemory());
    redoImageMask = DAVA::Image::CopyImageRegion(currentImageMask, updatedRect);
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
    SafeRelease(undoImageMask);
    SafeRelease(redoImageMask);
    SafeRelease(landscapeProxy);
}

void ModifyTilemaskCommand::Undo()
{
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
    ApplyImageToTexture(undoImageMask, landscapeProxy->GetLandscapeTexture(DAVA::Landscape::TEXTURE_TILEMASK));

    landscapeProxy->DecreaseTilemaskChanges();

    DAVA::Rect r = DAVA::Rect(DAVA::Vector2(0, 0), DAVA::Vector2(undoImageMask->GetWidth(), undoImageMask->GetHeight()));
    DAVA::Image* mask = landscapeProxy->GetTilemaskImageCopy();
    mask->InsertImage(undoImageMask, updatedRect.GetPosition(), r);
}

void ModifyTilemaskCommand::Redo()
{
    ApplyImageToTexture(redoImageMask, landscapeProxy->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
    ApplyImageToTexture(redoImageMask, landscapeProxy->GetLandscapeTexture(DAVA::Landscape::TEXTURE_TILEMASK));

    landscapeProxy->IncreaseTilemaskChanges();

    DAVA::Rect r = DAVA::Rect(DAVA::Vector2(0, 0), DAVA::Vector2(redoImageMask->GetWidth(), redoImageMask->GetHeight()));
    DAVA::Image* mask = landscapeProxy->GetTilemaskImageCopy();
    mask->InsertImage(redoImageMask, updatedRect.GetPosition(), r);
}

void ModifyTilemaskCommand::ApplyImageToTexture(DAVA::Image* image, DAVA::Texture* dstTex)
{
    DAVA::ScopedPtr<DAVA::Texture> fboTexture(DAVA::Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false));

    auto material = DAVA::RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL;

    DAVA::RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = dstTex->handle;
    desc.depthAttachment = dstTex->handleDepthStencil;
    desc.width = dstTex->GetWidth();
    desc.height = dstTex->GetHeight();
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = false;
    DAVA::RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    DAVA::RenderSystem2D::Instance()->DrawTexture(fboTexture, material, DAVA::Color::White, updatedRect);
    DAVA::RenderSystem2D::Instance()->EndRenderTargetPass();
}

SetTileColorCommand::SetTileColorCommand(LandscapeProxy* landscapeProxy_, const DAVA::FastName& level_, const DAVA::Color& color_)
    : RECommand(CMDID_SET_TILE_COLOR, "Set tile color")
    , level(level_)
    , redoColor(color_)
    , landscapeProxy(SafeRetain(landscapeProxy_))
{
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
