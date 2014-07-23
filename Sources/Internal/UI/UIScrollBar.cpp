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



#include "UI/UIScrollBar.h"
#include "Base/ObjectFactory.h"
#include "FileSystem/YamlNode.h"

namespace DAVA 
{


//use these names for children controls to define UIScrollBar in .yaml
static const String UISCROLLBAR_SLIDER_NAME = "slider";
	
UIScrollBar::UIScrollBar(const Rect &rect, eScrollOrientation requiredOrientation, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
,	delegate(NULL)
,	orientation(requiredOrientation)
,   resizeSliderProportionally(true)
{
	InitControls(rect);
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

void UIScrollBar::AddControl(UIControl *control)
{
	// Synchronize the pointers to the buttons each time new control is added.
	UIControl::AddControl(control);

	if (control->GetName() == UISCROLLBAR_SLIDER_NAME)
	{
		slider = control;
	}
}

List<UIControl* > UIScrollBar::GetSubcontrols()
{
	List<UIControl* > subControls;

	// Lookup for the contols by their names.
	AddControlToList(subControls, UISCROLLBAR_SLIDER_NAME);

	return subControls;
}

UIControl* UIScrollBar::Clone()
{
	UIScrollBar *t = new UIScrollBar(GetRect());
	t->CopyDataFrom(this);
	return t;
}

void UIScrollBar::CopyDataFrom(UIControl *srcControl)
{
    //release default buttons - they have to be copied from srcControl
    RemoveControl(slider);
 	SafeRelease(slider);

    UIControl::CopyDataFrom(srcControl);
	
	UIScrollBar* t = (UIScrollBar*) srcControl;
	this->orientation = t->orientation;
	this->resizeSliderProportionally = t->resizeSliderProportionally;
	this->slider = t->slider->Clone();

	AddControl(slider);
}

void UIScrollBar::InitControls(const Rect &rect)
{
	slider = new UIControl(Rect(0, 0, rect.dx, rect.dy));
	slider->SetName(UISCROLLBAR_SLIDER_NAME);
	slider->SetInputEnabled(false, false);
   	AddControl(slider);
}

void UIScrollBar::LoadFromYamlNodeCompleted()
{
	slider = SafeRetain(FindByName(UISCROLLBAR_SLIDER_NAME));
	if (!slider)
	{
		InitControls();
	}
}

void UIScrollBar::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
	RemoveControl(slider);
	SafeRelease(slider);

	UIControl::LoadFromYamlNode(node, loader);
		
	const YamlNode * orientNode = node->Get("orientation");
	if (orientNode)
	{
		if (orientNode->AsString() == "ORIENTATION_VERTICAL")
			orientation = ORIENTATION_VERTICAL;
		else if (orientNode->AsString() == "ORIENTATION_HORIZONTAL")
			orientation = ORIENTATION_HORIZONTAL;
		else 
		{
			DVASSERT(0 && "Orientation constant is wrong");
		}
	}
}

YamlNode * UIScrollBar::SaveToYamlNode(UIYamlLoader * loader)
{
	slider->SetName(UISCROLLBAR_SLIDER_NAME);

	YamlNode *node = UIControl::SaveToYamlNode(loader);
	//Temp variables
	String stringValue;

	//Orientation
	eScrollOrientation orient = this->GetOrientation();
	switch(orient)
	{
		case ORIENTATION_VERTICAL:
			stringValue = "ORIENTATION_VERTICAL";
			break;
		case ORIENTATION_HORIZONTAL:
			stringValue = "ORIENTATION_HORIZONTAL";
			break;
		default:
			stringValue = "ORIENTATION_VERTICAL";
			break;
	}
	node->Set("orientation", stringValue);

	return node;
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

		currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
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
					slider->size.y = GetValidSliderSize(slider->size.y);
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
					// DF-1998 - Don't allow to set size of slider less than minimum size
					slider->size.y = GetValidSliderSize(slider->size.y);
                    slider->relativePosition.y = 0;
                }
                else if(slider->relativePosition.y + slider->size.y > size.y)
                {
                    slider->size.y = size.y - slider->relativePosition.y;
					// DF-1998 - Don't allow to set size of slider less than minimum size
					// Also keep slider inside control's rect
					slider->size.y = GetValidSliderSize(slider->size.y);
					slider->relativePosition.y = size.y - slider->size.y;
                }
            }
                break;
            case ORIENTATION_HORIZONTAL:
            {
                if (resizeSliderProportionally)
                {
                    slider->size.x = size.x * (visibleArea / totalSize);
					slider->size.x = GetValidSliderSize(slider->size.x);
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
					// DF-1998 - Don't allow to set size of slider less than minimum size
					slider->size.x = GetValidSliderSize(slider->size.x);
                    slider->relativePosition.x = 0;
                }
                else if(slider->relativePosition.x + slider->size.x > size.x)
                {
                    slider->size.x = size.x - slider->relativePosition.x;
					// DF-1998 - Don't allow to set size of slider less than minimum size
					// Also keep slider inside control's rect
					slider->size.x = GetValidSliderSize(slider->size.x);
					slider->relativePosition.x = size.x - slider->size.x;
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

float32 UIScrollBar::GetValidSliderSize(float32 size)
{
	return (size < MINIMUM_SLIDER_SIZE) ? MINIMUM_SLIDER_SIZE : size;
}

};
