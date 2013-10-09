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
#include "Classes/UI/hierarchytreecontrol.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include "HierarchyTreeController.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "SubcontrolsHelper.h"

#define TREE_MIME_DATA 0
#define ITEM_ID 0, Qt::UserRole

HierarchyTreeControlMimeData::HierarchyTreeControlMimeData(const QList<QTreeWidgetItem*> items)
{
	for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		QVariant data = item->data(ITEM_ID);
		HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
		
		Logger::Debug("HierarchyTreeNode::HIERARCHYTREENODEID %d", id);
		
		HierarchyTreeNode::HIERARCHYTREENODESIDLIST::iterator it = std::find(this->items.begin(), this->items.end(), id);
		if (it == this->items.end())
			this->items.push_back(id);
	}
}

HierarchyTreeControlMimeData::~HierarchyTreeControlMimeData()
{
	
}

bool HierarchyTreeControlMimeData::IsDropEnable(const HierarchyTreeNode *parentItem) const
{
	const HierarchyTreePlatformNode* parentPlatform = dynamic_cast<const HierarchyTreePlatformNode*>(parentItem);
	const HierarchyTreeScreenNode* parentScreen = dynamic_cast<const HierarchyTreeScreenNode*>(parentItem);
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode*>(parentItem);
	const HierarchyTreeRootNode* rootNode = dynamic_cast<const HierarchyTreeRootNode*>(parentItem);
	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID id = (*iter);
		
		const HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(id);
		
		if (node->IsHasChild(parentItem))
			return false;
		
		if (parentPlatform)
		{
			//all items must be screen
			const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode*>(node);
			if (!screen)
				return false;
		}
		else if (parentScreen || parentControl)
		{
			//all items must be control
			const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(node);
			if (!control)
				return false;
		}
		else if (rootNode)
		{
			const HierarchyTreePlatformNode* platform = dynamic_cast<const HierarchyTreePlatformNode*>(node);
			if (!platform)
				return false;
		}
	}
	
	return true;
}

HierarchyTreeNode::HIERARCHYTREENODESIDLIST HierarchyTreeControlMimeData::GetItems() const
{
	return items;
}

HierarchyTreeControl::HierarchyTreeControl(QWidget *parent) :
    QTreeWidget(parent)
{
}

void HierarchyTreeControl::contextMenuEvent(QContextMenuEvent * event)
{
	emit ShowCustomMenu(event->globalPos());
}

uint32 HierarchyTreeControl::GetInternalIndex(QTreeWidgetItem* item, int& factor) const
{
	int idx = 0;
	QTreeWidgetItem* parent = item->parent();
	if (parent)
	{
		idx = parent->indexOfChild(item) + 1;
		int a = GetInternalIndex(parent, factor);
		factor *= 10;
		idx = idx * factor + a;
	}
	else
	{
		for(int i = 0; i < topLevelItemCount(); ++i)
		{
			if (item == topLevelItem(i))
			{
				return i + 1;
			}
		}
	}

	return idx;
}

uint32 HierarchyTreeControl::GetInternalIndex(QTreeWidgetItem* item) const
{
	int factor = 1;
	return GetInternalIndex(item, factor);
}

bool HierarchyTreeControl::SortByInternalIndex(const SortedItems &first, const SortedItems &second)
{
	int firstIdx = first.internalIndex;
	int secondIdx = second.internalIndex;
	while (firstIdx)
	{
		int firstId = firstIdx % 10;
		int secondId = secondIdx % 10;
		firstIdx /= 10;
		secondIdx /= 10;
		if (firstId > secondId)
			return true;
		if (firstId < secondId)
			return false;
	}
	return false;
}

QMimeData* HierarchyTreeControl::mimeData(const QList<QTreeWidgetItem*> items) const
{
	std::list<SortedItems> sortedItems;
	
	for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		sortedItems.push_back(SortedItems(item, GetInternalIndex(item)));
	}
	
	sortedItems.sort(SortByInternalIndex);
	QList<QTreeWidgetItem* > qSortedItems;
	for (std::list<SortedItems>::iterator iter = sortedItems.begin(); iter != sortedItems.end(); ++iter)
	{
		qSortedItems.push_back(iter->item);
	}
	
	QMimeData* data = QTreeWidget::mimeData(qSortedItems);
	data->setUserData(TREE_MIME_DATA, new HierarchyTreeControlMimeData(qSortedItems));
	return data;
}

void HierarchyTreeControl::dropEvent(QDropEvent *event)
{
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return;
	
	HierarchyTreeNode::HIERARCHYTREENODEID insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode::HIERARCHYTREENODEID insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	if (!GetMoveItemID(event, insertInTo, insertAfter))
		return;
	
	HierarchyTreeNode* parentNode = HierarchyTreeController::Instance()->GetTree().GetNode(insertInTo);
	if (!parentNode)
	{
		parentNode = (HierarchyTreeNode*) HierarchyTreeController::Instance()->GetTree().GetRootNode();
		insertInTo = parentNode->GetId();
	}
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;
	
	if (!mimeData->IsDropEnable(parentNode))
		return;
		
	//Copy current selected item(s) if ctrl key is pressed during drag
	if (event->keyboardModifiers() == Qt::ControlModifier)
	{
		CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
		CopyPasteController::Instance()->Paste(parentNode);
	}
	else //Otherwise move item(s)
	{
		HierarchyTreeNode::HIERARCHYTREENODESIDLIST items = mimeData->GetItems();
		ChangeNodeHeirarchy* cmd = new ChangeNodeHeirarchy(insertInTo, insertAfter, items);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}
 
void HierarchyTreeControl::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData())
		return;
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;

	HierarchyTreeNode * selectedTreeNode = HierarchyTreeController::Instance()->GetTree().GetNode(*(mimeData->GetItems().begin()));
	HierarchyTreeControlNode * selectedControlNode =  dynamic_cast<HierarchyTreeControlNode*>(selectedTreeNode);

	HierarchyTreeAggregatorNode * aggregatorNode =  dynamic_cast<HierarchyTreeAggregatorNode*>(selectedControlNode);
	if (aggregatorNode)
	{
		// Don't allow to drop anything to aggregator.
		return;
	}

	if(selectedControlNode)
	{
		if(SubcontrolsHelper::ControlIsSubcontrol(selectedControlNode->GetUIObject()))
		{
			return;
		}
	}

	event->accept();
}

void HierarchyTreeControl::dragMoveEvent(QDragMoveEvent *event)
{
	QTreeWidget::dragMoveEvent(event);

	event->ignore();
	
	if (!event->mimeData())
		return;

	HierarchyTreeNode::HIERARCHYTREENODEID insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode::HIERARCHYTREENODEID insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	if (!GetMoveItemID(event, insertInTo, insertAfter))
		return;
	
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(insertInTo);
	if (!node)
		node = (HierarchyTreeNode*) HierarchyTreeController::Instance()->GetTree().GetRootNode();
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;

	HierarchyTreeAggregatorControlNode* aggregatorControlNode = dynamic_cast<HierarchyTreeAggregatorControlNode*>(node);
	if (aggregatorControlNode)
	{
		// Don't allow to drop controls to aggregator controls.
		return;
	}

	if (mimeData->IsDropEnable(node))
	{
		event->accept();
	}
}

bool HierarchyTreeControl::GetMoveItemID(QDropEvent *event, HierarchyTreeNode::HIERARCHYTREENODEID &insertInTo, HierarchyTreeNode::HIERARCHYTREENODEID &insertAfter)
{
	DropIndicatorPosition position = dropIndicatorPosition();
	
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return false;

	if (item == currentItem())
		return false;

	insertInTo = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	
	switch (position)
	{
		case OnViewport:
		{
			return false;
		}break;
		case OnItem:
		{
			QVariant data = item->data(ITEM_ID);
			insertInTo = data.toInt();
			insertAfter = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
		} break;
		case AboveItem:
		{
			QTreeWidgetItem* parent = item->parent();
			if (parent)
				insertInTo = parent->data(ITEM_ID).toInt();
			QTreeWidgetItem* above = itemAbove(item);
			if (!above)
				return false;
			insertAfter = above->data(ITEM_ID).toInt();
		} break;
		case BelowItem:
		{
			QTreeWidgetItem* parent = item->parent();
			if (parent)
				insertInTo = parent->data(ITEM_ID).toInt();
			insertAfter = item->data(ITEM_ID).toInt();
		}break;
	}
	
	if (currentItem()->data(ITEM_ID) == insertAfter)
		return false;

	return true;
}
