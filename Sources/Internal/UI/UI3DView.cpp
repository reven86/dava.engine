#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"
#include "Render/RenderHelper.h"
#include "Core/Core.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/SnapToLandscapeControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

namespace DAVA
{
UI3DView::UI3DView(const Rect& rect)
    : UIControl(rect)
    , scene(nullptr)
    , registeredInUIControlSystem(false)
    , drawToFrameBuffer(false)
    , fbScaleFactor(1.f)
    , fbRenderSize()
{
}

UI3DView::~UI3DView()
{
    SafeRelease(frameBuffer);
    SafeRelease(scene);
}

void UI3DView::SetScene(Scene* _scene)
{
    SafeRelease(scene);

    scene = SafeRetain(_scene);

    if (scene)
    {
        float32 aspect = size.dx / size.dy;
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

Scene* UI3DView::GetScene() const
{
    return scene;
}

void UI3DView::AddControl(UIControl* control)
{
    DVASSERT(0 && "UI3DView do not support children");
}

void UI3DView::Update(float32 timeElapsed)
{
    if (scene)
        scene->Update(timeElapsed);
}

void UI3DView::Draw(const UIGeometricData& geometricData)
{
    if (!scene)
        return;

    RenderSystem2D::Instance()->Flush();

    rhi::RenderPassConfig& config = scene->GetMainPassConfig();
    const RenderSystem2D::RenderTargetPassDescriptor& currentTarget = RenderSystem2D::Instance()->GetActiveTargetDescriptor();

    Rect viewportRect = geometricData.GetUnrotatedRect();

    if (currentTarget.transformVirtualToPhysical)
        viewportRc = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(viewportRect);
    else
        viewportRc = viewportRect;

    if (drawToFrameBuffer)
    {
        // Calculate viewport for frame buffer
        viewportRc.x = 0.f;
        viewportRc.y = 0.f;
        viewportRc.dx *= fbScaleFactor;
        viewportRc.dy *= fbScaleFactor;

        PrepareFrameBuffer();

        config.priority = currentTarget.priority + PRIORITY_SERVICE_3D;
        config.colorBuffer[0].targetTexture = frameBuffer->handle;
        config.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
        config.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        config.depthStencilBuffer.targetTexture = frameBuffer->handleDepthStencil;
        config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        config.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    }
    else
    {
        if (currentTarget.transformVirtualToPhysical)
            viewportRc += VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

        const FastName& currentMSAA = QualitySettingsSystem::Instance()->GetCurMSAAQuality();
        if (currentMSAA.IsValid() && rhi::DeviceCaps().IsMultisamplingSupported())
        {
            config.samples = std::min(QualitySettingsSystem::Instance()->GetMSAAQuality(currentMSAA)->samples, rhi::DeviceCaps().maxSamples);
        }
        else
        {
            config.samples = 1;
        }

        config.priority = currentTarget.priority + basePriority;
        config.colorBuffer[0].targetTexture = currentTarget.colorAttachment;
        config.colorBuffer[0].loadAction = colorLoadAction;
        config.colorBuffer[0].storeAction = (config.samples > 1) ? rhi::STOREACTION_RESOLVE : rhi::STOREACTION_STORE;
        config.depthStencilBuffer.targetTexture = currentTarget.depthAttachment.IsValid() ? currentTarget.depthAttachment : rhi::DefaultDepthBuffer;
        config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        config.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    }

    scene->SetMainPassViewport(viewportRc);
    scene->Draw();

    if (drawToFrameBuffer)
    {
        RenderSystem2D::Instance()->DrawTexture(frameBuffer, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, geometricData.GetUnrotatedRect(), Rect(Vector2(), fbTexSize));
    }
}

bool UI3DView::IsClearRequested() const
{
    return colorLoadAction == rhi::LOADACTION_CLEAR;
}

void UI3DView::SetClearRequested(bool requested)
{
    if (requested)
    {
        colorLoadAction = rhi::LOADACTION_CLEAR;
    }
    else
    {
        colorLoadAction = rhi::LOADACTION_LOAD;
    }
}

void UI3DView::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
    float32 aspect = size.dx / size.dy;

    if (scene)
    {
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

UI3DView* UI3DView::Clone()
{
    UI3DView* ui3DView = new UI3DView(GetRect());
    ui3DView->CopyDataFrom(this);
    return ui3DView;
}

void UI3DView::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);

    UI3DView* srcView = DynamicTypeCheck<UI3DView*>(srcControl);
    drawToFrameBuffer = srcView->drawToFrameBuffer;
    fbScaleFactor = srcView->fbScaleFactor;
    fbRenderSize = srcView->fbRenderSize;
    fbTexSize = srcView->fbTexSize;
}

void UI3DView::Input(UIEvent* currentInput)
{
    if (scene)
    {
        scene->Input(currentInput);
    }

    UIControl::Input(currentInput);
}

void UI3DView::SetDrawToFrameBuffer(bool enable)
{
    drawToFrameBuffer = enable;

    if (!enable)
    {
        SafeRelease(frameBuffer);
    }
}

void UI3DView::SetFrameBufferScaleFactor(float32 scale)
{
    fbScaleFactor = scale;
}

void UI3DView::PrepareFrameBuffer()
{
    DVASSERT(scene);

    fbRenderSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(GetSize()) * fbScaleFactor;

    if (frameBuffer == nullptr || frameBuffer->GetWidth() < fbRenderSize.dx || frameBuffer->GetHeight() < fbRenderSize.dy)
    {
        SafeRelease(frameBuffer);
        int32 dx = static_cast<int32>(fbRenderSize.dx);
        int32 dy = static_cast<int32>(fbRenderSize.dy);
        frameBuffer = Texture::CreateFBO(dx, dy, FORMAT_RGBA8888, true);
    }

    Vector2 fbSize = Vector2(static_cast<float32>(frameBuffer->GetWidth()), static_cast<float32>(frameBuffer->GetHeight()));

    fbTexSize = fbRenderSize / fbSize;
}

void UI3DView::OnVisible()
{
    if (!registeredInUIControlSystem)
    {
        registeredInUIControlSystem = true;
        UIControlSystem::Instance()->UI3DViewAdded();
    }
}

void UI3DView::OnInvisible()
{
    if (registeredInUIControlSystem)
    {
        registeredInUIControlSystem = false;
        UIControlSystem::Instance()->UI3DViewRemoved();
    }
}
}