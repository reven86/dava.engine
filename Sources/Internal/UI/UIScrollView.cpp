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


#include "UIScrollView.h"
#include "Base/ObjectFactory.h"
#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIScrollView);

static const String UISCROLL_VIEW_CONTAINER_NAME = "scrollContainerControl";

UIScrollView::UIScrollView(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	scrollContainer(new UIScrollViewContainer())
{
	inputEnabled = true;
	multiInput = true;
	SetClipContents(true);
	
	scrollContainer->SetName(UISCROLL_VIEW_CONTAINER_NAME);
	// Scroll container is a child of ScrollView
	AddControl(scrollContainer);
}

UIScrollView::~UIScrollView()
{
	SafeRelease(scrollContainer);
}

void UIScrollView::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
	UIControl::SetRect(rect, rectInAbsoluteCoordinates);
	
	RecalculateContentSize();
}

void UIScrollView::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);

	RecalculateContentSize();
}

void UIScrollView::AddControl(UIControl *control)
{
	UIControl::AddControl(control);
	if (control->GetName() == UISCROLL_VIEW_CONTAINER_NAME)
	{
		scrollContainer = (UIScrollViewContainer*)control;
	}
	
	RecalculateContentSize();
}

Vector2 UIScrollView::GetMaxSize(UIControl * parentControl, Vector2 currentMaxSize, Vector2 parentOffset)
{
	// Initial content max size is actual control sizes
	Vector2 maxSize = currentMaxSize;
	
	List<UIControl*> childslist = parentControl->GetRealChildren();
	for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
	{
    	UIControl *childControl = (UIControl*)(*it);
		if (!childControl)
			continue;
		
		Rect childRect = childControl->GetRect();
		// Reset child rect - we can't move child controls from scroll container
		if ((childRect.x < 0) || (childRect.y < 0))
		{
			childRect.x = Max(0.0f, childRect.x);
			childRect.y = Max(0.0f, childRect.y);
			childControl->SetRect(childRect);
		}
		// Calculate control full "length" and "height"
		float32 controlSizeX = abs(parentOffset.x) + childRect.x + childRect.dx;
		float32 controlSizeY = abs(parentOffset.y) + childRect.y + childRect.dy;
		// Check horizontal size
		if (controlSizeX >= maxSize.x)
		{
			maxSize.x = controlSizeX;
		}
		if (controlSizeY >= maxSize.y)
		{
			maxSize.y = controlSizeY;
		}
		// Change global offset - it has to include parent offset and current child offset
		Vector2 offset;
		offset.x = abs(parentOffset.x) + childRect.x;
		offset.y = abs(parentOffset.y) + childRect.y;
		// Move to next child
		maxSize = GetMaxSize(childControl, maxSize, offset);
	}
	
	return maxSize;
}
    
List<UIControl* >& UIScrollView::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(FindByName(UISCROLL_VIEW_CONTAINER_NAME));
	
	return realChildren;
}

List<UIControl* > UIScrollView::GetSubcontrols()
{
	List<UIControl* > subControls;
	
	AddControlToList(subControls, UISCROLL_VIEW_CONTAINER_NAME);
	
	return subControls;
}

UIControl* UIScrollView::Clone()
{
	UIScrollView *t = new UIScrollView(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollView::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	
	RemoveControl(scrollContainer);
  	SafeRelease(scrollContainer);
	
	UIScrollView* t = (UIScrollView*) srcControl;
		
	this->scrollContainer = static_cast<UIScrollViewContainer*>(t->scrollContainer->Clone());
		
	AddControl(scrollContainer);
}

void UIScrollView::FindRequiredControls()
{
    UIControl * scrollContainerControl = FindByName(UISCROLL_VIEW_CONTAINER_NAME);
    DVASSERT(scrollContainerControl);
    scrollContainer = SafeRetain(DynamicTypeCheck<UIScrollViewContainer*>(scrollContainerControl));
    DVASSERT(scrollContainer);
}

void UIScrollView::SetPadding(const Vector2 & padding)
{
	if (!scrollContainer)
		return;

	Rect parentRect = this->GetRect();
	Rect contentRect = scrollContainer->GetRect();
	
	// Apply scroll offset only if it value don't exceed content size
	if ((abs(padding.x) + parentRect.dx) <= contentRect.dx)
	{
		contentRect.x = padding.x;
	}
	
	if ((abs(padding.y) + parentRect.dy) <= contentRect.dy)
	{
		contentRect.y = padding.y;
	}
	
	scrollContainer->SetRect(contentRect);
}

const Vector2 UIScrollView::GetPadding() const
{
	Rect contentRect = scrollContainer ? scrollContainer->GetRect() : Rect();
	return Vector2(contentRect.x, contentRect.y);
}

const Vector2 UIScrollView::GetContentSize() const
{
	Rect contentRect = scrollContainer ? scrollContainer->GetRect() : Rect();
	return Vector2(contentRect.dx, contentRect.dy);
}
		
void UIScrollView::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	RemoveControl(scrollContainer);
	SafeRelease(scrollContainer);

    UIControl::LoadFromYamlNode(node, loader);
}

void UIScrollView::LoadFromYamlNodeCompleted()
{
	FindRequiredControls();
}

YamlNode * UIScrollView::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    // Control Type
	SetPreferredNodeType(node, "UIScrollView");

	// Scroll container with all childs have to be saved too.
	if (scrollContainer)
	{
		YamlNode* scrollContainerNode = scrollContainer->SaveToYamlNode(loader);
		node->AddNodeToMap(UISCROLL_VIEW_CONTAINER_NAME, scrollContainerNode);
	}
	
    return node;
}

void UIScrollView::RecalculateContentSize()
{
	if (scrollContainer)
	{
		Rect contentRect = scrollContainer->GetRect();
		Rect parentRect = this->GetRect();
		
		// Get max size of content - all childrens
		Vector2 maxSize = GetMaxSize(scrollContainer,
										Vector2(parentRect.dx + abs(contentRect.x), parentRect.dy + abs(contentRect.y)),
										Vector2(0, 0));

		// Update scroll view content size
		scrollContainer->SetRect(Rect(contentRect.x, contentRect.y, maxSize.x, maxSize.y));
	}
}

void UIScrollView::SetReturnSpeed(int32 speedInPixelsPerSec)
{
	if (scrollContainer)
	{
		scrollContainer->SetReturnSpeed(speedInPixelsPerSec);
	}
}

float32 UIScrollView::VisibleAreaSize(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollContainer)
	{
		return 0.0f;
	}

	// Visible area is our rect.
	Vector2 visibleAreaSize(GetRect().dx, GetRect().dy);
	return GetParameterForScrollBar(forScrollBar, visibleAreaSize);
}

float32 UIScrollView::TotalAreaSize(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollContainer)
	{
		return 0.0f;
	}

	// Total area is the rect of the container.
	Vector2 totalAreaSize(scrollContainer->GetRect().dx, scrollContainer->GetRect().dy);
	return GetParameterForScrollBar(forScrollBar, totalAreaSize);
}

float32 UIScrollView::ViewPosition(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollContainer)
	{
		return 0.0f;
	}

	// View position is the position of the scroll container in relation to us.
	Vector2 viewPosition = Vector2(scrollContainer->GetRect().x, scrollContainer->GetRect().y);
	return GetParameterForScrollBar(forScrollBar, viewPosition);
}
 
void UIScrollView::OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition)
{
	if (!byScrollBar || !scrollContainer)
	{
		return;
	}

	Rect curContainerRect = scrollContainer->GetRect();
	if (byScrollBar->GetOrientation() == UIScrollBar::ORIENTATION_HORIZONTAL)
	{
		curContainerRect.x = -newPosition;
	}
	else if (byScrollBar->GetOrientation() == UIScrollBar::ORIENTATION_VERTICAL)
	{
		curContainerRect.y = -newPosition;
	}
	
	scrollContainer->SetRect(curContainerRect);
}

float32 UIScrollView::GetParameterForScrollBar(UIScrollBar* forScrollBar, const Vector2& vectorParam)
{
	if (forScrollBar->GetOrientation() == UIScrollBar::ORIENTATION_HORIZONTAL)
	{
		return vectorParam.x;
	}
	else if (forScrollBar->GetOrientation() == UIScrollBar::ORIENTATION_VERTICAL)
	{
		return vectorParam.y;
	}

	DVASSERT_MSG(false, "Unknown orientation!")
	return 0.0f;
}

void UIScrollView::AddControlToContainer(UIControl* control)
{
	if (scrollContainer)
	{
		scrollContainer->AddControl(control);
	}
}
	
UIScrollViewContainer* UIScrollView::GetContainer()
{
	return scrollContainer;
}

};
