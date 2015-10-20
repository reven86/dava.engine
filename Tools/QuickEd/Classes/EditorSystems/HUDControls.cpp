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


#include "EditorSystems/HUDControls.h"

using namespace DAVA;

namespace
{
const Vector2 PIVOT_CONTROL_SIZE(20.0f, 20.0f);
const Vector2 FRAME_RECT_SIZE(15.0f, 15.0f);
const Vector2 ROTATE_CONTROL_SIZE(20.0f, 20.0f);
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

HUDContainer::HUDContainer(UIControl* container)
    : ControlContainer(HUDAreaInfo::NO_AREA)
    , control(container)
{
    SetName("HudContainer of " + container->GetName());
}

void HUDContainer::AddChild(ControlContainer* container)
{
    AddControl(container);
    childs.emplace_back(container);
}

void HUDContainer::InitFromGD(const UIGeometricData& gd)
{
    const Rect& ur = gd.GetUnrotatedRect();
    SetSize(ur.GetSize());
    SetPivot(control->GetPivot());
    SetRect(ur);
    SetAngle(gd.angle);
    bool valid_ = control->GetSystemVisible() && gd.size.dx > 0.0f && gd.size.dy > 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    SetValid(valid_);
    if (valid)
    {
        for (auto child : childs)
        {
            child->InitFromGD(gd);
        }
    }
}

void HUDContainer::SystemDraw(const UIGeometricData& geometricData)
{
    InitFromGD(control->GetGeometricData());
    UIControl::SystemDraw(geometricData);
}

void HUDContainer::SetValid(bool arg)
{
    valid = arg;
    SetVisible(valid && visibleInSystems);
}

void HUDContainer::SetVisibleInSystems(bool arg)
{
    visibleInSystems = arg;
    SetVisible(valid && visibleInSystems);
}

void FrameControl::Init()
{
    SetName("Frame Control");

    for (uint32 i = 0; i < BORDERS_COUNT; ++i)
    {
        UIControl* control = new UIControl();
        control->SetName("border of frame");
        UIControlBackground* background = control->GetBackground();
        background->SetSprite("~res:/Gfx/HUDControls/BlackGrid", 0);
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        AddControl(control);
    }
}

void FrameControl::InitFromGD(const UIGeometricData& geometricData)
{
    Rect rect = geometricData.GetUnrotatedRect();
    UIControl* parent = GetParent();
    DVASSERT(parent != nullptr);
    Vector2 parentPivotPoint = parent->GetPivot() * parent->GetSize();
    rect.SetPosition(rect.GetPosition() - geometricData.position + parentPivotPoint);
    SetRect(rect);

    auto& children = GetChildren();
    DVASSERT(children.size() == BORDERS_COUNT);
    auto chilrenIt = children.begin();
    for (uint32 i = 0; i < BORDERS_COUNT; ++i, ++chilrenIt)
    {
        Rect borderRect = CreateFrameBorderRect(i, rect);
        (*chilrenIt)->SetRect(borderRect);
    }
}

FrameControl::FrameControl()
    : ControlContainer(HUDAreaInfo::FRAME_AREA)
{
}

Rect FrameControl::CreateFrameBorderRect(uint32 border, const Rect& frameRect) const
{
    switch (border)
    {
    case BORDER_TOP:
        return Rect(frameRect.x, frameRect.y, frameRect.dx, 1.0f);
    case BORDER_BOTTOM:
        return Rect(frameRect.x, frameRect.y + frameRect.dy, frameRect.dx, 1.0f);
    case BORDER_LEFT:
        return Rect(frameRect.x, frameRect.y, 1.0f, frameRect.dy);
    case BORDER_RIGHT:
        return Rect(frameRect.x + frameRect.dx, frameRect.y, 1.0f, frameRect.dy);
    default:
        DVASSERT("!impossible value for frame control position");
        return Rect();
    }
}

FrameRectControl::FrameRectControl(const HUDAreaInfo::eArea area_)
    : ControlContainer(area_)
{
    SetName("Frame Rect Control");
    background->SetSprite("~res:/Gfx/HUDControls/Rect", 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
}

void FrameRectControl::InitFromGD(const UIGeometricData& geometricData)
{
    Rect rect(Vector2(), FRAME_RECT_SIZE);
    rect.SetCenter(GetPos(geometricData));

    UIControl* parent = GetParent();
    DVASSERT(parent != nullptr);
    Vector2 parentPivotPoint = parent->GetPivot() * parent->GetSize();
    rect.SetPosition(rect.GetPosition() - geometricData.position + parentPivotPoint);
    SetRect(rect);
}

Vector2 FrameRectControl::GetPos(const UIGeometricData& geometricData) const
{
    Rect rect = geometricData.GetUnrotatedRect();
    Vector2 retVal = rect.GetPosition();
    switch (area)
    {
    case HUDAreaInfo::TOP_LEFT_AREA:
        return retVal;
    case HUDAreaInfo::TOP_CENTER_AREA:
        return retVal + Vector2(rect.dx / 2.0f, 0.0f);
    case HUDAreaInfo::TOP_RIGHT_AREA:
        return retVal + Vector2(rect.dx, 0.0f);
    case HUDAreaInfo::CENTER_LEFT_AREA:
        return retVal + Vector2(0, rect.dy / 2.0f);
    case HUDAreaInfo::CENTER_RIGHT_AREA:
        return retVal + Vector2(rect.dx, rect.dy / 2.0f);
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
        return retVal + Vector2(0, rect.dy);
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
        return retVal + Vector2(rect.dx / 2.0f, rect.dy);
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return retVal + Vector2(rect.dx, rect.dy);
    default:
        DVASSERT(!"wrong area passed to hud control");
        return Vector2(0.0f, 0.0f);
    }
}

PivotPointControl::PivotPointControl()
    : ControlContainer(HUDAreaInfo::PIVOT_POINT_AREA)
{
    SetName("pivot point control");
    background->SetSprite("~res:/Gfx/HUDControls/Pivot", 0);
    background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
}

void PivotPointControl::InitFromGD(const UIGeometricData& geometricData)
{
    Rect rect(Vector2(), PIVOT_CONTROL_SIZE);
    const Rect& controlRect = geometricData.GetUnrotatedRect();
    rect.SetCenter(controlRect.GetPosition() + geometricData.pivotPoint * geometricData.scale);

    UIControl* parent = GetParent();
    DVASSERT(parent != nullptr);
    Vector2 parentPivotPoint = parent->GetPivot() * parent->GetSize();
    rect.SetPosition(rect.GetPosition() - geometricData.position + parentPivotPoint);
    SetRect(rect);
}

RotateControl::RotateControl()
    : ControlContainer(HUDAreaInfo::ROTATE_AREA)
{
    SetName("rotate control");
    background->SetSprite("~res:/Gfx/HUDControls/Rotate", 2);
    background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
}

void RotateControl::InitFromGD(const UIGeometricData& geometricData)
{
    Rect rect(Vector2(), ROTATE_CONTROL_SIZE);
    Rect controlRect = geometricData.GetUnrotatedRect();
    rect.SetCenter(Vector2(controlRect.GetPosition().x + controlRect.dx / 2.0f, controlRect.GetPosition().y - 20));

    UIControl* parent = GetParent();
    DVASSERT(parent != nullptr);
    Vector2 parentPivotPoint = parent->GetPivot() * parent->GetSize();
    rect.SetPosition(rect.GetPosition() - geometricData.position + parentPivotPoint);
    SetRect(rect);
}

void SelectionRect::Draw(const UIGeometricData& geometricData)
{
    Rect rect = geometricData.GetUnrotatedRect();
    rect.SetPosition(Vector2());
    auto& children = GetChildren();
    DVASSERT(children.size() == BORDERS_COUNT);
    auto chilrenIt = children.begin();
    for (uint32 i = 0; i < BORDERS_COUNT; ++i, ++chilrenIt)
    {
        Rect borderRect = CreateFrameBorderRect(i, rect);
        (*chilrenIt)->SetRect(borderRect);
    }
    UIControl::Draw(geometricData);
}
