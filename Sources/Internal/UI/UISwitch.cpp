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


#include "UISwitch.h"
#include "Animation/LinearAnimation.h"
#include "UI/UIEvent.h"

namespace DAVA 
{

//use these names for children controls to define UISwitch in .yaml
static const String UISWITCH_BUTTON_LEFT_NAME = "buttonLeft";
static const String UISWITCH_BUTTON_RIGHT_NAME = "buttonRight";
static const String UISWITCH_BUTTON_TOGGLE_NAME = "buttonToggle";
static const float32 SWITCH_ANIMATION_TIME = 0.1f;
static const int32 MOVE_ANIMATION_TRACK = 10;
static const float32 ANCHOR_UNDEFINED = 10000;
static float32 dragAnchorX = ANCHOR_UNDEFINED;

class TogglePositionAnimation : public LinearAnimation<float32>
{
protected:
    virtual ~TogglePositionAnimation()
    {
        SafeRelease(uiSwitch);
    }
public:
    TogglePositionAnimation(bool _isCausedByTap, UISwitch * _uiSwitch, float32 * _var, float32 _endValue, float32 _animationTimeLength, Interpolation::FuncType _iType)
        : LinearAnimation(_uiSwitch->GetToggle(), _var, _endValue, _animationTimeLength, _iType)
        , uiSwitch(SafeRetain(_uiSwitch))
        , isFromLeftToRight(false)
        , centerNotPassed(_isCausedByTap) //center is not yet passed by in this case
        , centerPos(0.f)
    {
        if (_isCausedByTap) //toggle is on opposite side from _endValue, we can calculate center
        {
            centerPos = (_endValue + *_var) / 2;
            isFromLeftToRight = _endValue > *_var;
        }
    }
    
    virtual void Update(float32 timeElapsed)
    {
        LinearAnimation::Update(timeElapsed);
        if (centerNotPassed)
        {
            if (isFromLeftToRight ^ (*var < centerPos))
            {
                centerNotPassed = false;
                uiSwitch->ChangeVisualState();
            }
        }
    }

private:
    UISwitch * uiSwitch;
    bool isFromLeftToRight;
    bool centerNotPassed;
    float32 centerPos;
};

UISwitch::UISwitch(const Rect &rect) 
    : UIControl(rect)
    , buttonLeft(new UIButton())
    , buttonRight(new UIButton())
    , toggle(new UIButton())
    , switchOnTapBesideToggle(true)
{
    buttonLeft->SetName(UISWITCH_BUTTON_LEFT_NAME);
    buttonRight->SetName(UISWITCH_BUTTON_RIGHT_NAME);
    toggle->SetName(UISWITCH_BUTTON_TOGGLE_NAME);
    AddControl(buttonLeft.Get());
    AddControl(buttonRight.Get());
    AddControl(toggle.Get());
    InitControls();

    Vector2 leftAndRightSize(size.dx / 2, size.dy);
    buttonLeft->SetSize(leftAndRightSize);
    buttonRight->SetSize(leftAndRightSize);
    Vector2 newPivotPoint = buttonRight->GetPivotPoint();
    newPivotPoint.x = leftAndRightSize.dx;
    buttonRight->SetPivotPoint(newPivotPoint);
    buttonRight->SetPosition(Vector2(size.x, buttonRight->relativePosition.y));
}

UISwitch::~UISwitch()
{
}

void UISwitch::InitControls()
{
    buttonLeft->SetInputEnabled(false);
    buttonRight->SetInputEnabled(false);
    toggle->SetInputEnabled(false);
    BringChildFront(toggle.Get());
    CheckToggleSideChange();
    float32 toggleXPosition = GetToggleUttermostPosition();
    toggle->SetPosition(Vector2(toggleXPosition, toggle->relativePosition.y));
    ChangeVisualState();//forcing visual state change cause it can be skipped in CheckToggleSideChange()
}

void UISwitch::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    //release default buttons - they have to be loaded from yaml
    buttonLeft = nullptr;
    buttonRight = nullptr;
    toggle = nullptr;

    UIControl::LoadFromYamlNode(node, loader);
}

YamlNode * UISwitch::SaveToYamlNode(UIYamlLoader * loader)
{
    buttonLeft->SetName(UISWITCH_BUTTON_LEFT_NAME);
	toggle->SetName(UISWITCH_BUTTON_TOGGLE_NAME);
	buttonRight->SetName(UISWITCH_BUTTON_RIGHT_NAME);

	YamlNode *node = UIControl::SaveToYamlNode(loader);
	return node;
}

void UISwitch::AddControl(UIControl *control)
{
	// Synchronize the pointers to the buttons each time new control is added.
	UIControl::AddControl(control);

    if (control->GetName() == UISWITCH_BUTTON_LEFT_NAME && buttonLeft.Get() != control)
	{
		buttonLeft = DynamicTypeCheck<UIButton*>(control);
	}
    else if (control->GetName() == UISWITCH_BUTTON_TOGGLE_NAME && toggle.Get() != control)
	{
        toggle = DynamicTypeCheck<UIButton*>(control);
	}
    else if (control->GetName() == UISWITCH_BUTTON_RIGHT_NAME && buttonRight.Get() != control)
	{
        buttonRight = DynamicTypeCheck<UIButton*>(control);
	}
}

void UISwitch::RemoveControl(UIControl *control)
{
    if (control == buttonRight.Get())
    {
        buttonRight = nullptr;
    }
    else if (control == buttonLeft.Get())
    {
        buttonLeft = nullptr;
    }
    else if (control == toggle.Get())
    {
        toggle = nullptr;
    }
    
    UIControl::RemoveControl(control);
}

void UISwitch::CopyDataFrom(UIControl *srcControl)
{
	//release default buttons - they have to be copied from srcControl
    buttonLeft = nullptr;
    buttonRight = nullptr;
    toggle = nullptr;

	UIControl::CopyDataFrom(srcControl);

    InitControls();
}

UISwitch* UISwitch::Clone()
{
	UISwitch *t = new UISwitch(GetRect());
	t->CopyDataFrom(this);
	return t;
}

void UISwitch::LoadFromYamlNodeCompleted()
{
    InitControls();
}

void UISwitch::Input(UIEvent *currentInput)
{
    if (toggle->IsAnimating(MOVE_ANIMATION_TRACK))
    {
        return;
    }

    Vector2 touchPos = currentInput->point;
    if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        if (toggle->IsPointInside(touchPos))
        {
            dragAnchorX = touchPos.x - toggle->GetPosition().x;
            toggle->SetSelected(true);
        }
        else
        {
            dragAnchorX = ANCHOR_UNDEFINED;
        }
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        if (dragAnchorX < ANCHOR_UNDEFINED)
        {
            CheckToggleSideChange(currentInput);

            float32 newToggleX = touchPos.x - dragAnchorX;
            float32 newToggleLeftEdge = newToggleX - toggle->GetPivotPoint().x;

            float32 leftBound = buttonLeft->relativePosition.x;
            float32 rightBound = buttonRight->relativePosition.x;
            if (newToggleLeftEdge < leftBound || newToggleLeftEdge + toggle->size.dx > rightBound)
            {
                toggle->relativePosition.x = GetToggleUttermostPosition();
            }
            else
            {
                toggle->relativePosition.x = newToggleX;
            }
        }
    }
    else if (currentInput->phase == UIEvent::Phase::ENDED || currentInput->phase == UIEvent::Phase::CANCELLED)
    {
        if (dragAnchorX < ANCHOR_UNDEFINED)
        {
            CheckToggleSideChange(currentInput);
            toggle->SetSelected(false);
        }
        else if (switchOnTapBesideToggle)
        {
            InternalSetIsLeftSelected(!isLeftSelected, false, currentInput); //switch logical state immediately,
        }       
        float32 toggleX = GetToggleUttermostPosition();

        bool causedByTap = dragAnchorX >= ANCHOR_UNDEFINED;
        Animation * animation = new TogglePositionAnimation(causedByTap, this, &(toggle->relativePosition.x), toggleX, SWITCH_ANIMATION_TIME, Interpolation::EASY_IN);
        animation->Start(MOVE_ANIMATION_TRACK);
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}

void UISwitch::SetIsLeftSelected(bool aIsLeftSelected)
{
    InternalSetIsLeftSelected(aIsLeftSelected, true);
    float32 toggleXPosition = GetToggleUttermostPosition();
    toggle->StopAnimations(MOVE_ANIMATION_TRACK);
    toggle->SetPosition(Vector2(toggleXPosition, toggle->relativePosition.y));
}

void UISwitch::InternalSetIsLeftSelected(bool aIsLeftSelected, bool changeVisualState, UIEvent *inputEvent /*= NULL*/)
{
    bool prevIsLeftSelected = isLeftSelected;
    isLeftSelected = aIsLeftSelected;
    if (prevIsLeftSelected != isLeftSelected)
    {
        if (changeVisualState)
        {
            ChangeVisualState();
        }

        PerformEventWithData(EVENT_VALUE_CHANGED, inputEvent);
    }
}

void UISwitch::ChangeVisualState()
{
    buttonLeft->SetSelected(isLeftSelected);
    buttonRight->SetSelected(!isLeftSelected);
    SetSelected(!isLeftSelected);
    BringChildBack(isLeftSelected ? buttonLeft.Get() : buttonRight.Get());
}

float32 UISwitch::GetToggleUttermostPosition()
{
    float32 leftBound = buttonLeft->relativePosition.x;
    float32 rightBound = buttonRight->relativePosition.x;
    float32 result = isLeftSelected ? leftBound : rightBound - toggle->size.dx;
    result += toggle->GetPivotPoint().x;
    return result;
}

void UISwitch::CheckToggleSideChange(UIEvent *inputEvent /*= NULL*/)
{
    float32 leftBound = buttonLeft->relativePosition.x;
    float32 rightBound = buttonRight->relativePosition.x;
    float32 toggleCenter = toggle->relativePosition.x - toggle->GetPivotPoint().x + toggle->size.dx / 2;
    float32 toggleSpaceCenter = (leftBound + rightBound) / 2;
    InternalSetIsLeftSelected(toggleCenter < toggleSpaceCenter, true, inputEvent);
}

}