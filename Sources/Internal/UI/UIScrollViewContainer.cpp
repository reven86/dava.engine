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


#include "UI/UIScrollViewContainer.h"
#include "UI/UIScrollView.h"
#include "UI/UIControlSystem.h"
#include "UI/ScrollHelper.h"

namespace DAVA 
{
	
const int32 DEFAULT_TOUCH_TRESHOLD = 15;  // Default value for finger touch tresshold


UIScrollViewContainer::UIScrollViewContainer(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
: UIControl(rect, rectInAbsoluteCoordinates)
, state(STATE_NONE)
, touchTreshold(DEFAULT_TOUCH_TRESHOLD)
, mainTouch(-1)
, oldPos(0.f, 0.f)
, newPos(0.f, 0.f)
, currentScroll(NULL)
, lockTouch(false)
, scrollStartMovement(false)
, enableHorizontalScroll(true)
, enableVerticalScroll(true)
{
	this->SetInputEnabled(true);
	this->SetMultiInput(true);
    SetFocusEnabled(false);
}

UIScrollViewContainer::~UIScrollViewContainer()
{
}

UIControl* UIScrollViewContainer::Clone()
{
	UIScrollViewContainer *t = new UIScrollViewContainer(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollViewContainer::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
}

void UIScrollViewContainer::SetSize(const Vector2& size)
{
    UIControl::SetSize(size);

    UIControl *parent = GetParent();
	if (parent)
	{
        const Vector2& parentSize = parent->GetSize();
        // We should not allow scrolling when content rect is less than or is equal ScrollView "window"
        enableHorizontalScroll = size.dx > parentSize.dx;
        enableVerticalScroll = size.dy > parentSize.dy;
        Array<bool, Vector2::AXIS_COUNT> enableScroll;
        enableScroll[Vector2::AXIS_X] = enableHorizontalScroll;
        enableScroll[Vector2::AXIS_Y] = enableVerticalScroll;

        UIScrollView* scrollView = cast_if_equal<UIScrollView*>(parent);
        if (scrollView != nullptr)
        {
            scrollView->OnScrollViewContainerSizeChanged();

            if (scrollView->IsAutoUpdate())
            {
                for (int32 axis = 0; axis < Vector2::AXIS_COUNT; axis++)
                {
                    if (!enableScroll[axis])
                    {
                        if (scrollView->IsCenterContent())
                        {
                            relativePosition.data[axis] = (scrollView->GetSize().data[axis] - GetSize().data[axis]) / 2;
                        }
                        else
                        {
                            relativePosition.data[axis] = 0;
                        }
                    }
                }
            }
        }
    }
}

void UIScrollViewContainer::SetTouchTreshold(int32 holdDelta)
{
	touchTreshold = holdDelta;
}
int32 UIScrollViewContainer::GetTouchTreshold()
{
	return touchTreshold;
}

void UIScrollViewContainer::Input(UIEvent *currentTouch)
{
	if(currentTouch->tid == mainTouch)
	{
		newPos = currentTouch->point;
		
		switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollStartInitialPosition = currentTouch->point;
				scrollStartMovement = false;
				state = STATE_SCROLL;
				lockTouch = true;
				oldPos = newPos;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL)
				{
					scrollStartMovement = true;
				}
			}
			break;
			case UIEvent::PHASE_ENDED:
			{
				lockTouch = false;
				state = STATE_DECCELERATION;
			}
			break;
		}
	}
}

bool UIScrollViewContainer::SystemInput(UIEvent *currentTouch)
{
	if(!GetInputEnabled() || !visible || !visibleForUIEditor || (controlState & STATE_DISABLED))
	{
		return false;
	}

	if (currentTouch->touchLocker != this)
	{
		controlState |= STATE_DISABLED; //this funny code is written to fix bugs with calling Input() twice.
	}
	bool systemInput = UIControl::SystemInput(currentTouch);
    controlState &= ~STATE_DISABLED; //All this control must be reengeneried

	if (currentTouch->GetInputHandledType() == UIEvent::INPUT_HANDLED_HARD)
	{
		// Can't scroll - some child control already processed this input.
        mainTouch = -1;
		return systemInput;
	}

	if(currentTouch->phase == UIEvent::PHASE_BEGAN && mainTouch == -1)
	{
		if(IsPointInside(currentTouch->point))
		{
            currentScroll = NULL;
			mainTouch = currentTouch->tid;
			PerformEvent(EVENT_TOUCH_DOWN);
			Input(currentTouch);
		}
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_DRAG)
	{
		// Don't scroll if touchTreshold is not exceeded
		if ((Abs(currentTouch->point.x - scrollStartInitialPosition.x) > touchTreshold) ||
			(Abs(currentTouch->point.y - scrollStartInitialPosition.y) > touchTreshold))
		{
            UIScrollView *scrollView = DynamicTypeCheck<UIScrollView*>(this->GetParent());
            DVASSERT(scrollView);
            if(enableHorizontalScroll
               && Abs(currentTouch->point.x - scrollStartInitialPosition.x) > touchTreshold
               && (!currentScroll || currentScroll == scrollView->GetHorizontalScroll()))
            {
                currentScroll = scrollView->GetHorizontalScroll();
            }
            else if(enableVerticalScroll
                    && (Abs(currentTouch->point.y - scrollStartInitialPosition.y) > touchTreshold)
                    && (!currentScroll || currentScroll == scrollView->GetVerticalScroll()))
            {
                currentScroll = scrollView->GetVerticalScroll();
            }
            if (currentTouch->touchLocker != this && currentScroll)
            {
                UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
            }
			Input(currentTouch);
		}
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_ENDED)
	{
		Input(currentTouch);
		mainTouch = -1;
	}

	if (scrollStartMovement && currentTouch->tid == mainTouch)
	{
		return true;
	}
	
	return systemInput;
}

void UIScrollViewContainer::Update(float32 timeElapsed)
{

	
	UIScrollView *scrollView = cast_if_equal<UIScrollView*>(this->GetParent());
	if (scrollView)
	{
	
		Vector2 posDelta = newPos - oldPos;
		oldPos = newPos;
	
		// Get scrolls positions and change scroll container relative position
        if (enableHorizontalScroll)
        {
            if (scrollView->GetHorizontalScroll() == currentScroll)
            {
                relativePosition.x = currentScroll->GetPosition(posDelta.x, timeElapsed, lockTouch);
            }
            else
            {
                relativePosition.x = scrollView->GetHorizontalScroll()->GetPosition(0, timeElapsed, false);
            }
        }
        else if (scrollView->IsAutoUpdate())
        {
            if (scrollView->IsCenterContent())
            {
                relativePosition.x = (scrollView->GetSize().dx - GetSize().dx) / 2;
            }
            else
            {
                relativePosition.x = 0;
            }
        }

        if (enableVerticalScroll)
        {
            if (scrollView->GetVerticalScroll() == currentScroll)
            {
                relativePosition.y = currentScroll->GetPosition(posDelta.y, timeElapsed, lockTouch);
            }
            else
            {
                relativePosition.y = scrollView->GetVerticalScroll()->GetPosition(0, timeElapsed, false);
            }
        }
        else if (scrollView->IsAutoUpdate())
        {
            if (scrollView->IsCenterContent())
            {
                relativePosition.y = (scrollView->GetSize().dy - GetSize().dy) / 2;
            }
            else
            {
                relativePosition.y = 0;
            }
        }

        // Change state when scrolling is not active
		if (state != STATE_NONE && !lockTouch && (scrollView->GetHorizontalScroll()->GetCurrentSpeed() == 0) && (scrollView->GetVerticalScroll()->GetCurrentSpeed() == 0))
		{
			state = STATE_NONE;
		}
	}
}

void UIScrollViewContainer::InputCancelled( UIEvent *currentInput )
{
    if (currentInput->tid == mainTouch)
    {
        mainTouch = -1;
        lockTouch = false;
    }
}

void UIScrollViewContainer::WillDisappear()
{
    mainTouch = -1;
    lockTouch = false;
    UIScrollView *scrollView = dynamic_cast<UIScrollView*>(GetParent());
    if (scrollView)
    {
        scrollView->GetHorizontalScroll()->GetPosition(0, 1.0f, true);
        scrollView->GetVerticalScroll()->GetPosition(0, 1.0f, true);
    }
    state = STATE_NONE;
}

};
