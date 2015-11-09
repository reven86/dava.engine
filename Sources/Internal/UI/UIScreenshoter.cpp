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
UIScreenshoter::UIScreenshoter()
{
}

UIScreenshoter::~UIScreenshoter()
{
    for (auto& waiter : waiters)
    {
        SafeRelease(waiter.texture);
    }
}

void UIScreenshoter::OnFrame()
{
    auto itEnd = waiters.end();
    for (auto it = waiters.begin(); it != itEnd;)
    {
        if (rhi::SyncObjectSignaled(it->syncObj))
        {
            if (it->callback != nullptr)
            {
                it->callback(it->texture);
            }

            SafeRelease(it->texture);

            it = waiters.erase(it);
            itEnd = waiters.end();
        }
        else
        {
            ++it;
        }
    }
}

Texture* UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture* screenshot(Texture::CreateFBO((int32)size.dx, (int32)size.dy, format, true));

    MakeScreenshotInternal(control, screenshot, nullptr);

    return screenshot;
}

void UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture* screenshot(Texture::CreateFBO((int32)size.dx, (int32)size.dy, format, true));

    MakeScreenshotInternal(control, screenshot, callback);

    SafeRelease(screenshot);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot)
{
    MakeScreenshotInternal(control, screenshot, nullptr);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback)
{
    MakeScreenshotInternal(control, screenshot, callback);
}

void UIScreenshoter::MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback)
{
    if (control == nullptr)
        return;

    // Prepare waiter
    ScreenshotWaiter waiter;
    waiter.texture = SafeRetain(screenshot);
    waiter.callback = callback;
    waiter.syncObj = rhi::GetCurrentFrameSyncObject();
    waiters.push_back(waiter);
    // End preparing

    // Render to texture
    List<Control3dInfo> controls3d;
    FindAll3dViews(control, controls3d);
    for (auto& info : controls3d)
    {
        SafeRetain(info.control);
        if (nullptr != info.control->GetScene())
        {
            rhi::RenderPassConfig& config = info.control->GetScene()->GetMainPassConfig();
            info.scenePassConfig = config;

            config.priority = PRIORITY_SCREENSHOT_3D;
            config.colorBuffer[0].texture = waiter.texture->handle;
            config.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
            Memcpy(config.colorBuffer[0].clearColor, Color::Clear.color, sizeof(Color));
            if (waiter.texture->handleDepthStencil != rhi::InvalidHandle)
                config.depthStencilBuffer.texture = waiter.texture->handleDepthStencil;
        }
    }
    RenderSystem2D::Instance()->BeginRenderTargetPass(waiter.texture, false, Color::Clear, PRIORITY_SCREENSHOT_2D);
    control->Update(0.0f);
    control->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());
    RenderSystem2D::Instance()->EndRenderTargetPass();
    for (auto& info : controls3d)
    {
        if (nullptr != info.control->GetScene())
        {
            rhi::RenderPassConfig& config = info.control->GetScene()->GetMainPassConfig();
            config = info.scenePassConfig;
        }
        SafeRelease(info.control);
    }
    // End render
}

void UIScreenshoter::FindAll3dViews(UIControl* control, List<UIScreenshoter::Control3dInfo>& foundViews)
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
            foundViews.push_back(info);
        }
    }
}
};