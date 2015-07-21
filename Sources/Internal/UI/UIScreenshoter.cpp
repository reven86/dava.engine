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

#include "UIScreenshoter.h"
#include "Render/RenderHelper.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Scene3D/Scene.h"
#include "UI/UI3DView.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{

static const int32 PRIORITY_SCREENSHOT_CLEAR_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 12;
static const int32 PRIORITY_SCREENSHOT_3D_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 11;
static const int32 PRIORITY_SCREENSHOT_2D_PASS = eDefaultPassPriority::PRIORITY_SERVICE_2D + 10;
static const int32 DEFAULT_COOLDOWN = 3; // frames

UIScreenshoter::UIScreenshoter()
    : clearPriority(PRIORITY_SCREENSHOT_CLEAR_PASS)
    , pass3dPriority(PRIORITY_SCREENSHOT_3D_PASS)
    , pass2dPriority(PRIORITY_SCREENSHOT_2D_PASS)
    , cooldownFrames(DEFAULT_COOLDOWN)
{
}

UIScreenshoter::~UIScreenshoter()
{
    for (auto& waiter : waiters)
    {
        waiter.Free();
    }
}

Texture* UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, bool cloneControl /*= false*/)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture * screenshot(Texture::CreateFBO((int32)size.dx, (int32)size.dy, format));
    
    MakeScreenshotInternal(control, screenshot, 0, cloneControl);
    
    return screenshot;
}

void UIScreenshoter::MakeScreenshotWithCallback(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback, bool cloneControl /*= false*/)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture * screenshot(Texture::CreateFBO((int32)size.dx, (int32)size.dy, format));

    MakeScreenshotInternal(control, screenshot, callback, cloneControl);

    SafeRelease(screenshot);
}

void UIScreenshoter::MakeScreenshotWithPreparedTexture(UIControl* control, Texture* screenshot, bool cloneControl /*= false*/)
{
    MakeScreenshotInternal(control, screenshot, 0, cloneControl);
}

void UIScreenshoter::OnFrame()
{
    auto itEnd = waiters.end();
    for (auto it = waiters.begin(); it != itEnd;)
    {
        it->cooldown--;

        if (it->cooldown < 0)
        {
            if (it->callback != 0)
            {
                it->callback(it->texture);
            }
            it->Free();
            it = waiters.erase(it);
            itEnd = waiters.end();
        }
        else
        {
            ++it;
        }
    }
}

void UIScreenshoter::MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback, bool cloneControl)
{
    if (control == nullptr)
        return;

    if (cloneControl)
    {
        control = control->Clone();
    }

    // Create depth and stencil buffer
    rhi::Texture::Descriptor descriptor;
    descriptor.width = screenshot->GetWidth();
    descriptor.height = screenshot->GetHeight();
    descriptor.autoGenMipmaps = false;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = rhi::TEXTURE_FORMAT_D24S8;
    rhi::HTexture dephtTexture = rhi::CreateTexture(descriptor);
    // End creating

    // Prepare waiter
    ScreenshotWaiter waiter;
    waiter.control = SafeRetain(control);
    waiter.texture = SafeRetain(screenshot);
    waiter.depht = dephtTexture;
    waiter.callback = callback;
    waiter.cooldown = DEFAULT_COOLDOWN;
    waiters.push_back(waiter);
    // End preparing

    // Render to texture
    rhi::Viewport viewport;
    viewport.height = waiter.texture->GetHeight();
    viewport.width = waiter.texture->GetWidth();
    RenderHelper::Instance()->CreateClearPass(waiter.texture->handle, clearPriority, Color::Clear, viewport);

    List<Control3dInfo> controls3d;
    FindAll3dViews(waiter.control, controls3d);
    for (auto& info : controls3d)
    {
        SafeRetain(info.control);
        if (nullptr != info.control->GetScene())
        {
            rhi::RenderPassConfig& config = info.control->GetScene()->GetMainPassConfig();
            info.priority = config.priority;
            info.texture = config.colorBuffer[0].texture;
            info.depht = config.depthStencilBuffer.texture;
            config.priority = pass3dPriority;
            config.colorBuffer[0].texture = waiter.texture->handle;
            config.depthStencilBuffer.texture = waiter.depht;
        }
    }
    RenderSystem2D::Instance()->BeginRenderTargetPass(waiter.texture, false, Color::Clear, pass2dPriority);
    waiter.control->Update(0.f);
    waiter.control->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());
    RenderSystem2D::Instance()->EndRenderTargetPass();
    for (auto& info : controls3d)
    {
        if (nullptr != info.control->GetScene())
        {
            rhi::RenderPassConfig& config = info.control->GetScene()->GetMainPassConfig();
            config.priority = info.priority;
            config.colorBuffer[0].texture = info.texture;
            config.depthStencilBuffer.texture = info.depht;
        }
        SafeRelease(info.control);
    }
    // End render

    if (cloneControl)
    {
        SafeRelease(control);
    }
}

void UIScreenshoter::FindAll3dViews(UIControl * control, List<UIScreenshoter::Control3dInfo> & foundViews)
{
    List<UIControl*> processControls;
    processControls.push_back(control);
    while (!processControls.empty())
    {
        auto currentCtrl = processControls.front();
        processControls.pop_front();

        auto& children = currentCtrl->GetChildren();
        for (auto child : children)
        {
            processControls.push_back(child);
        }

        UI3DView* current3dView = dynamic_cast<UI3DView*>(currentCtrl);
        if (nullptr != current3dView)
        {
            Control3dInfo info;
            info.control = current3dView;
            info.priority = 0;
            info.texture = rhi::InvalidHandle;
            foundViews.push_back(info);
        }
    }
}

void UIScreenshoter::ScreenshotWaiter::Free()
{
    SafeRelease(control);
    SafeRelease(texture);
    if (depht.IsValid())
    {
        rhi::DeleteTexture(depht);
    }
}

};