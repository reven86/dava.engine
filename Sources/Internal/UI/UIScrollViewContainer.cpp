/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIScrollViewContainer);

const float32 SCROLL_BEGIN_PIXELS = 8.0f;
const int32	DEFAULT_RETURN_TO_BOUNDS_SPEED = 200; // in pixels per second.

UIScrollViewContainer::UIScrollViewContainer(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	scrollOrigin(0, 0),
	returnToBoundsSpeed(DEFAULT_RETURN_TO_BOUNDS_SPEED),
	mainTouch(-1)
{
	this->SetInputEnabled(true);
	this->SetMultiInput(true);
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
	
	UIScrollViewContainer* t = (UIScrollViewContainer*) srcControl;
		
	scrollOrigin = t->scrollOrigin;
}

void UIScrollViewContainer::SetReturnSpeed(int32 speedInPixelsPerSec)
{
	this->returnToBoundsSpeed = speedInPixelsPerSec;
}

void UIScrollViewContainer::StartScroll(Vector2 _startScrollPosition)
{
	scrollStartInitialPosition = _startScrollPosition;
	scrollStartPosition = _startScrollPosition;
	scrollCurrentPosition = _startScrollPosition;
	scrollStartMovement = false;

	scrollOutboundsOfset = CalculateOutboundsOffset();
}

void UIScrollViewContainer::ProcessScroll(Vector2 _currentScrollPosition)
{
	scrollCurrentPosition = _currentScrollPosition;

	scrollOutboundsOfset = CalculateOutboundsOffset();

	//	This check is required on iPhone, to avoid bugs in movement.	
#if defined(__DAVAENGINE_IPHONE__)
	Vector2 lineLenght = scrollCurrentPosition - scrollStartInitialPosition;

	if (lineLenght.Length() >= SCROLL_BEGIN_PIXELS)
#endif
	{
		//touchStartTime = 0;
		if (!scrollStartMovement) scrollStartPosition = scrollCurrentPosition;
		scrollCurrentShift = Vector2((scrollCurrentPosition.x - scrollStartPosition.x),  (scrollCurrentPosition.y - scrollStartPosition.y));
		scrollStartMovement = true;
	}

	ScrollToPosition(scrollCurrentShift);
}

void UIScrollViewContainer::EndScroll()
{
	if (scrollStartMovement)
	{
		scrollOrigin.x = scrollCurrentShift.x;
		scrollOrigin.y = scrollCurrentShift.y;
	}

	scrollCurrentShift.x = 0;
	scrollCurrentShift.y = 0;
}

void UIScrollViewContainer::ScrollToPosition(const DAVA::Vector2 &position)
{
	Rect rect = this->GetRect();	
	rect.x = scrollOrigin.x + position.x;
	rect.y = scrollOrigin.y + position.y;	
	this->SetRect(rect);
}

void UIScrollViewContainer::Input(UIEvent *currentTouch)
{
	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();
	
	if(1 == touches.size())
	{
		switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				
				Vector2 start = currentTouch->point;
				StartScroll(start);
				state = STATE_SCROLL;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						// perform scrolling
						ProcessScroll(currentTouch->point);
					}
				}
			}
				break;
			case UIEvent::PHASE_ENDED:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						EndScroll();
						state = STATE_DECCELERATION;
					}
				}
			}
				break;
		}
	}
}

bool UIScrollViewContainer::SystemInput(UIEvent *currentTouch)
{
	if(!inputEnabled || !visible || controlState & STATE_DISABLED)
	{
		return false;
	}
	
	if(currentTouch->phase == UIEvent::PHASE_BEGAN)
	{
		if(IsPointInside(currentTouch->point))
		{
			mainTouch = currentTouch->tid;
			PerformEvent(EVENT_TOUCH_DOWN);
			Input(currentTouch); 
		}
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_DRAG)
	{
		UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
		Input(currentTouch);
	}
	else if(currentTouch->tid == mainTouch && currentTouch->phase == UIEvent::PHASE_ENDED)
	{
		mainTouch = -1;
	}

	if (scrollStartMovement && currentTouch->tid == mainTouch)
	{
		return true;
	}
	
	return UIControl::SystemInput(currentTouch);
}

void UIScrollViewContainer::Update(float32 timeElapsed)
{
	if (state == STATE_NONE)
	{
		return;
	}

	if(state == STATE_DECCELERATION)
	{
		Rect contentRect = this->GetRect();
		scrollOrigin = Vector2(contentRect.x, contentRect.y);
	}
	
	if(state != STATE_ZOOM && state != STATE_SCROLL) 
	{
		Rect contentRect = this->GetRect();
		Rect parentRect = this->GetParent()->GetRect();

		Vector2 curOutboundOffset = CalculateOutboundsOffset();
		if (FLOAT_EQUAL(curOutboundOffset.x, 0.0f) && FLOAT_EQUAL(curOutboundOffset.y, 0.0f))
		{
			// Returned back to the bounds.
			return;
		}

		// Calculate the new position and clamp it to avoid overscrolling and flickering.
		float32 curOffset = (timeElapsed * returnToBoundsSpeed);
		
		// Position and clamp X axis.
		const float32 SCROLLING_EPSILON = 1.0f;
		if (FLOAT_EQUAL(scrollOutboundsOfset.x, 0.0f))
		{
			// Do nothing in this case.
		}
		else if (scrollOutboundsOfset.x > 0)
		{
			contentRect.x -= curOffset;
			// Clamp "too right".
			if (contentRect.x < SCROLLING_EPSILON)
			{
				contentRect.x = 0.0f;
			}
		}
		else if (scrollOutboundsOfset.x < 0)
		{
			contentRect.x += curOffset;
			// Clamp "too left".
			if (contentRect.x + contentRect.dx > parentRect.dx)
			{
				contentRect.x = parentRect.dx - contentRect.dx;
			}
		}
		
		// The same with Y.
		if (FLOAT_EQUAL(scrollOutboundsOfset.y, 0.0f))
		{
			// Do nothing in this case.
		}
		if (scrollOutboundsOfset.y > 0)
		{
			contentRect.y -= curOffset;
			// Clamp "too top".
			if (contentRect.y < SCROLLING_EPSILON)
			{
				contentRect.y = 0.0f;
			}
		}
		else if (scrollOutboundsOfset.y < 0)
		{
			contentRect.y += curOffset;
			// Clamp "too bottom".
			if (contentRect.y + contentRect.dy > parentRect.dy)
			{
				contentRect.y = parentRect.dy - contentRect.dy;
			}
		}

		// Done calculations.
		this->SetRect(contentRect);
	}
}

YamlNode * UIScrollViewContainer::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
	
    // Control Type
	SetPreferredNodeType(node, "UIScrollViewContainer");
	// Save scroll view container childs including all sub-childs
	SaveChilds(this, loader, node);
    
    return node;
}

void UIScrollViewContainer::SaveChilds(UIControl *parent, UIYamlLoader * loader, YamlNode * parentNode)
{
	List<UIControl*> childslist = parent->GetRealChildren();
	for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
    {
       	UIControl *childControl = (UIControl*)(*it);
	   	if (!childControl)
	   		continue;

		YamlNode* childNode = childControl->SaveToYamlNode(loader);		
		parentNode->AddNodeToMap(childControl->GetName(), childNode);
		// Save sub-childs
		SaveChilds(childControl, loader, childNode);
	}
}

Vector2 UIScrollViewContainer::CalculateOutboundsOffset()
{
	Rect contentRect = this->GetRect();
	Rect parentRect = this->GetParent()->GetRect();
	
	float32 shiftSizeX = abs(contentRect.x) + parentRect.dx;
	float32 shiftSizeY = abs(contentRect.y) + parentRect.dy;

	Vector2 outboundsOffset;
	if (contentRect.x > 0.0f)
	{
		// Scrolled too right.
		outboundsOffset.x = contentRect.x;
	}
	else if (shiftSizeX > contentRect.dx)
	{
		// Scrolled too left (the offset is negative).
		outboundsOffset.x = contentRect.dx - shiftSizeX;
	}
	
	if (contentRect.y > 0.0f)
	{
		// Scrolled too bottom.
		outboundsOffset.y = contentRect.y;
	}
	else if (shiftSizeY > contentRect.dy)
	{
		// Scrolled too top (the offset is negative).
		outboundsOffset.y = contentRect.dy - shiftSizeY;
	}

	return outboundsOffset;
}

};