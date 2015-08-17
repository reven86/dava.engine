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

#include "HUDSystem.h"

#include "UI/UIControl.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Render/RenderState.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

using namespace DAVA;

class ControlContainer : public UIControl
{
public:
    explicit ControlContainer(UIControl *container)
        : UIControl()
        , control(container)
    {}
protected:
    UIControl *control;
};

class FrameControl : public ControlContainer
{
public:
    explicit FrameControl(UIControl *container)
        : ControlContainer(container)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        SetRect(control->GetGeometricData().GetUnrotatedRect());
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.f));
        RenderHelper::Instance()->DrawRect(GetRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};

class HUDControl : public UIControl
{
public:
};

class FrameRectControl : public ControlContainer
{
public:
    enum PLACE
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
        COUNT
    };
    FrameRectControl(PLACE place_, UIControl *container)
        : ControlContainer(container)
        , place(place_)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Rect rect(0, 0, 5, 5);
        rect.SetCenter(GetPos());
        SetRect(rect);

        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.f));
        RenderHelper::Instance()->FillRect(GetRect(), RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }

    Vector2 FrameRectControl::GetPos()
    {
        Rect rect = control->GetGeometricData().GetUnrotatedRect();
        Vector2 retVal = rect.GetPosition();
        switch (place)
        {
        case TOP_LEFT: return retVal;
        case TOP_CENTER: return retVal + Vector2(rect.dx / 2.0f, 0);
        case TOP_RIGHT: return retVal + Vector2(rect.dx, 0);
        case CENTER_LEFT: return retVal + Vector2(0, rect.dy / 2.0f);
        case CENTER_RIGHT: return retVal + Vector2(rect.dx, rect.dy / 2.0f);
        case BOTTOM_LEFT: return retVal + Vector2(0, rect.dy);
        case BOTTOM_CENTER: return retVal + Vector2(rect.dx / 2.0f, rect.dy);
        case BOTTOM_RIGHT: return retVal + Vector2(rect.dx, rect.dy);
        default: DVASSERT_MSG(false, "what are you doing here?!"); return Vector2(0, 0);
        }
    }
    PLACE place;
};

class PivotPointControl : public ControlContainer
{
public:
    PivotPointControl(UIControl *container)
        : ControlContainer(container)
    { }
private:
    void Draw(const UIGeometricData &geometricData) override
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->SetColor(Color(0.0f, 0.0f, 1.0f, 1.f));
        Rect rect(0, 0, 5, 5);
        rect.SetCenter(control->GetGeometricData().GetUnrotatedRect().GetPosition() + control->GetPivotPoint());
        RenderHelper::Instance()->FillRect(rect, RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(oldColor);
    }
};


HUDSystem::HUDSystem()
    : hudControl(new UIControl())
    , selectionRect(new UIControl())
{
}

void HUDSystem::Attach(UIControl* root)
{
    root->AddControl(hudControl);
}

void HUDSystem::SelectionWasChanged(const SelectedControls& selected, const SelectedControls& deselected)
{
    for (auto control : deselected)
    {
        hudMap.erase(control);
    }
    for (auto control : selected)
    {
        hudMap.emplace(std::piecewise_construct, 
            std::forward_as_tuple(control),
            std::forward_as_tuple(control->GetControl(), hudControl));
    }
}

bool HUDSystem::OnInput(DAVA::UIEvent *currentInput)
{
    return false;
}

HUDSystem::HUD::HUD(UIControl* control_, UIControl* hudControl_)
    : frame(new FrameControl(control_))
    , control(control_)
    , hudControl(hudControl_)
{
    hudControl->AddControl(frame);
    
    for (int i = FrameRectControl::TOP_LEFT; i < FrameRectControl::COUNT; ++i)
    {
        ScopedPtr<UIControl> littleRectFrame(new FrameRectControl(static_cast<FrameRectControl::PLACE>(i), control_));
        frameRects.push_back(littleRectFrame);
        hudControl->AddControl(littleRectFrame);
    }
}

HUDSystem::HUD::~HUD()
{
    hudControl->RemoveControl(frame);
    for (auto frameRect : frameRects)
    {
        hudControl->RemoveControl(frameRect);
    }
}