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

#include "UI/UIScrollBar.h"
#include "Base/ObjectFactory.h"

namespace DAVA 
{

REGISTER_CLASS(UIScrollBar);
	
UIScrollBar::UIScrollBar(const Rect &rect, eScrollOrientation requiredOrientation, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
,	delegate(NULL)
,	orientation(requiredOrientation)
,   resizeSliderProportionally(true)
{
    slider = new UIControl(Rect(0, 0, rect.dx, rect.dy));
    slider->SetInputEnabled(false, false);
    AddControl(slider);
}

UIScrollBar::~UIScrollBar()
{
    SafeRelease(slider);
}

void UIScrollBar::SetDelegate(UIScrollBarDelegate *newDelegate)
{
    delegate = newDelegate;
}

UIControl *UIScrollBar::GetSlider()
{
    return slider;
}

    
void UIScrollBar::Input(UIEvent *currentInput)
{
    if (!delegate) 
    {
        return;
    }

    if ((currentInput->phase == UIEvent::PHASE_BEGAN) ||
		(currentInput->phase == UIEvent::PHASE_DRAG) ||
		(currentInput->phase == UIEvent::PHASE_ENDED))
    {
		if (currentInput->phase == UIEvent::PHASE_BEGAN)
		{
			startPoint = currentInput->point;
			CalculateStartOffset(currentInput->point);
		}

        float32 newPos;
		if(orientation == ORIENTATION_HORIZONTAL)
		{
			float32 centerOffsetX = (currentInput->point.x - startPoint.x);
			newPos = (startOffset.x + centerOffsetX) * (delegate->TotalAreaSize(this) / size.x);

		}
		else
		{
			float32 centerOffsetY = (currentInput->point.y - startPoint.y);
			newPos = (startOffset.y + centerOffsetY) * (delegate->TotalAreaSize(this) / size.y);
		}

		// Clamp.
        newPos = Min(Max(0.0f, newPos), delegate->TotalAreaSize(this) - delegate->VisibleAreaSize(this));
        
        delegate->OnViewPositionChanged(this, newPos);
    }
}

void UIScrollBar::CalculateStartOffset(const Vector2& inputPoint)
{
    const Rect &r = GetGeometricData().GetUnrotatedRect();
	Rect sliderRect = slider->GetRect();

	if(orientation == ORIENTATION_HORIZONTAL)
	{
		if (((inputPoint.x - r.x) >= sliderRect.x) &&
			((inputPoint.x - r.x) <= sliderRect.x + sliderRect.dx))
		{
			// The tap happened inside the slider - start "as is".
			startOffset.x = (sliderRect.x - r.x);
		}
		else
		{
			// The tap happened outside of the slider - center the slider.
			startOffset.x = (inputPoint.x - r.x - slider->size.x/2);
		}
	}
	else
	{
		// The same with Y.
		if (((inputPoint.y - r.y) >= sliderRect.y) &&
			((inputPoint.y - r.y) <= sliderRect.y + sliderRect.dy))
		{
			// The tap happened inside the slider - start "as is".
			startOffset.y = (sliderRect.y - r.y);
		}
		else
		{
			// The tap happened outside of the slider - center the slider.
			startOffset.y = (inputPoint.y - r.y - slider->size.y/2);
		}
	}
	
	if (startOffset.x < 0.0f)
	{
		startOffset.x = 0.0f;
	}
	
	if (startOffset.y < 0.0f)
	{
		startOffset.y = 0.0f;
	}
}
	
void UIScrollBar::Draw(const UIGeometricData &geometricData)
{
    if (delegate) 
    {
        float32 visibleArea = delegate->VisibleAreaSize(this);
        float32 totalSize = delegate->TotalAreaSize(this);
        float32 viewPos = -delegate->ViewPosition(this);
        switch (orientation) 
        {
            case ORIENTATION_VERTICAL:
            {
                if (resizeSliderProportionally)
                {
                    slider->size.y = size.y * (visibleArea / totalSize);
                    if (slider->size.y < MINIMUM_SLIDER_SIZE) 
                    {
                        slider->size.y = MINIMUM_SLIDER_SIZE;
                    }
                    if (slider->size.y >= size.y) 
                    {
                        slider->SetVisible(false, true);
                    }
                    else 
                    {
                        slider->SetVisible(true, true);
                    }
                }
                    //TODO: optimize
                slider->relativePosition.y = (size.y - slider->size.y) * (viewPos / (totalSize - visibleArea));
                if (slider->relativePosition.y < 0) 
                {
                    slider->size.y += slider->relativePosition.y;
                    slider->relativePosition.y = 0;
                }
                else if(slider->relativePosition.y + slider->size.y > size.y)
                {
                    slider->size.y = size.y - slider->relativePosition.y;
                }

            }
                break;
            case ORIENTATION_HORIZONTAL:
            {
                if (resizeSliderProportionally)
                {
                    slider->size.x = size.x * (visibleArea / totalSize);
                    if (slider->size.x < MINIMUM_SLIDER_SIZE) 
                    {
                        slider->size.x = MINIMUM_SLIDER_SIZE;
                    }
                    if (slider->size.x >= size.x) 
                    {
                        slider->SetVisible(false, true);
                    }
                    else 
                    {
                        slider->SetVisible(true, true);
                    }
                }
                slider->relativePosition.x = (size.x - slider->size.x) * (viewPos / (totalSize - visibleArea));
                if (slider->relativePosition.x < 0) 
                {
                    slider->size.x += slider->relativePosition.x;
                    slider->relativePosition.x = 0;
                }
                else if(slider->relativePosition.x + slider->size.x > size.x)
                {
                    slider->size.x = size.x - slider->relativePosition.x;
                }
            }
                break;
        }
    }
    UIControl::Draw(geometricData);
}

UIScrollBar::eScrollOrientation UIScrollBar::GetOrientation() const
{
	return (eScrollOrientation)orientation;
}

void UIScrollBar::SetOrientation(eScrollOrientation value)
{
	orientation = value;
}

};
