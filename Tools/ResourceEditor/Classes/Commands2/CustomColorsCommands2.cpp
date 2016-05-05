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


#include "Commands2/CustomColorsCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"
#include "../Qt/Main/QtUtils.h"

ModifyCustomColorsCommand::ModifyCustomColorsCommand(Image* originalImage, Image* currentImage, CustomColorsProxy* customColorsProxy_,
                                                     const Rect& updatedRect_, bool shouldClear)
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

    auto material = disableBlend ? RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL : customColorsProxy->GetBrushMaterial();

    Texture* proxy = customColorsProxy->GetTexture();
    desc.colorAttachment = proxy->handle;
    desc.depthAttachment = proxy->handleDepthStencil;
    desc.width = proxy->GetWidth();
    desc.height = proxy->GetHeight();
    desc.clearTarget = shouldClearTexture;
    desc.transformVirtualToPhysical = false;

    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    RenderSystem2D::Instance()->DrawTexture(fboTexture, material, Color::White, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();

    customColorsProxy->UpdateRect(updatedRect);
}

DAVA::Entity* ModifyCustomColorsCommand::GetEntity() const
{
    return nullptr;
}
