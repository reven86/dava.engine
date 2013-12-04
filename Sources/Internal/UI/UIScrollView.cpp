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




#include "UIScrollView.h"
#include "Base/ObjectFactory.h"
#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	

static const String UISCROLL_VIEW_CONTAINER_NAME = "scrollContainerControl";

UIScrollView::UIScrollView(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	scrollContainer(new UIScrollViewContainer()),
	scrollHorizontal(new ScrollHelper()),
	scrollVertical(new ScrollHelper())
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
	SafeRelease(scrollHorizontal);
	SafeRelease(scrollVertical);
}

void UIScrollView::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
	UIControl::SetRect(rect, rectInAbsoluteCoordinates);

	scrollHorizontal->SetViewSize(rect.dx);
	scrollVertical->SetViewSize(rect.dy);
}

void UIScrollView::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);

	scrollHorizontal->SetViewSize(newSize.x);
	scrollVertical->SetViewSize(newSize.y);
}

void UIScrollView::AddControl(UIControl *control)
{
	UIControl::AddControl(control);
	if (control->GetName() == UISCROLL_VIEW_CONTAINER_NAME)
	{
		scrollContainer = (UIScrollViewContainer*)control;
	}	
}

void UIScrollView::PushContentToBounds(UIControl *parentControl)
{
	// We have to shift each child of ScrollContent to fit its bounds
	const List<UIControl*> &childslist = parentControl->GetRealChildren();
	for(List<UIControl*>::const_iterator it = childslist.begin(); it != childslist.end(); ++it)
	{
	   	UIControl *childControl = (*it);
		if (!childControl)
			continue;
		
		Rect childRect = childControl->GetRect();
		
		Vector2 position = GetControlOffset(childControl, Vector2(childRect.x, childRect.y));
		
		if (position.x < 0)
		{
			childRect.x += Abs(position.x);
		}
		
		if (position.y < 0)
		{
			childRect.y += Abs(position.y);
		}
		// Move each first child
		childControl->SetRect(childRect);
	}
}

Vector2 UIScrollView::GetControlOffset(UIControl *parentControl, Vector2 currentContentOffset)
{
	Vector2 currentOffset = currentContentOffset;
	// Get control's farest position inside scrollContainer
	const List<UIControl*> &childslist = parentControl->GetRealChildren();
	for(List<UIControl*>::const_iterator it = childslist.begin(); it != childslist.end(); ++it)
	{	
	   	UIControl *childControl = (*it);
		if (!childControl)
			continue;
		
		Rect childRect = childControl->GetRect();	
		float32 controlPosX = currentContentOffset.x + childRect.x;
		float32 controlPosY = currentContentOffset.y + childRect.y;
		
		Vector2 controlOffset = GetControlOffset(childControl, Vector2(controlPosX, controlPosY));
		currentOffset.x = Min(currentOffset.x, controlOffset.x);
		currentOffset.y = Min(currentOffset.y, controlOffset.y);
	}
	return currentOffset;
}


Vector2 UIScrollView::GetMaxSize(UIControl * parentControl, Vector2 currentMaxSize, Vector2 parentOffset)
{
	// Initial content max size is actual control sizes
	Vector2 maxSize = currentMaxSize;
	
	const List<UIControl*> &childslist = parentControl->GetRealChildren();
	for(List<UIControl*>::const_iterator it = childslist.begin(); it != childslist.end(); ++it)
	{
    	UIControl *childControl = (*it);
		if (!childControl)
			continue;
		
		Rect childRect = childControl->GetRect();
		// Calculate control full "length" and "height"
		float32 controlSizeX = Abs(parentOffset.x) + childRect.x + childRect.dx;
		float32 controlSizeY = Abs(parentOffset.y) + childRect.y + childRect.dy;
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
		offset.x = Abs(parentOffset.x) + childRect.x;
		offset.y = Abs(parentOffset.y) + childRect.y;
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
	if (!scrollHorizontal || !scrollVertical || !scrollContainer)
	{
		return;
	}
	
	Rect parentRect = this->GetRect();
	Rect contentRect = scrollContainer->GetRect();
	
	// Apply scroll offset only if it value don't exceed content size
	if ((Abs(padding.x) + parentRect.dx) <= contentRect.dx)
	{
		contentRect.x = padding.x;
	}
	
	if ((Abs(padding.y) + parentRect.dy) <= contentRect.dy)
	{
		contentRect.y = padding.y;
	}
	
	scrollContainer->SetRect(contentRect);
	scrollHorizontal->SetPosition(contentRect.x);
	scrollVertical->SetPosition(contentRect.y);
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
		
void UIScrollView::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
	RemoveControl(scrollContainer);
	SafeRelease(scrollContainer);

    UIControl::LoadFromYamlNode(node, loader);
}

void UIScrollView::LoadFromYamlNodeCompleted()
{
	FindRequiredControls();
	RecalculateContentSize();
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
	if (!scrollHorizontal || !scrollVertical || !scrollContainer)
	{
		return;
	}
	
	Rect contentRect = scrollContainer->GetRect();
	Rect parentRect = this->GetRect();
	
	// Move all scrollContainer content with negative positions iside its rect
	PushContentToBounds(scrollContainer);
	// Get max size of content - all childrens
	Vector2 maxSize = GetMaxSize(scrollContainer,
									Vector2(parentRect.dx + abs(contentRect.x), parentRect.dy + abs(contentRect.y)),
									Vector2(0, 0));
									
	// Update scroll view content size
	scrollContainer->SetRect(Rect(contentRect.x, contentRect.y, maxSize.x, maxSize.y));
	scrollHorizontal->SetElementSize(maxSize.x);
	scrollVertical->SetElementSize(maxSize.y);
}

void UIScrollView::SetReturnSpeed(float32 speedInSeconds)
{
	if (!scrollHorizontal || !scrollVertical)
	{
		return;
	}
	
	scrollHorizontal->SetBorderMoveModifer(speedInSeconds);
	scrollVertical->SetBorderMoveModifer(speedInSeconds);
}

void UIScrollView::SetScrollSpeed(float32 speedInSeconds)
{
	if (!scrollHorizontal || !scrollVertical)
	{
		return;
	}
	
	scrollHorizontal->SetSlowDownTime(speedInSeconds);
	scrollVertical->SetSlowDownTime(speedInSeconds);
}

float32 UIScrollView::VisibleAreaSize(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollHorizontal || !scrollVertical)
	{
		return 0.0f;
	}

	// Visible area is our rect.
	Vector2 visibleAreaSize(scrollHorizontal->GetViewSize(), scrollVertical->GetViewSize());
	return GetParameterForScrollBar(forScrollBar, visibleAreaSize);
}

float32 UIScrollView::TotalAreaSize(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollHorizontal || !scrollVertical)
	{
		return 0.0f;
	}

	// Total area is the rect of the container.
	Vector2 totalAreaSize(scrollHorizontal->GetElementSize(), scrollVertical->GetElementSize());
	return GetParameterForScrollBar(forScrollBar, totalAreaSize);
}

float32 UIScrollView::ViewPosition(UIScrollBar *forScrollBar)
{
	if (!forScrollBar || !scrollHorizontal || !scrollVertical)
	{
		return 0.0f;
	}

	// View position is the position of the scroll container in relation to us.
	Vector2 viewPosition = Vector2(scrollHorizontal->GetPosition(), scrollVertical->GetPosition());
	return GetParameterForScrollBar(forScrollBar, viewPosition);
}
 
void UIScrollView::OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition)
{
	if (!byScrollBar || !scrollContainer || !scrollHorizontal || !scrollVertical)
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
	
	scrollHorizontal->SetPosition(curContainerRect.x);
	scrollVertical->SetPosition(curContainerRect.y);
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

ScrollHelper* UIScrollView::GetHorizontalScroll()
{
	return scrollHorizontal;
}

ScrollHelper* UIScrollView::GetVerticalScroll()
{
	return scrollVertical;
}

};
