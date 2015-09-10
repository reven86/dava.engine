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

#include "Document.h"
#include "HUDSystem.h"

#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

using namespace DAVA;

namespace
{
    const Vector2 PIVOT_CONTROL_SIZE(5.0f, 5.0f);
    const Vector2 FRAME_RECT_SIZE(10.0f, 10.0f);
    const Vector2 ROTATE_CONTROL_SIZE(20.0f, 20.0f);
}

ControlContainer::ControlContainer(const HUDareaInfo::eArea area_)
    : UIControl()
    , area(area_)
{
}

HUDareaInfo::eArea ControlContainer::GetArea() const
{
    return area;
}

class HUDContainer : public ControlContainer
{
public:
    explicit HUDContainer(UIControl *container)
        : ControlContainer(HUDareaInfo::NO_AREA)
        , control(container)
    {
    }
    void AddChild(ControlContainer *container)
    {
        AddControl(container);
        childs.push_back(container);
    }
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        SetPivot(control->GetPivot());
        SetAbsoluteRect(geometricData.GetUnrotatedRect());
        SetAngle(geometricData.angle);
        for (auto child : childs)
        {
            child->InitFromGD(geometricData);
        }
    }

    void SystemDraw(const UIGeometricData &geometricData) override
    {
        InitFromGD(control->GetGeometricData());
        UIControl::SystemDraw(geometricData);
    }
private:
    UIControl *control = nullptr;
    Vector<ControlContainer*> childs;
};

class FrameControl : public ControlContainer
{
public:
    explicit FrameControl()
        : ControlContainer(HUDareaInfo::FRAME_AREA)
    {
        SetDebugDraw(true);
        SetDebugDrawColor(Color(0.0f, 0.0f, 1.0f, 1.0f));
    }
private:
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        SetAbsoluteRect(geometricData.GetUnrotatedRect());
    }
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const HUDareaInfo::eArea area_)
        : ControlContainer(area_)
    {
        SetDebugDraw(true);
        SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.f));
    }
private:
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        Rect rect(Vector2(0.0f, 0.0f), FRAME_RECT_SIZE);
        rect.SetCenter(GetPos(geometricData));
        SetAbsoluteRect(rect);
    }

    Vector2 GetPos(const UIGeometricData &geometricData) const
    {
        Rect rect = geometricData.GetUnrotatedRect();
        Vector2 retVal = rect.GetPosition();
        switch (area)
        {
        case HUDareaInfo::TOP_LEFT_AREA: return retVal;
        case HUDareaInfo::TOP_CENTER_AREA: return retVal + Vector2(rect.dx / 2.0f, 0.0f);
        case HUDareaInfo::TOP_RIGHT_AREA: return retVal + Vector2(rect.dx, 0.0f);
        case HUDareaInfo::CENTER_LEFT_AREA: return retVal + Vector2(0, rect.dy / 2.0f);
        case HUDareaInfo::CENTER_RIGHT_AREA: return retVal + Vector2(rect.dx, rect.dy / 2.0f);
        case HUDareaInfo::BOTTOM_LEFT_AREA: return retVal + Vector2(0, rect.dy);
        case HUDareaInfo::BOTTOM_CENTER_AREA: return retVal + Vector2(rect.dx / 2.0f, rect.dy);
        case HUDareaInfo::BOTTOM_RIGHT_AREA: return retVal + Vector2(rect.dx, rect.dy);
        default: DVASSERT_MSG(false, "what are you doing here?!"); return Vector2(0.0f, 0.0f);
        }
    }
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl()
        : ControlContainer(HUDareaInfo::PIVOT_POINT_AREA)
    {
        SetDebugDraw(true);
        SetDebugDrawColor(Color(1.0f, 0.0f, 0.0f, 1.f));
    }
private:
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        Rect rect(Vector2(0.0f, 0.0f), PIVOT_CONTROL_SIZE);
        const Rect &controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(controlRect.GetPosition() + geometricData.pivotPoint);
        SetAbsoluteRect(rect);
    }
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl()
        : ControlContainer(HUDareaInfo::ROTATE_AREA)
    {
        SetDebugDraw(true);
        SetDebugDrawColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
private:
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        Rect rect(Vector2(0.0f, 0.0f), ROTATE_CONTROL_SIZE);
        Rect controlRect = geometricData.GetUnrotatedRect();
        rect.SetCenter(Vector2(controlRect.GetPosition().x + controlRect.dx / 2.0f, controlRect.GetPosition().y - 20));
        SetAbsoluteRect(rect);
    }
};

HUDSystem::HUDSystem(Document *document_)
    : BaseSystem(document_)
    , hudControl(new UIControl())
    , selectionRectControl(new UIControl())
{
    selectionRectControl->SetDebugDraw(true);
    selectionRectControl->SetDebugDrawColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
    hudControl->AddControl(selectionRectControl);
    hudControl->SetName("hudControl");
    document->SelectionChanged.Connect(this, &HUDSystem::SetSelection);
}

void HUDSystem::OnActivated()
{
    if (!document->IsInEmulationMode())
    {
        document->GetRootControl()->AddControl(hudControl);
    }
}

void HUDSystem::OnDeactivated()
{
    hudControl->RemoveFromParent();
    canDrawRect = false;
    selectionRectControl->SetSize(Vector2(0.0f, 0.0f));
}

void HUDSystem::SetSelection(const SelectedControls& selected, const SelectedControls& deselected)
{
    for (auto control : deselected)
    {
        hudMap.erase(control);
    }
    for (auto control : selected)
    {
        hudMap.emplace(std::piecewise_construct, 
            std::forward_as_tuple(control),
            std::forward_as_tuple(control, hudControl));
    }
}

bool HUDSystem::OnInput(UIEvent *currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::PHASE_MOVE:
        ProcessCursor(currentInput->point);
        return false;
    case UIEvent::PHASE_BEGAN:
        canDrawRect = InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_CTRL);
        pressedPoint = currentInput->point;
        return canDrawRect;
    case UIEvent::PHASE_DRAG:
        if(canDrawRect)
        {
            Vector2 point(currentInput->point);
            Vector2 size(point - pressedPoint);
            selectionRectControl->SetAbsoluteRect(Rect(pressedPoint, size));
            document->SelectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        return canDrawRect;
    case UIEvent::PHASE_ENDED:
        ProcessCursor(currentInput->point);
        if (canDrawRect)
        {
            selectionRectControl->SetSize(Vector2(0, 0));
        }
        bool retVal = canDrawRect;
        canDrawRect = false;
        return retVal;
    }
    return false;
}

void HUDSystem::OnEmulationModeChanged(bool emulationMode)
{
    if (emulationMode)
    {
        document->GetRootControl()->RemoveControl(hudControl);
    }
    else
    {
        document->GetRootControl()->AddControl(hudControl);
    }
}

void HUDSystem::ProcessCursor(const Vector2& pos)
{
    SetNewArea(GetControlArea(pos));
}

HUDareaInfo HUDSystem::GetControlArea(const Vector2 &pos)
{
    HUDareaInfo areaInfo;
    for (const auto &iter : hudMap)
    {
        auto &hud = iter.second;
        for (const auto &hudControl : hud.hudControls)
        {
            if (hudControl->IsPointInside(pos))
            {
                auto container = hudControl.get();
                areaInfo.owner = hud.node;
                areaInfo.area = container->GetArea();
                DVASSERT_MSG(areaInfo.area != HUDareaInfo::NO_AREA && areaInfo.owner != nullptr
                             , "no control node for area");
                return areaInfo;
            }
        }
    }
    return areaInfo;
}

void HUDSystem::SetNewArea(const HUDareaInfo &areaInfo)
{
    if (activeAreaInfo.area != areaInfo.area
        || activeAreaInfo.owner != areaInfo.owner)
    {
        activeAreaInfo = areaInfo;
        document->ActiveAreaChanged.Emit(activeAreaInfo);
    }
}

HUDSystem::HUD::HUD(ControlNode *node_, UIControl* hudControl)
    : node(node_)
    , control(node_->GetControl())
    , container(new HUDContainer(control))
{
    hudControls.emplace_back(new PivotPointControl);
    hudControls.emplace_back(new RotateControl);
    for (int i = HUDareaInfo::TOP_LEFT_AREA; i < HUDareaInfo::CORNERS_COUNT; ++i)
    {
        HUDareaInfo::eArea area = static_cast<HUDareaInfo::eArea>(i);
        hudControls.emplace_back(new FrameRectControl(area));
    }
    hudControls.emplace_back(new FrameControl);

    hudControl->AddControl(container);
    auto hudContainer = static_cast<HUDContainer*>(container.get());
    for (auto iter = hudControls.rbegin(); iter != hudControls.rend(); ++iter)
    {
        hudContainer->AddChild((*iter).get());
    }
    container->InitFromGD(control->GetGeometricData());
}

HUDSystem::HUD::~HUD()
{
    container->RemoveFromParent();
    for (auto control : hudControls)
    {
        control->RemoveFromParent();
    }
}