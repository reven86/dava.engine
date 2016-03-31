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
    (IN gCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "UI/Layouts/UIAnchorComponent.h"
#include "EditorSystems/HUDControls.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

using namespace DAVA;

namespace
{
const Vector2 PIVOT_CONTROL_SIZE(15.0f, 15.0f);
const Vector2 FRAME_RECT_SIZE(10.0f, 10.0f);
const Vector2 ROTATE_CONTROL_SIZE(15.0f, 15);
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

HUDContainer::HUDContainer(ControlNode* node_)
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
    childs.emplace_back(container);
}

void HUDContainer::InitFromGD(const UIGeometricData& gd)
{
    bool contolIsInValidState = systemVisible && gd.size.dx >= 0.0f && gd.size.dy >= 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    bool valid = contolIsInValidState && visibleProperty->GetVisibleInEditor();
    if (valid)
    {
        PackageBaseNode* parent = node->GetParent();
        while (valid && nullptr != parent)
        {
            ControlNode* parentControlNode = dynamic_cast<ControlNode*>(parent);
            if (parentControlNode == nullptr)
            {
                break;
            }
            valid &= parentControlNode->GetRootProperty()->GetVisibleProperty()->GetVisibleInEditor();
            parent = parent->GetParent();
        }
    }
    SetVisibilityFlag(valid);
    if (valid)
    {
        auto actualSize = gd.size * gd.scale;
        auto changedGD = gd;
        bool controlIsMoveOnly = actualSize.dx < minimumSize.dx && actualSize.dy < minimumSize.dy;
        if (controlIsMoveOnly)
        {
            changedGD.position -= ::Rotate((minimumSize - actualSize) / 2.0f, changedGD.angle);
            changedGD.size = minimumSize / gd.scale;
        }

        Rect ur(changedGD.position - ::Rotate(changedGD.pivotPoint, changedGD.angle) * changedGD.scale, changedGD.size * changedGD.scale);
        SetRect(ur);

        SetAngle(changedGD.angle);

        for (auto child : childs)
        {
            if (child->GetArea() != HUDAreaInfo::FRAME_AREA)
            {
                bool visible = gd.scale.x > 0.0f && gd.scale.y > 0.0f && !controlIsMoveOnly;
                child->SetVisibilityFlag(systemVisible && visible);
            }

            child->InitFromGD(changedGD);
        }
    }
}

void HUDContainer::SystemDraw(const UIGeometricData& gd)
{
    auto controlGD = control->GetGeometricData();
    InitFromGD(controlGD);
    UIControl::SystemDraw(gd);
}

FrameControl::FrameControl()
    : ControlContainer(HUDAreaInfo::FRAME_AREA)
{
    SetName(FastName("Frame Control"));
    for (uint32 i = 0; i < BORDERS_COUNT; ++i)
    {
        ScopedPtr<UIControl> control(CreateFrameBorderControl(i));
        control->SetName(FastName(String("border of ") + GetName().c_str()));
        UIControlBackground* background = control->GetBackground();
        background->SetSprite("~res:/Gfx/HUDControls/BlackGrid/BlackGrid", 0);
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        AddControl(control);
    }
}

void FrameControl::InitFromGD(const UIGeometricData& gd)
{
    SetRect(Rect(Vector2(0.0f, 0.0f), gd.size * gd.scale));
}

UIControl* FrameControl::CreateFrameBorderControl(uint32 border)
{
    UIControl* control = new UIControl(Rect(1.0f, 1.0f, 1.0f, 1.0f));
    UIAnchorComponent* anchor = control->GetOrCreateComponent<UIAnchorComponent>();
    anchor->SetLeftAnchorEnabled(true);
    anchor->SetRightAnchorEnabled(true);
    anchor->SetTopAnchorEnabled(true);
    anchor->SetBottomAnchorEnabled(true);
    switch (border)
    {
    case BORDER_TOP:
        anchor->SetBottomAnchorEnabled(false);
        break;
    case BORDER_BOTTOM:
        anchor->SetTopAnchorEnabled(false);
        break;
    case BORDER_LEFT:
        anchor->SetRightAnchorEnabled(false);
        break;
    case BORDER_RIGHT:
        anchor->SetLeftAnchorEnabled(false);
        break;
    default:
        DVASSERT("!impossible value for frame control position");
    }
    return control;
}

FrameRectControl::FrameRectControl(const HUDAreaInfo::eArea area_)
    : ControlContainer(area_)
{
    SetName(FastName("Frame Rect Control"));
    background->SetSprite("~res:/Gfx/HUDControls/Rect", 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void FrameRectControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), FRAME_RECT_SIZE);
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
    background->SetSprite("~res:/Gfx/HUDControls/Pivot", 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void PivotPointControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), PIVOT_CONTROL_SIZE);
    rect.SetCenter(gd.pivotPoint * gd.scale);
    SetRect(rect);
}

RotateControl::RotateControl()
    : ControlContainer(HUDAreaInfo::ROTATE_AREA)
{
    SetName(FastName("rotate control"));
    background->SetSprite("~res:/Gfx/HUDControls/Rotate", 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void RotateControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(0.0f, 0.0f), ROTATE_CONTROL_SIZE);
    Rect controlRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);

    const int margin = 5;
    rect.SetCenter(Vector2(controlRect.dx / 2.0f, controlRect.GetPosition().y - ROTATE_CONTROL_SIZE.y - margin));

    SetRect(rect);
}

void SetupHUDMagnetLineControl(UIControl* control)
{
    control->GetBackground()->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
    control->GetBackground()->SetSprite("~res:/Gfx/HUDControls/MagnetLine/MagnetLine", 0);
    control->GetBackground()->SetDrawType(UIControlBackground::DRAW_TILED);
}

void SetupHUDMagnetRectControl(UIControl* parentControl)
{
    const int bordersCount = 4;
    for (int i = 0; i < bordersCount; ++i)
    {
        ScopedPtr<UIControl> control(FrameControl::CreateFrameBorderControl(i));
        SetupHUDMagnetLineControl(control);
        control->SetName(FastName(String("border of magnet rect")));
        parentControl->AddControl(control);
    }
}