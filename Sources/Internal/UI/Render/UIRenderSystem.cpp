#include "UIRenderSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/Render/UIClipContentComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"
#include "UI/Render/UISceneComponent.h"
#include "UI/UIControl.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenTransition.h"
#include "UI/UIScreenshoter.h"

namespace DAVA
{
UIRenderSystem::UIRenderSystem(RenderSystem2D* renderSystem2D_)
    : renderSystem2D(renderSystem2D_)
    , screenshoter(std::make_unique<UIScreenshoter>())
{
    baseGeometricData.position = Vector2(0, 0);
    baseGeometricData.size = Vector2(0, 0);
    baseGeometricData.pivotPoint = Vector2(0, 0);
    baseGeometricData.scale = Vector2(1.0f, 1.0f);
    baseGeometricData.angle = 0;
}

UIRenderSystem::~UIRenderSystem() = default;

void UIRenderSystem::OnControlVisible(UIControl* control)
{
    uint32 cmpCount = control->GetComponentCount<UISceneComponent>();
    ui3DViewCount += cmpCount;
}

void UIRenderSystem::OnControlInvisible(UIControl* control)
{
    uint32 cmpCount = control->GetComponentCount<UISceneComponent>();
    ui3DViewCount -= cmpCount;

    DVASSERT(ui3DViewCount >= 0);
}

void UIRenderSystem::Process(float32 elapsedTime)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = renderSystem2D->GetMainTargetDescriptor();
    newDescr.clearTarget = (ui3DViewCount == 0 || currentScreenTransition.Valid()) && needClearMainPass;
    renderSystem2D->SetMainTargetDescriptor(newDescr);
}

void UIRenderSystem::Render()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_RENDER_SYSTEM);

    if (currentScreenTransition.Valid())
    {
        RenderControlHierarhy(currentScreenTransition.Get(), baseGeometricData, nullptr);
    }
    else if (currentScreen.Valid())
    {
        RenderControlHierarhy(currentScreen.Get(), baseGeometricData, nullptr);
    }

    if (popupContainer.Valid())
    {
        RenderControlHierarhy(popupContainer.Get(), baseGeometricData, nullptr);
    }

    screenshoter->OnFrame();
}

void UIRenderSystem::ForceRenderControl(UIControl* control)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_RENDER_SYSTEM);

    RenderControlHierarhy(control, baseGeometricData, nullptr);
}

const UIGeometricData& UIRenderSystem::GetBaseGeometricData() const
{
    return baseGeometricData;
}

UIScreenshoter* UIRenderSystem::GetScreenshoter() const
{
    return screenshoter.get();
}

int32 UIRenderSystem::GetUI3DViewCount() const
{
    return ui3DViewCount;
}

void UIRenderSystem::SetClearColor(const Color& clearColor)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = renderSystem2D->GetMainTargetDescriptor();
    newDescr.clearColor = clearColor;
    renderSystem2D->SetMainTargetDescriptor(newDescr);
}

void UIRenderSystem::SetUseClearPass(bool useClearPass)
{
    needClearMainPass = useClearPass;
}

void UIRenderSystem::SetCurrentScreen(const RefPtr<UIScreen>& _screen)
{
    currentScreen = _screen;
}

void UIRenderSystem::SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& _screenTransition)
{
    currentScreenTransition = _screenTransition;
}

void UIRenderSystem::SetPopupContainer(const RefPtr<UIControl>& _popupContainer)
{
    popupContainer = _popupContainer;
}

void UIRenderSystem::RenderControlHierarhy(UIControl* control, const UIGeometricData& geometricData, const UIControlBackground* parentBackground)
{
    if (!control->GetVisibilityFlag() || control->IsHiddenForDebug())
        return;

    UIGeometricData drawData = control->GetLocalGeometricData();
    drawData.AddGeometricData(geometricData);

    const Color& parentColor = parentBackground ? parentBackground->GetDrawColor() : Color::White;

    control->SetParentColor(parentColor);

    const Rect& unrotatedRect = drawData.GetUnrotatedRect();

    bool clipContents = (control->GetComponentCount<UIClipContentComponent>() != 0);
    if (clipContents)
    { //WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
        renderSystem2D->PushClip();
        renderSystem2D->IntersectClipRect(unrotatedRect); //anyway it doesn't work with rotation
    }

    control->Draw(drawData);

    const UIControlBackground* bg = control->GetComponent<UIControlBackground>();
    const UIControlBackground* parentBgForChild = bg ? bg : parentBackground;
    control->isIteratorCorrupted = false;
    for (UIControl* child : control->GetChildren())
    {
        RenderControlHierarhy(child, drawData, parentBgForChild);
        DVASSERT(!control->isIteratorCorrupted);
    }

    control->DrawAfterChilds(drawData);

    if (clipContents)
    {
        renderSystem2D->PopClip();
    }

    const UIDebugRenderComponent* debugRenderComponent = control->GetComponent<UIDebugRenderComponent>();
    if (debugRenderComponent && debugRenderComponent->IsEnabled())
    {
        DebugRender(debugRenderComponent, drawData, unrotatedRect);
    }
}

void UIRenderSystem::DebugRender(const UIDebugRenderComponent* component, const UIGeometricData& geometricData, const Rect& unrotatedRect)
{
    renderSystem2D->PushClip();
    renderSystem2D->RemoveClip();
    RenderDebugRect(component, geometricData);
    UIDebugRenderComponent::ePivotPointDrawMode drawMode = component->GetPivotPointDrawMode();
    if (drawMode != UIDebugRenderComponent::DRAW_NEVER &&
        (drawMode != UIDebugRenderComponent::DRAW_ONLY_IF_NONZERO || !component->GetControl()->GetPivotPoint().IsZero()))
    {
        RenderPivotPoint(component, unrotatedRect);
    }
    renderSystem2D->PopClip();
}

void UIRenderSystem::RenderDebugRect(const UIDebugRenderComponent* component, const UIGeometricData& gd)
{
    const Color& drawColor = component->GetDrawColor();

    if (gd.angle != 0.0f)
    {
        Polygon2 poly;
        gd.GetPolygon(poly);

        renderSystem2D->DrawPolygon(poly, true, drawColor);
    }
    else
    {
        renderSystem2D->DrawRect(gd.GetUnrotatedRect(), drawColor);
    }
}

void UIRenderSystem::RenderPivotPoint(const UIDebugRenderComponent* component, const Rect& drawRect)
{
    static const float32 PIVOT_POINT_MARK_RADIUS = 10.0f;
    static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 13.0f;

    const Color& drawColor = component->GetDrawColor();

    Vector2 pivotPointCenter = drawRect.GetPosition() + component->GetControl()->GetPivotPoint();
    renderSystem2D->DrawCircle(pivotPointCenter, PIVOT_POINT_MARK_RADIUS, drawColor);

    // Draw the cross mark.
    Vector2 lineStartPoint = pivotPointCenter;
    Vector2 lineEndPoint = pivotPointCenter;
    lineStartPoint.y -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.y += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    renderSystem2D->DrawLine(lineStartPoint, lineEndPoint, drawColor);

    lineStartPoint = pivotPointCenter;
    lineEndPoint = pivotPointCenter;
    lineStartPoint.x -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.x += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    renderSystem2D->DrawLine(lineStartPoint, lineEndPoint, drawColor);
}
}
