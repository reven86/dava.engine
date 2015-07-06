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


#include "HierarchyTreeControlNode.h"
#include "UI/UIList.h"
#include "UI/UIScrollViewContainer.h"

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   UIControl* uiObject,
												   const QString& name) :
	HierarchyTreeNode(name)
    , listDelegate(NULL)
{
	this->parent = parent;
	
	this->uiObject = uiObject;
	this->parentUIObject = NULL;
	this->needReleaseUIObjects = false;
	
	// All UIList controls should always have a delegate
	// We set a delegate here to avoid inappropriate loading of saved list
	UIList *list = dynamic_cast<UIList*>(uiObject);
	if (list)
	{
		listDelegate = new EditorListDelegate(list);
		list->SetDelegate(listDelegate);
	}

	AddControlToParent();
}

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   const HierarchyTreeControlNode* node):
	HierarchyTreeNode(node)
    , listDelegate(NULL)
{
	this->parent = parent;
	this->uiObject = node->GetUIObject()->Clone();
	this->needReleaseUIObjects = false;
	
	// All UIList controls should always have a delegate
	// We set a delegate here to avoid inappropriate loading of saved list
	UIList *list = dynamic_cast<UIList*>(this->uiObject);
	UIList *srcList = dynamic_cast<UIList*>(node->GetUIObject());
	if (list)
	{
		listDelegate = new EditorListDelegate(list);
		EditorListDelegate *srcListDelegate = dynamic_cast<EditorListDelegate*>(srcList->GetDelegate());
		if (srcListDelegate)
		{
			listDelegate->SetAggregatorID(srcListDelegate->GetAggregatorID());
		}
		list->SetDelegate(listDelegate);
	}
	
	// Remove real children & subcontrols - each control is responsible for its
	// subcontrols by itself.
	const List<UIControl* > &realChildren = GetUIObject()->GetRealChildren();
	for (List<UIControl* >::const_iterator iter = realChildren.begin();
		 iter != realChildren.end();
		 ++iter)
	{
        GetUIObject()->RemoveControl(*iter);
	}
	
	AddControlToParent();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>((*iter));
		if (!controlNode)
			continue;
				
		AddTreeNode(controlNode->CreateControlCopy(parent ? this : NULL));
	}
}

void HierarchyTreeControlNode::AddControlToParent()
{
	if (!parent)
		return;
	
	DVASSERT(uiObject);
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
	{
		screenNode->GetScreen()->AddControl(this->uiObject);
	}
	else
	{
		//HierarchyTreeControlNode* controlNode = GetControlNode();
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
		if (controlNode)
		{
			UIControl* control = controlNode->GetUIObject();
			if (control)
				control->AddControl(this->uiObject);
		}
	}	
}

HierarchyTreeControlNode::~HierarchyTreeControlNode()
{
	Cleanup();
	
	UIControl* parent = uiObject->GetParent();
	if (parent)
		parent->RemoveControl(uiObject);
	else
		SafeRelease(uiObject);

	if (needReleaseUIObjects)
	{
		SafeRelease(uiObject);
		SafeRelease(parentUIObject);
	}
}

HierarchyTreeScreenNode* HierarchyTreeControlNode::GetScreenNode() const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
		return screenNode;
	
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode->GetScreenNode();
	
	return NULL;
}

HierarchyTreeControlNode* HierarchyTreeControlNode::GetControlNode() const
{
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode;
	
	return NULL;
}

void HierarchyTreeControlNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	if (this == insertAfter)
		return;
	
	if (parent)
		parent->RemoveTreeNode(this, false, false);
	
	HierarchyTreeControlNode* newParentControl = dynamic_cast<HierarchyTreeControlNode* >(node);
	HierarchyTreeScreenNode* newParentScreen = dynamic_cast<HierarchyTreeScreenNode* >(node);
	DVASSERT(newParentControl || newParentScreen);
	if (!newParentControl && !newParentScreen)
		return;

	UIControl* afterControl = NULL;
	HierarchyTreeControlNode* insertAfterControl = dynamic_cast<HierarchyTreeControlNode* >(insertAfter);
	if (insertAfterControl)
		afterControl = insertAfterControl->GetUIObject();
	
	UIControl* newParentUI = NULL;
	if (newParentControl)
		newParentUI = newParentControl->GetUIObject();
	else if (newParentScreen)
		newParentUI = newParentScreen->GetScreen();
	
	node->AddTreeNode(this, insertAfter);
	if (newParentUI && uiObject)
	{
        Rect controlAbsoluteRect = uiObject->GetRect(true);
        Rect parentAbsoluteRect = newParentUI->GetRect(true);

		if (insertAfter != node)
		{
			newParentUI->InsertChildAbove(uiObject, afterControl);
		}
		else
		{
			UIControl* belowControl = NULL;
			const List<UIControl*> & controls = newParentUI->GetChildren();
			if (controls.size())
			{
				belowControl = *controls.begin();
			}
			newParentUI->InsertChildBelow(uiObject, belowControl);
		}

        // Recalculate the relative coords of the moved object to don't change its position.
        Vector2 newControlOffset = controlAbsoluteRect.GetPosition() - parentAbsoluteRect.GetPosition();
        uiObject->SetRect(Rect(newControlOffset, uiObject->GetRect().GetSize()));
        
        // Fix
		// DF-2395 - Recalculate scrollContainer content each time we add controls to it
		UIScrollViewContainer *container = dynamic_cast<UIScrollViewContainer*>(newParentUI);
		if (container)
		{
			UIScrollView *scroll =  dynamic_cast<UIScrollView*>(container->GetParent());
			if (scroll)
			{
				scroll->RecalculateContentSize();
			}
		}
	}
	
	parent = node;
}

HierarchyTreeControlNode* HierarchyTreeControlNode::CreateControlCopy(HierarchyTreeNode* parent) const
{
	return new HierarchyTreeControlNode(parent, this);
}

Vector2 HierarchyTreeControlNode::GetParentDelta(bool skipControl/* = false*/) const
{
	Vector2 parentDelta(0, 0);
	
	if (!skipControl && uiObject)
	{
		Rect rect = uiObject->GetRect();
		parentDelta = rect.GetPosition();
		// For UIScrollView we should consider scrollcontainer delta also
		UIScrollView* scrollView = dynamic_cast<UIScrollView* >(uiObject);
		if (scrollView)
		{
			parentDelta += scrollView->GetPadding();
		}
	}
	
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode* >(parent);
	if (parentControl)
		parentDelta += parentControl->GetParentDelta();
	return parentDelta;
}

void HierarchyTreeControlNode::RemoveTreeNodeFromScene()
{
	if (!this->GetParent() || !uiObject->GetParent())
	{
		return;
	}
	
	this->parentUIObject = uiObject->GetParent();
	SafeRetain(this->parentUIObject);

	SafeRetain(uiObject);
	this->parentUIObject->RemoveControl(uiObject);
	
	// We added an additional reference to uiObject and parentUIObject - don't forget
	// to release them in destructor.
	this->needReleaseUIObjects = true;
}

void HierarchyTreeControlNode::ReturnTreeNodeToScene()
{
	if (!this->uiObject || !this->parentUIObject || !this->redoParentNode)
	{
		return;
	}

	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);

	// Return the object back to the proper position.
    UIControl* childUIObjectAbove = NULL;
    HierarchyTreeControlNode* controlRedoPreviousNode = dynamic_cast<HierarchyTreeControlNode*>(redoPreviousNode);
    if (controlRedoPreviousNode)
        childUIObjectAbove = controlRedoPreviousNode->GetUIObject();
    if (childUIObjectAbove == NULL)
	{
		// Set it to the top.
		int childCount = parentUIObject->GetChildren().size();
		if (childCount == 0)
		{
			parentUIObject->AddControl(uiObject);
		}
		else
		{
			parentUIObject->InsertChildBelow(uiObject, parentUIObject->GetChildren().front());
		}
	}
	else
	{
		parentUIObject->InsertChildBelow(uiObject, childUIObjectAbove);
	}

	uiObject->Release();
	parentUIObject->Release();
	
	// We just reset extra references to uiObject and parentUIObject - no additional release needed.
	this->needReleaseUIObjects = false;
}

Rect HierarchyTreeControlNode::GetRect(bool checkAngle/*=false*/) const
{
	Rect rect;
    
    if (uiObject && dynamic_cast<UIScrollView*>(uiObject))
    {
        // We are Scroll View, and our child controls should not be taken into account
        // while calculating the rect. See pls DF-1844 and DF-2704.
        UIScrollView* scrollView = dynamic_cast<UIScrollView*>(uiObject);
        return Rect(scrollView->GetPosition(true), scrollView->GetSize());
    }

	if (uiObject)
    {
        if(!checkAngle)
        {
            rect = uiObject->GetRect(true);
        }
        else
        {
            rect = uiObject->GetGeometricData().GetAABBox();
        }
    }

	const HIERARCHYTREENODESLIST& childs = GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
		if (!control)
			continue;
		
		Rect controlRect = control->GetRect();
		rect = rect.Combine(controlRect);
	}

	return rect;
}

void HierarchyTreeControlNode::SetVisibleFlag(bool value)
{
	if (uiObject)
	{
        uiObject->SetVisibleForUIEditor(value);
	}
}

bool HierarchyTreeControlNode::GetVisibleFlag() const
{
	if (uiObject)
	{
        return uiObject->GetVisibleForUIEditor();
	}

	return false;
}

void HierarchyTreeControlNode::OnScreenScaleChanged()
{
    UpdateUIObject();
}

void HierarchyTreeControlNode::OnScreenPositionChanged()
{
    UpdateUIObject();
}

void HierarchyTreeControlNode::UpdateUIObject()
{
    // UIWebView contains the interal system object,
    // which has to be repositioned while screen scale/position is changed.
    UIWebView* webView = dynamic_cast<UIWebView*>(GetUIObject());
    if (!webView)
    {
        return;
    }
    
    webView->SetRect(webView->GetRect(true), true);
}