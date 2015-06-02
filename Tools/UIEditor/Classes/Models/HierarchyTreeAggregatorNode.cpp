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


#include "HierarchyTreeAggregatorNode.h"
#include "ResourcesManageHelper.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorControlNode.h"

#include "UI/UIList.h"

#define WIDTH_NODE "width"
#define HEIGHT_NODE "height"

using namespace DAVA;

HierarchyTreeAggregatorNode::HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent, const QString& name, const Rect& rect) :
	HierarchyTreeScreenNode(parent, name),
    listDelegate(NULL)
{
	this->rect = rect;
	screen->SetRect(rect);
}

HierarchyTreeAggregatorNode::HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent,
														 const HierarchyTreeAggregatorNode* base,
                                                         bool needLoad/* = true*/)
:	HierarchyTreeScreenNode(parent, base, needLoad),
    listDelegate(NULL)
{
	this->rect = base->GetRect();
	screen->SetRect(rect);
}

HierarchyTreeAggregatorNode::~HierarchyTreeAggregatorNode()
{
	DVASSERT(childs.size() == 0);
    
    SafeRelease(listDelegate);
}

void HierarchyTreeAggregatorNode::AddChild(HierarchyTreeControlNode *node)
{
	childs.insert(node);
	UpdateChilds();
}

void HierarchyTreeAggregatorNode::RemoveChild(HierarchyTreeControlNode* node)
{
	CHILDS::iterator iter = childs.find(node);
	if (iter != childs.end())
		childs.erase(node);
}

void HierarchyTreeAggregatorNode::ReturnTreeNodeToScene()
{
	HierarchyTreeScreenNode::ReturnTreeNodeToScene();
}

void HierarchyTreeAggregatorNode::SetRect(const Rect& rect)
{
	this->rect = rect;
	screen->SetRect(rect);
	UpdateChilds();
}

Rect HierarchyTreeAggregatorNode::GetRect() const
{
    // Need to override because GetRect() implementation of HierarchyTreeScreenNode does additional processing.
    return this->rect;
}

Rect HierarchyTreeAggregatorNode::GetOwnRect() const
{
    return GetRect();
}

void HierarchyTreeAggregatorNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	Rect rect = GetScreen()->GetRect();
	HierarchyTreeScreenNode::SetParent(node, insertAfter);
	GetScreen()->SetRect(rect); //restore rect
}

HierarchyTreeControlNode* HierarchyTreeAggregatorNode::CreateChild(HierarchyTreeNode* parentNode, const QString& name)
{
	UIAggregatorControl* control = new UIAggregatorControl();
	control->CopyDataFrom(GetScreen());
	HierarchyTreeControlNode* controlNode = new HierarchyTreeAggregatorControlNode(this, parentNode, control, name);
	childs.insert(controlNode);
	UpdateChilds();
	return controlNode;
}

void HierarchyTreeAggregatorNode::UpdateChilds()
{
	for (CHILDS::iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		
		UIAggregatorControl* aggregatorControl = dynamic_cast<UIAggregatorControl*>(controlNode->GetUIObject());
		DVASSERT(aggregatorControl);
		if (!aggregatorControl)
			continue;
		
		// Remove any child controls of UIControl to prevent appearance of deleted
		// child in case when screen child is aggregator.
		/*List<UIControl*> aggregatorChilds = aggregatorControl->GetChildren();
		int size = GetScreen()->GetChildren().size();
		for (List<UIControl*>::iterator iter = aggregatorChilds.begin(); iter != aggregatorChilds.end();)
		{
			if (--size < 0)
				break;
			UIControl* child = (*iter);
			++iter;
			aggregatorControl->RemoveControl(child);
		}*/

		// TODO! Yuri Coder, 2013/10/09. This method causes problems when aggregator control has its own
		// children. Adding children to aggregator is disabled because of DF-2163.
		aggregatorControl->RemoveAllControls();
		aggregatorControl->SetSize(rect.GetSize());

		const List<UIControl*> & childsList = screen->GetChildren();
		UIControl* belowControl = NULL;
		List<UIControl*>::const_iterator belowIter = aggregatorControl->GetChildren().begin();
		if (belowIter != aggregatorControl->GetChildren().end())
			belowControl = (*belowIter);
		for (List<UIControl*>::const_iterator iter = childsList.begin(); iter != childsList.end(); ++iter)
		{
			UIControl* control = (*iter);
			UIControl* newControl = control->Clone();
            
            UIList* controlIsList = dynamic_cast<UIList*>(control);
            if (controlIsList)
            {
                EditorListDelegate* delegate = dynamic_cast<EditorListDelegate*>(controlIsList->GetDelegate());
                SafeRetain(delegate);
                static_cast<UIList*>(newControl)->SetDelegate(delegate);
            }

			aggregatorControl->InsertChildBelow(newControl, belowControl);
			aggregatorControl->AddAggregatorChild(newControl);
            newControl->Release();
		}

		// Have to remember Align Data prior to changing rectangle and then apply the alignment
		// back, since SetRect affects the alignment. See please DF-2342.
		AlignData alignData = SaveAlignData(aggregatorControl);
		aggregatorControl->SetRect(aggregatorControl->GetRect());	//update childs size and position
		RestoreAlignData(aggregatorControl, alignData);
	}
}

void HierarchyTreeAggregatorNode::RemoveSelection()
{
	UpdateChilds();
}

bool HierarchyTreeAggregatorNode::Load(const Rect& rect, const QString& path)
{
	this->path = ResourcesManageHelper::GetResourceRelativePath(path, true).toStdString();
	this->rect = rect;
	screen->SetRect(rect);

	bool result = HierarchyTreeScreenNode::Load(path);
	UpdateHierarchyTree();
	return result;
}

bool HierarchyTreeAggregatorNode::Save(YamlNode* node, const QString& path, bool saveAll)
{
	// DF-2164 - Get proper relative path for aggregator
	String aggregatorPath = ResourcesManageHelper::GetResourceRelativePath(path, true).toStdString();
	// Always update aggregator path if it is empty while save
	if (this->path.IsEmpty())
	{
		this->path = aggregatorPath;
	}
	
	for (CHILDS::iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		
		UIAggregatorControl* aggregatorControl = dynamic_cast<UIAggregatorControl*>(controlNode->GetUIObject());
		DVASSERT(aggregatorControl);
		if (!aggregatorControl)
			continue;
			
		// DF-2164 - Get proper relative path for aggregator
		String aggregatorPath = ResourcesManageHelper::GetResourceRelativePath(path, true).toStdString();
		aggregatorControl->SetAggregatorPath(aggregatorPath);
	}
	
	node->Set(WIDTH_NODE, (int32)rect.dx);
	node->Set(HEIGHT_NODE, (int32)rect.dy);
	return HierarchyTreeScreenNode::Save(path, saveAll);
}

void HierarchyTreeAggregatorNode::UpdateHierarchyTree()
{
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeScreenNode* node = dynamic_cast<HierarchyTreeScreenNode*>(*iter);
		DVASSERT(node);
		if (!node)
			continue;
		
		const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = node->GetChildNodes();
		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
			 iter != child.end();
			 ++iter)
		{
			HierarchyTreeControlNode* node = dynamic_cast<HierarchyTreeControlNode*>(*iter);
			DVASSERT(node);
			if (!node)
				continue;
			
			ReplaceAggregator(node);
		}
	}
}

void HierarchyTreeAggregatorNode::ReplaceAggregator(HierarchyTreeControlNode *node)
{
	UIList *list = dynamic_cast<UIList*>(node->GetUIObject());
	// For UIList control we should should always create a delegate
	// Set aggregator ID for list if it has saved aggregator path and it is available in tree
	if (list && list->GetAggregatorPath() == path)
	{
        SafeRelease(listDelegate);
		listDelegate = new EditorListDelegate(list);
		// If loaded delegate has aggregator path - pass its id to delegate
		listDelegate->SetAggregatorID(GetId());
		// Always set a delegate for loaded UIList
		list->SetDelegate(listDelegate);
	}

	UIAggregatorControl* uiAggregator = dynamic_cast<UIAggregatorControl*>(node->GetUIObject());

	if (uiAggregator && uiAggregator->GetAggregatorPath() == path)
	{
		Logger::Debug(uiAggregator->GetAggregatorPath().GetAbsolutePathname().c_str());
		HIERARCHYTREENODESLIST childs = node->GetChildNodes();
		uint32 i = 0;
		for (HIERARCHYTREENODESLIST::iterator iter = childs.begin(); iter != childs.end(); ++iter)
		{
			HierarchyTreeNode* childNode = (*iter);
			node->RemoveTreeNode(childNode, true, false);
			if (++i == GetScreen()->GetChildren().size())
				break;
		}
	
		HierarchyTreeAggregatorControlNode* controlNode = dynamic_cast<HierarchyTreeAggregatorControlNode*>(node);
		DVASSERT(controlNode);
		if (controlNode)
			controlNode->SetAggregatorNode(this);
		this->childs.insert(node);
		UpdateChilds();
	}
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeControlNode* node = dynamic_cast<HierarchyTreeControlNode*>(*iter);
		DVASSERT(node);
		if (!node)
			continue;
		
		ReplaceAggregator(node);
	}
}

void HierarchyTreeAggregatorNode::SetName(const QString& name)
{
	HierarchyTreeScreenNode::SetName(name);
}

const HierarchyTreeAggregatorNode::CHILDS& HierarchyTreeAggregatorNode::GetChilds() const
{
	return childs;
}

const FilePath& HierarchyTreeAggregatorNode::GetPath()
{
	return path;
}
