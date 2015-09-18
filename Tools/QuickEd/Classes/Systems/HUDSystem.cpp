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
#include "UI/Layouts/UISizePolicyComponent.h"
#include "Render/RenderState.h"
#include "Render/RenderManager.h"

using namespace DAVA;

namespace
{
const Vector2 PIVOT_CONTROL_SIZE(20.0f, 20.0f);
const Vector2 FRAME_RECT_SIZE(20.0f, 20.0f);
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
    enum
    {
        BORDER_TOP,
        BORDER_BOTTOM,
        BORDER_LEFT,
        BORDER_RIGHT,
        BORDERS_COUNT
    };
    explicit FrameControl()
        : ControlContainer(HUDareaInfo::FRAME_AREA)
    {
        for (int i = 0; i < BORDERS_COUNT; ++i)
        {
            UIControl* control = new UIControl();
            control->GetBackground()->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            control->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
            borders.emplace_back(control);
        }
    }
private:
    void InitFromGD(const UIGeometricData &geometricData) override
    {
        const Rect& rect = geometricData.GetUnrotatedRect();
        SetAbsoluteRect(rect);
        for (int i = 0; i < BORDERS_COUNT; ++i)
        {
            if (firstInit)
            {
                GetParent()->AddControl(borders[i]);
            }
            Rect borderRect = CreateFrameBorderRect(i, rect);
            borders[i]->SetAbsoluteRect(borderRect);
        }
        firstInit = false;
    }
    Rect CreateFrameBorderRect(int border, const Rect& frameRect) const
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
    bool firstInit = true;
    DAVA::Vector<ScopedPtr<UIControl>> borders;
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const HUDareaInfo::eArea area_)
        : ControlContainer(area_)
    {
        background->SetSprite("~res:/Gfx/HUDControls/HUDControls2", 0);
        background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
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
        background->SetSprite("~res:/Gfx/HUDControls/HUDControls2", 1);
        background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
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
        background->SetSprite("~res:/Gfx/HUDControls/HUDControls2", 2);
        background->SetDrawType(UIControlBackground::DRAW_ALIGNED);
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
    document->EmulationModeChangedSignal.Connect(this, &HUDSystem::OnEmulationModeChanged);
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

void HUDSystem::SetSelection(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    Set<ControlNode*> deselectedControls = SelectionTracker::GetSetTFromSetU<Set<ControlNode*>>(deselected);
    for (auto control : deselectedControls)
    {
        hudMap.erase(control);
    }

    Set<ControlNode*> selectedControls = SelectionTracker::GetSetTFromSetU<Set<ControlNode*>>(selected);
    for (auto control : selectedControls)
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
        dragRequested = true;
        if(canDrawRect)
        {
            Vector2 point(currentInput->point);
            Vector2 size(point - pressedPoint);
            selectionRectControl->SetAbsoluteRect(Rect(pressedPoint, size));
            document->SelectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        return true;
    case UIEvent::PHASE_ENDED:
        ProcessCursor(currentInput->point);
        if (canDrawRect)
        {
            selectionRectControl->SetSize(Vector2(0, 0));
        }
        bool retVal = canDrawRect || dragRequested;
        canDrawRect = false;
        dragRequested = false;
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