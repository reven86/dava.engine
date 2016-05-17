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

Texture* UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, bool clearAlpha)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture* screenshot(Texture::CreateFBO(static_cast<int32>(size.dx), static_cast<int32>(size.dy), format, true));

    MakeScreenshotInternal(control, screenshot, nullptr, clearAlpha);

    return screenshot;
}

void UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    Texture* screenshot(Texture::CreateFBO(static_cast<int32>(size.dx), static_cast<int32>(size.dy), format, true));

    MakeScreenshotInternal(control, screenshot, callback, false);

    SafeRelease(screenshot);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot)
{
    MakeScreenshotInternal(control, screenshot, nullptr, false);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback)
{
    MakeScreenshotInternal(control, screenshot, callback, false);
}

void UIScreenshoter::MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback, bool clearAlpha)
{
    if (control == nullptr)
        return;
    DVASSERT(screenshot);
    // Prepare waiter
    ScreenshotWaiter waiter;
    waiter.texture = SafeRetain(screenshot);
    waiter.callback = callback;
    waiter.syncObj = rhi::GetCurrentFrameSyncObject();
    waiters.push_back(waiter);
    // End preparing

    // Render to texture

    //[CLEAR]
    rhi::Viewport viewport;
    viewport.x = viewport.y = 0U;
    viewport.width = screenshot->GetWidth();
    viewport.height = screenshot->GetHeight();
    RenderHelper::CreateClearPass(screenshot->handle, screenshot->handleDepthStencil, PRIORITY_SCREENSHOT + PRIORITY_CLEAR, Color::Clear, viewport);

    //[DRAW]
    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = screenshot->handle;
    desc.depthAttachment = screenshot->handleDepthStencil;
    desc.width = screenshot->GetWidth();
    desc.height = screenshot->GetHeight();
    desc.priority = PRIORITY_SCREENSHOT + PRIORITY_MAIN_2D;
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = true;

    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    control->SystemUpdate(0.0f);
    control->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());

    //[CLEAR ALPHA]
    if (clearAlpha)
    {
        RenderSystem2D::Instance()->FillRect(Rect(0.0f, 0.0f, static_cast<float32>(screenshot->GetWidth()), static_cast<float32>(screenshot->GetHeight())), Color::White, RenderSystem2D::DEFAULT_2D_FILL_ALPHA_MATERIAL);
    }

    RenderSystem2D::Instance()->EndRenderTargetPass();
    // End render
}
};