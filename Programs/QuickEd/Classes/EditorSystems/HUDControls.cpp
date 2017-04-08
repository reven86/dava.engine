#include "UI/Layouts/UIAnchorComponent.h"
#include "EditorSystems/HUDControls.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "EditorSystems/EditorTransformSystem.h"

#include <Render/2D/Sprite.h>
#include <Render/2D/Systems/RenderSystem2D.h>

using namespace DAVA;

namespace HUDControlsDetails
{
const Vector2 PIVOT_CONTROL_SIZE(15.0f, 15.0f);
const Vector2 FRAME_RECT_SIZE(10.0f, 10.0f);
const Vector2 ROTATE_CONTROL_SIZE(15.0f, 15);

RefPtr<UIControl> CreateFrameBorderControl(FrameControl::eBorder border)
{
    RefPtr<UIControl> control(new UIControl(Rect(1.0f, 1.0f, 1.0f, 1.0f)));
    UIAnchorComponent* anchor = control->GetOrCreateComponent<UIAnchorComponent>();
    anchor->SetLeftAnchorEnabled(true);
    anchor->SetRightAnchorEnabled(true);
    anchor->SetTopAnchorEnabled(true);
    anchor->SetBottomAnchorEnabled(true);
    switch (border)
    {
    case FrameControl::TOP:
        anchor->SetBottomAnchorEnabled(false);
        break;
    case FrameControl::BOTTOM:
        anchor->SetTopAnchorEnabled(false);
        break;
    case FrameControl::LEFT:
        anchor->SetRightAnchorEnabled(false);
        break;
    case FrameControl::RIGHT:
        anchor->SetLeftAnchorEnabled(false);
        break;
    default:
        DVASSERT("!impossible value for frame control position");
    }
    return control;
}
}

ControlContainer::ControlContainer(const HUDAreaInfo::eArea area_)
    : UIControl()
    , area(area_)
{
}

HUDAreaInfo::eArea ControlContainer::GetArea() const
{
    return area;
}

void ControlContainer::SetSystemVisible(bool visible)
{
    systemVisible = visible;
}

bool ControlContainer::GetSystemVisible() const
{
    return systemVisible;
}

HUDContainer::HUDContainer(const ControlNode* node_)
    : ControlContainer(HUDAreaInfo::NO_AREA)
    , node(node_)
{
    DVASSERT(nullptr != node);
    SetName(FastName(String("HudContainer of ") + node->GetName().c_str()));
    control = node->GetControl();
    visibleProperty = node->GetRootProperty()->GetVisibleProperty();
    DVASSERT(nullptr != control && nullptr != visibleProperty);
}

void HUDContainer::AddChild(ControlContainer* container)
{
    AddControl(container);
}

void HUDContainer::InitFromGD(const UIGeometricData& gd)
{
    bool contolIsInValidState = systemVisible && gd.size.dx >= 0.0f && gd.size.dy >= 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    bool containerVisible = contolIsInValidState && visibleProperty->GetVisibleInEditor();
    if (containerVisible)
    {
        PackageBaseNode* parent = node->GetParent();
        while (containerVisible && nullptr != parent)
        {
            ControlNode* parentControlNode = dynamic_cast<ControlNode*>(parent);
            if (parentControlNode == nullptr)
            {
                break;
            }
            containerVisible &= parentControlNode->GetRootProperty()->GetVisibleProperty()->GetVisibleInEditor();
            parent = parent->GetParent();
        }
    }
    SetVisibilityFlag(containerVisible);
    if (containerVisible)
    {
        const DAVA::Vector2& minimumSize = EditorTransformSystem::GetMinimumSize();
        DAVA::Vector2 actualSize(gd.size * gd.scale);
        DAVA::UIGeometricData changedGD = gd;
        bool controlIsMoveOnly = actualSize.dx < minimumSize.dx && actualSize.dy < minimumSize.dy;
        if (controlIsMoveOnly)
        {
            changedGD.position -= ::Rotate((minimumSize - actualSize) / 2.0f, changedGD.angle);
            changedGD.size = minimumSize / gd.scale;
        }

        Rect ur(changedGD.position - ::Rotate(changedGD.pivotPoint, changedGD.angle) * changedGD.scale, changedGD.size * changedGD.scale);
        SetRect(ur);

        SetAngle(changedGD.angle);

        for (UIControl* child : GetChildren())
        {
            ControlContainer* childControlContainer = dynamic_cast<ControlContainer*>(child);
            if (nullptr != childControlContainer)
            {
                HUDAreaInfo::eArea area = childControlContainer->GetArea();
                bool childVisible = childControlContainer->GetSystemVisible() && changedGD.scale.x > 0.0f && changedGD.scale.y > 0.0f;
                if (area != HUDAreaInfo::FRAME_AREA)
                {
                    childVisible &= !controlIsMoveOnly;
                }
                childControlContainer->SetVisibilityFlag(childVisible);
                if (childVisible)
                {
                    childControlContainer->InitFromGD(changedGD);
                }
            }
        }
    }
}

void HUDContainer::SystemDraw(const UIGeometricData& gd, const UIControlBackground* parentBackground)
{
    auto controlGD = control->GetGeometricData();
    InitFromGD(controlGD);
    ControlContainer::SystemDraw(gd, parentBackground);
}

FrameControl::FrameControl(eType type_)
    : ControlContainer(HUDAreaInfo::FRAME_AREA)
    , type(type_)
{
    SetName(FastName("Frame Control"));
    for (uint32 i = 0; i < eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        RefPtr<UIControl> control(HUDControlsDetails::CreateFrameBorderControl(border));
        control->SetName(FastName(String("border of ") + GetName().c_str()));
        UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
        if (type == CHECKERED)
        {
            ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/BlackGrid/BlackGrid.png"));
            background->SetSprite(sprite, 0);
        }
        else
        {
            ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/MagnetLine/MagnetLine.png"));
            background->SetSprite(sprite, 0);
        }
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        AddControl(control.Get());
    }
}

void FrameControl::InitFromGD(const UIGeometricData& gd)
{
    SetRect(Rect(Vector2(0.0f, 0.0f), gd.size * gd.scale));
}

FrameRectControl::FrameRectControl(const HUDAreaInfo::eArea area_)
    : ControlContainer(area_)
{
    SetName(FastName("Frame Rect Control"));
    UIControlBackground* background = GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/Rect.png", true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void FrameRectControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), HUDControlsDetails::FRAME_RECT_SIZE);
    rect.SetCenter(GetPos(gd));
    SetRect(rect);
}

Vector2 FrameRectControl::GetPos(const UIGeometricData& gd) const
{
    Rect rect(Vector2(0.0f, 0.0f), gd.size * gd.scale);
    switch (area)
    {
    case HUDAreaInfo::TOP_LEFT_AREA:
        return Vector2(0.0f, 0.0f);
    case HUDAreaInfo::TOP_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, 0.0f);
    case HUDAreaInfo::TOP_RIGHT_AREA:
        return Vector2(rect.dx, 0.0f);
    case HUDAreaInfo::CENTER_LEFT_AREA:
        return Vector2(0, rect.dy / 2.0f);
    case HUDAreaInfo::CENTER_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy / 2.0f);
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
        return Vector2(0, rect.dy);
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, rect.dy);
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy);
    default:
        DVASSERT(!"wrong area passed to hud control");
        return Vector2(0.0f, 0.0f);
    }
}

PivotPointControl::PivotPointControl()
    : ControlContainer(HUDAreaInfo::PIVOT_POINT_AREA)
{
    SetName(FastName("pivot point control"));
    UIControlBackground* background = GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/Pivot.png", true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void PivotPointControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), HUDControlsDetails::PIVOT_CONTROL_SIZE);
    rect.SetCenter(gd.pivotPoint * gd.scale);
    SetRect(rect);
}

RotateControl::RotateControl()
    : ControlContainer(HUDAreaInfo::ROTATE_AREA)
{
    SetName(FastName("rotate control"));
    UIControlBackground* background = GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/Rotate.png", true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void RotateControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(0.0f, 0.0f), HUDControlsDetails::ROTATE_CONTROL_SIZE);
    Rect controlRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);

    const int margin = 5;
    rect.SetCenter(Vector2(controlRect.dx / 2.0f, controlRect.GetPosition().y - HUDControlsDetails::ROTATE_CONTROL_SIZE.y - margin));

    SetRect(rect);
}

void SetupHUDMagnetLineControl(UIControl* control)
{
    UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/HUDControls/MagnetLine/MagnetLine.png"));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    control->SetName("Magnet line");
}

void SetupHUDMagnetRectControl(UIControl* parentControl)
{
    for (int i = 0; i < FrameControl::eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        RefPtr<UIControl> control(HUDControlsDetails::CreateFrameBorderControl(border));
        SetupHUDMagnetLineControl(control.Get());
        control->SetName(FastName(String("border of magnet rect")));
        parentControl->AddControl(control.Get());
    }
}

RefPtr<UIControl> CreateHUDRect(const ControlNode* node)
{
    RefPtr<HUDContainer> container(new HUDContainer(node));
    container->SetName("HUD rect container");
    RefPtr<ControlContainer> frame(new FrameControl(FrameControl::UNIFORM));
    frame->SetName("HUD rect frame control");
    container->AddChild(frame.Get());

    container->InitFromGD(node->GetControl()->GetGeometricData());
    return container;
}
