#include "UI/UIScreenTransition.h"
#include "Render/RenderHelper.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/Image.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIControlSystem.h"
#include "Time/SystemTimer.h"

#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"
#include "UI/UIScreenshoter.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Update/UIUpdateComponent.h"
#include "UI/Render/UIRenderSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIScreenTransition)
{
    ReflectionRegistrator<UIScreenTransition>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIScreenTransition* o) { o->Release(); })
    .End();
}

UIScreenTransition::UIScreenTransition()
{
    interpolationFunc = Interpolation::GetFunction(Interpolation::EASY_IN_EASY_OUT);
    GetOrCreateComponent<UIUpdateComponent>();
}

UIScreenTransition::~UIScreenTransition()
{
    DVASSERT(renderTargetPrevScreen == nullptr && renderTargetNextScreen == nullptr);
}

void UIScreenTransition::CreateRenderTargets()
{
    if (renderTargetPrevScreen || renderTargetNextScreen)
    {
        Logger::FrameworkDebug("Render targets already created");
        return;
    }

    VirtualCoordinatesSystem* vcs = UIControlSystem::Instance()->vcs;

    Size2i physicalTargetSize = vcs->GetPhysicalScreenSize();

    uint32 width = physicalTargetSize.dx;
    uint32 height = physicalTargetSize.dy;

    Texture* tex1 = Texture::CreateFBO(width, height, rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R5G6B5) ? FORMAT_RGB565 : FORMAT_RGBA8888, true);
    Texture* tex2 = Texture::CreateFBO(width, height, rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R5G6B5) ? FORMAT_RGB565 : FORMAT_RGBA8888, true);

    renderTargetPrevScreen = Sprite::CreateFromTexture(tex1, 0, 0, static_cast<float32>(width), static_cast<float32>(height), true);
    renderTargetNextScreen = Sprite::CreateFromTexture(tex2, 0, 0, static_cast<float32>(width), static_cast<float32>(height), true);

    SafeRelease(tex1);
    SafeRelease(tex2);
}

void UIScreenTransition::ReleaseRenderTargets()
{
    SafeRelease(renderTargetPrevScreen);
    SafeRelease(renderTargetNextScreen);
}

void UIScreenTransition::StartTransition()
{
    currentTime = 0.0f;
    complete = false;

    CreateRenderTargets();
}

void UIScreenTransition::SetSourceScreen(UIControl* prevScreen, bool updateScreen)
{
    DVASSERT(renderTargetPrevScreen && renderTargetNextScreen);

    UIScreenshoter* screenshoter = UIControlSystem::Instance()->GetRenderSystem()->GetScreenshoter();
    screenshoter->MakeScreenshot(prevScreen, renderTargetPrevScreen->GetTexture(), true, updateScreen);
}

void UIScreenTransition::SetDestinationScreen(UIControl* nextScreen, bool updateScreen)
{
    DVASSERT(renderTargetPrevScreen && renderTargetNextScreen);

    UIScreenshoter* screenshoter = UIControlSystem::Instance()->GetRenderSystem()->GetScreenshoter();
    screenshoter->MakeScreenshot(nextScreen, renderTargetNextScreen->GetTexture(), true, updateScreen);
}

void UIScreenTransition::EndTransition()
{
    ReleaseRenderTargets();
}

void UIScreenTransition::Update(float32 timeElapsed)
{
    UIScreen::Update(timeElapsed);

    currentTime += timeElapsed;
    normalizedTime = interpolationFunc(currentTime / duration);
    if (currentTime >= duration)
    {
        currentTime = duration;
        complete = true;
    }
}

void UIScreenTransition::Draw(const UIGeometricData& geometricData)
{
    if (renderTargetPrevScreen && renderTargetNextScreen)
    {
        Sprite::DrawState drawState;
        drawState.SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);

        drawState.SetScale(0.5f, 1.0f);
        drawState.SetPosition(0, 0);

        RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState, Color::White);

        drawState.SetScale(0.5f, 1.0f);
        drawState.SetPosition((UIControlSystem::Instance()->vcs->GetFullScreenVirtualRect().dx) / 2.0f, 0);

        RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState, Color::White);
    }
}

void UIScreenTransition::SetDuration(float32 timeInSeconds)
{
    duration = timeInSeconds;
};

bool UIScreenTransition::IsComplete() const
{
    return complete;
}
};
