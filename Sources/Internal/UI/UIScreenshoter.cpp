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

RefPtr<Texture> UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, bool clearAlpha)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    RefPtr<Texture> screenshot(Texture::CreateFBO(static_cast<int32>(size.dx), static_cast<int32>(size.dy), format, true));

    MakeScreenshotInternal(control, screenshot.Get(), nullptr, clearAlpha);

    return screenshot;
}

RefPtr<Texture> UIScreenshoter::MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback, bool clearAlpha)
{
    const Vector2 size(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(control->GetSize()));
    RefPtr<Texture> screenshot(Texture::CreateFBO(static_cast<int32>(size.dx), static_cast<int32>(size.dy), format, true));

    MakeScreenshotInternal(control, screenshot.Get(), callback, clearAlpha);

    return screenshot;
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot, bool clearAlpha)
{
    MakeScreenshotInternal(control, screenshot, nullptr, clearAlpha);
}

void UIScreenshoter::MakeScreenshot(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback, bool clearAlpha)
{
    MakeScreenshotInternal(control, screenshot, callback, clearAlpha);
}

void UIScreenshoter::Unsubscribe(Texture* screenshot)
{
    for (auto& waiter : waiters)
    {
        if (waiter.texture == screenshot)
        {
            waiter.callback = nullptr;
        }
    }
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

    //[DRAW]
    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = screenshot->handle;
    desc.depthAttachment = screenshot->handleDepthStencil;
    desc.width = screenshot->GetWidth();
    desc.height = screenshot->GetHeight();
    desc.priority = PRIORITY_SCREENSHOT + PRIORITY_MAIN_2D;
    desc.clearTarget = true;
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