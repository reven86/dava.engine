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


#include "Classes/UI/hierarchytreewidget.h"
#include "ui_hierarchytreewidget.h"
#include "HierarchyTreeController.h"
#include "HierarchyTree.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "IconHelper.h"
#include "SubcontrolsHelper.h"
#include "ResourcesManageHelper.h"
#include "WidgetSignalsBlocker.h"

#include <QVariant>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>

#define ITEM_ID 0, Qt::UserRole

#define MENU_ITEM_DELETE tr("Delete")
#define MENU_ITEM_COPY tr("Copy")
#define MENU_ITEM_PASTE tr("Paste")
#define MENU_ITEM_CREATE_SCREEN tr("Create screen")
#define MENU_ITEM_CREATE_AGGREGATOR tr("Create aggregator")
#define MENU_ITEM_IMPORT_SCREEN_OR_AGGREGATOR tr("Import screen or aggregator")

#define DEFAULT_CONTROL_FONT_COLOR QColor(0x00, 0x00, 0x00, 0xFF)
#define SUBCONTROL_FONT_COLOR QColor(0x80, 0x80, 0x80, 0xFF)

HierarchyTreeWidget::HierarchyTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyTreeWidget)
{
    ui->setupUi(this);
	
	connect(HierarchyTreeController::Instance(), SIGNAL(HierarchyTreeUpdated(bool)), this, SLOT(OnTreeUpdated(bool)));
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &)),
			this,
			SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &)));
	
	connect(ui->treeWidget, SIGNAL(ShowCustomMenu(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
	connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTreeItemChanged(QTreeWidgetItem*, int)));

	internalSelectionChanged = false;
}

HierarchyTreeWidget::~HierarchyTreeWidget()
{
    delete ui;
}

void HierarchyTreeWidget::OnTreeUpdated(bool needRestoreSelection)
{
	EXPANDEDITEMS expandedItems;
	EXPANDEDITEMS selectedItems;
	//save opened node
	TREEITEMS oldItems = GetAllItems();
	for (TREEITEMS::iterator iter = oldItems.begin(); iter != oldItems.end(); ++iter) {
		QTreeWidgetItem* item = iter->second;
		if (item->isExpanded())
		{
			QVariant data = item->data(ITEM_ID);
			expandedItems.insert(data.toInt());
		}
	}

	//save selected node
	for (TREEITEMS::iterator iter = oldItems.begin(); iter != oldItems.end(); ++iter) {
		QTreeWidgetItem* item = iter->second;
		if (item->isSelected())
		{
			QVariant data = item->data(ITEM_ID);
			selectedItems.insert(data.toInt());
			HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(data.toInt());
			HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);
			if(NULL != selectedControl)
			{
				internalSelectionChanged = true;
			}
		}
	}
	
	//reset tree
	ui->treeWidget->clear();

	const HierarchyTree& tree = HierarchyTreeController::Instance()->GetTree();
	const HierarchyTreeRootNode* rootNode = tree.GetRootNode();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = rootNode->GetChildNodes().begin();
		 iter != rootNode->GetChildNodes().end();
		 ++iter)
	{
		//add platform node
		const HierarchyTreePlatformNode* platformNode = (const HierarchyTreePlatformNode*)(*iter);
		QTreeWidgetItem* platformItem = new QTreeWidgetItem();
		platformItem->setData(ITEM_ID, platformNode->GetId());

		QString platformName = platformNode->GetName();
		if (platformNode->IsNeedSave())
		{
			platformName += " *";
		}
		platformItem->setText(0, platformName);

		platformItem->setIcon(0, QIcon(IconHelper::GetPlatformIconPath()));

		ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), platformItem);
		
		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = platformNode->GetChildNodes().begin();
			 iter != platformNode->GetChildNodes().end();
			 ++iter)
		{
			const HierarchyTreeScreenNode* screenNode = dynamic_cast<const HierarchyTreeScreenNode*>(*iter);
			DVASSERT(screenNode);

			QTreeWidgetItem* screenItem = new QTreeWidgetItem();
			screenItem->setData(ITEM_ID, screenNode->GetId());

			// Check whether this screen was changed.
			QString screenItemText = screenNode->GetName();
			if (screenNode->IsNeedSave())
			{
				screenItemText += " *";
			}
			screenItem->setText(0, screenItemText);

			if (dynamic_cast<const HierarchyTreeAggregatorNode*>(screenNode))
				screenItem->setIcon(0, QIcon(IconHelper::GetAggregatorIconPath()));
			else
				screenItem->setIcon(0, QIcon(IconHelper::GetScreenIconPath()));
			platformItem->insertChild(platformItem->childCount(), screenItem);
			
			AddControlItem(screenItem, screenNode->GetChildNodes());
		}
	}

	// Restore the selected items only after the tree is fully built.
	int itemsCount = ui->treeWidget->topLevelItemCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		QTreeWidgetItem* rootItem = ui->treeWidget->topLevelItem(i);
		RestoreTreeItemExpandedStateRecursive(rootItem, expandedItems);
		
		if (needRestoreSelection)
		{
			RestoreTreeItemSelectedStateRecursive(rootItem, selectedItems);
		}
	}

	internalSelectionChanged = false;
}

void HierarchyTreeWidget::AddControlItem(QTreeWidgetItem* parent, const HierarchyTreeNode::HIERARCHYTREENODESLIST& items)
{
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* controlNode = (const HierarchyTreeControlNode*)(*iter);
		
		QTreeWidgetItem* controlItem = new QTreeWidgetItem();
		controlItem->setData(ITEM_ID, controlNode->GetId());
		controlItem->setText(0, controlNode->GetName());
		controlItem->setCheckState(0, controlNode->GetVisibleFlag() ? Qt::Checked : Qt::Unchecked);

		Decorate(controlItem, controlNode->GetUIObject());

		parent->insertChild(parent->childCount(), controlItem);

		// Perform the recursive call.
		AddControlItem(controlItem, controlNode->GetChildNodes());
	}
}

void HierarchyTreeWidget::RestoreTreeItemSelectedStateRecursive(QTreeWidgetItem* parentItem,
	const EXPANDEDITEMS& selectedItems)
{
	if (!parentItem )
	{
		return;
	}

	QVariant itemIDData = parentItem->data(ITEM_ID);
	if (itemIDData.type() != QVariant::Int)
	{
		return;
	}

	int itemID = itemIDData.toInt();
	if (selectedItems.find(itemID) != selectedItems.end() && 
		!parentItem->isSelected())
	{
		parentItem->setSelected(true);
	}

	// Repeat for all children.
	for (int i = 0; i < parentItem->childCount(); i ++)
	{
		RestoreTreeItemSelectedStateRecursive(parentItem->child(i), selectedItems);
	}
}

void HierarchyTreeWidget::RestoreTreeItemExpandedStateRecursive(QTreeWidgetItem* parentItem,
	const EXPANDEDITEMS& expandedItems)
{
	if (!parentItem )
	{
		return;
	}

	QVariant itemIDData = parentItem->data(ITEM_ID);
	if (itemIDData.type() != QVariant::Int)
	{
		return;
	}

	int itemID = itemIDData.toInt();
	if (expandedItems.find(itemID) != expandedItems.end() &&
		parentItem->childCount() > 0 && !parentItem->isExpanded())
	{
		parentItem->setExpanded(true);
	}

	// Repeat for all children.
	for (int i = 0; i < parentItem->childCount(); i ++)
	{
		RestoreTreeItemExpandedStateRecursive(parentItem->child(i), expandedItems);
	}
}

void HierarchyTreeWidget::Decorate(QTreeWidgetItem *item, DAVA::UIControl *uiControl)
{
	if (!item || !uiControl)
	{
		return;
	}

	QString iconPath = IconHelper::GetIconPathForUIControl(uiControl);
	item->setIcon(0, QIcon(iconPath));
	item->setTextColor(0, SubcontrolsHelper::ControlIsSubcontrol(uiControl) ? SUBCONTROL_FONT_COLOR : DEFAULT_CONTROL_FONT_COLOR);
}

void HierarchyTreeWidget::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* selectedItem = ui->treeWidget->currentItem();
	if (!selectedItem)
	{
		return;
	}

	if (internalSelectionChanged)
	{
		return;
	}

	QVariant data = selectedItem->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);
	
	if (!selectedPlatform && !selectedScreen && !selectedControl)
		return;
	
	internalSelectionChanged = true;
	
	//only platform or screen node can be seleted
	if (selectedPlatform || selectedScreen)
	{
		ResetSelection();
		HierarchyTreeController::Instance()->ResetSelectedControl();
		selectedItem->setSelected(true);
	}
	
	if (selectedControl)
	{
		selectedScreen = selectedControl->GetScreenNode();
		selectedPlatform = selectedScreen->GetPlatform();
	}
	else if (selectedScreen)
	{
		selectedPlatform = selectedScreen->GetPlatform();
	}
		
	HierarchyTreeController::Instance()->UpdateSelection(selectedPlatform, selectedScreen);
	HierarchyTreeController::Instance()->UpdateSelection(baseNode);
	
	internalSelectionChanged = false;
	
	if (selectedControl)
	{
		int32 selectedItemsSize = ui->treeWidget->selectedItems().size();
		if (selectedItemsSize == 0)
		{
			// Just reset selected control and don't select anything else.
			// See please DF-2377 for details.
			HierarchyTreeController::Instance()->ResetSelectedControl();
			return;
		}
		else if (selectedItemsSize == 1)
		{
			// Only one control is selected, reset the previous selection and continue.
			HierarchyTreeController::Instance()->ResetSelectedControl();
		}

		HierarchyTreeController::Instance()->SelectControl(selectedControl);
	}
}

void HierarchyTreeWidget::ResetSelection()
{
	QList<QTreeWidgetItem*> selectedList = ui->treeWidget->selectedItems();
	for (QList<QTreeWidgetItem*>::iterator iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
		QTreeWidgetItem* item = (*iter);
		item->setSelected(false);
	}
}

void HierarchyTreeWidget::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedControls)
{
	if (internalSelectionChanged)
		return;
	
	internalSelectionChanged = true;
	ResetSelection();
	
	TREEITEMS items = GetAllItems();
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedControls.begin();
		 iter != selectedControls.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* node = (*iter);
		TREEITEMS::iterator itemIter = items.find(node->GetId());
		if (itemIter != items.end())
		{
            QTreeWidgetItem* item = itemIter->second;
			item->setSelected(true);
			
			// Force show selected item
            QTreeWidgetItem* parentItem = item->parent();
            while (parentItem)
            {
                parentItem->setExpanded(true);
                parentItem = parentItem->parent();
            }
		}
	}

	internalSelectionChanged = false;
}

HierarchyTreeWidget::TREEITEMS HierarchyTreeWidget::GetAllItems()
{
	TREEITEMS items;
	
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
		if (!item)
			continue;
		
		QVariant data = item->data(ITEM_ID);
		items[data.toInt()] = item;
		
		if (item->childCount())
			GetChildItems(item, items);
	}
	
	return items;
}

void HierarchyTreeWidget::GetChildItems(const QTreeWidgetItem* parent, TREEITEMS &items)
{
	for(int i = 0; i < parent->childCount(); i++)
	{
		QTreeWidgetItem* item = parent->child(i);

		QVariant data = item->data(ITEM_ID);
		items[data.toInt()] = item;
		
		if (item->childCount())
			GetChildItems(item, items);
	}
}

void HierarchyTreeWidget::OnShowCustomMenu(const QPoint& pos)
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);

	QMenu menu;
	if (selectedControl || selectedPlatform || selectedScreen)
	{
		// Yuri Coder, 2013/03/29. Deletion is disabled for Subcontrols.
		if (IsDeleteNodeAllowed(selectedControl))
		{
			QAction* deleteControlAction = new QAction(MENU_ITEM_DELETE, &menu);
			connect(deleteControlAction, SIGNAL(triggered()), this, SLOT(OnDeleteControlAction()));
			menu.addAction(deleteControlAction);
		}

		QAction* copyControlAction = new QAction(MENU_ITEM_COPY, &menu);
		connect(copyControlAction, SIGNAL(triggered()), this, SLOT(OnCopyAction()));
		menu.addAction(copyControlAction);
	}
	if (selectedPlatform)
	{
		QAction* createScreenAction = new QAction(MENU_ITEM_CREATE_SCREEN, &menu);
		connect(createScreenAction, SIGNAL(triggered()), this, SLOT(OnCreateScreenAction()));
		menu.addAction(createScreenAction);
		
		QAction* createAggregatorAction = new QAction(MENU_ITEM_CREATE_AGGREGATOR, &menu);
		connect(createAggregatorAction, SIGNAL(triggered()), this, SLOT(OnCreateAggregatorAction()));
		menu.addAction(createAggregatorAction);

		QAction* importScreenOrAggregatorAction = new QAction(MENU_ITEM_IMPORT_SCREEN_OR_AGGREGATOR, &menu);
		connect(importScreenOrAggregatorAction, SIGNAL(triggered()), this, SLOT(OnImportScreenOrAggregatorAction()));
		menu.addAction(importScreenOrAggregatorAction);

		if (CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeScreen ||
			CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeAggregator)
		{
			QAction* pasteScreenAction = new QAction(MENU_ITEM_PASTE, &menu);
			connect(pasteScreenAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
			menu.addAction(pasteScreenAction);
		}
	}
	else if (selectedScreen || selectedControl)
	{
		if (CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeControl)
		{
			QAction* pasteScreenAction = new QAction(MENU_ITEM_PASTE, &menu);
			connect(pasteScreenAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
			menu.addAction(pasteScreenAction);
		}
	}
	menu.exec(pos);
}

void HierarchyTreeWidget::OnDeleteControlAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	bool needConfirm = false;
	bool needDeleteFiles = false;

	// DF-1273 - Remove all child nodes. We don't have to remove them here.
	QList<QTreeWidgetItem*> parentItems(items);
	for (QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		HierarchyTreeNode* node = GetNodeFromTreeItem(*iter);
		
		if (!node)
			continue;
				
		for (QList<QTreeWidgetItem*>::iterator innerIter = items.begin(); innerIter != items.end(); ++innerIter)
		{
			HierarchyTreeNode* innerNode = GetNodeFromTreeItem(*innerIter);
				
			if (node->IsHasChild(innerNode))
			{
				parentItems.removeOne(*innerIter);
			}
		}
	}	

	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	for (QList<QTreeWidgetItem*>::iterator iter = parentItems.begin(); iter != parentItems.end(); ++iter)
	{
		HierarchyTreeNode* node = GetNodeFromTreeItem(*iter);
		
		HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregatorNode)
		{
			const HierarchyTreeAggregatorNode::CHILDS& childs = aggregatorNode->GetChilds();
			needConfirm |= (childs.size() > 0);
			for (HierarchyTreeAggregatorNode::CHILDS::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
			{
				nodes.push_back((*iter));
			}
		}

		if (aggregatorNode ||
			dynamic_cast<HierarchyTreeScreenNode*>(node) ||
			dynamic_cast<HierarchyTreePlatformNode*>(node))
		{
			QMessageBox messageBox;
			messageBox.setText(tr("Delete nodes"));
			messageBox.setInformativeText(tr("Do you want to remove selected nodes only from project, or delete their files from disk?"));
			QAbstractButton* removeFromProjectButton = (QAbstractButton*)messageBox.addButton(tr("Remove from project"),
																							  QMessageBox::YesRole);
			QAbstractButton* deleteFromProjectButton = (QAbstractButton*)messageBox.addButton(tr("Delete from disk"),
																							  QMessageBox::YesRole);
			QAbstractButton* cancelButton = (QAbstractButton*)messageBox.addButton(tr("Cancel"),
																				   QMessageBox::RejectRole);
			messageBox.setDefaultButton((QPushButton*)removeFromProjectButton);
			messageBox.setIcon(QMessageBox::Question);
			messageBox.exec();

			if (messageBox.clickedButton() == removeFromProjectButton)
			{
				Logger::Debug("removeFromProjectButton");
			}
			if (messageBox.clickedButton() == deleteFromProjectButton)
			{
				needDeleteFiles = true;
				Logger::Debug("deleteFromProjectButton");
			}
			if (messageBox.clickedButton() == cancelButton)
			{
				Logger::Debug("cancelButton");
				return;
			}
		}
		
		nodes.push_front(node);
	}
	
	if (needConfirm)
	{
		if (QMessageBox::No == QMessageBox::information(this,
								 "",
								 "Selected aggregator control has child controls. Do you want delete aggregator with all child controls?",
								 QMessageBox::Yes | QMessageBox::No))
			return;
	}
	
	DeleteSelectedNodeCommand* cmd = new DeleteSelectedNodeCommand(nodes, needDeleteFiles);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void HierarchyTreeWidget::OnImportScreenOrAggregatorAction()
{
	emit ImportScreenOrAggregator();
}

void HierarchyTreeWidget::OnCreateScreenAction()
{
	emit CreateNewScreen();
}

void HierarchyTreeWidget::OnCreateAggregatorAction()
{
	emit CreateNewAggregator();
}

void HierarchyTreeWidget::OnCopyAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);
	
	if (selectedControl)
	{
		CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
	}
	else if (selectedScreen)
	{
		HierarchyTreeNode::HIERARCHYTREENODESLIST list;
		list.push_back(selectedScreen);
		CopyPasteController::Instance()->Copy(list);
	}
	else if (selectedPlatform)
	{
		HierarchyTreeNode::HIERARCHYTREENODESLIST list;
		list.push_back(selectedPlatform);
		CopyPasteController::Instance()->Copy(list);
		CopyPasteController::Instance()->Paste(selectedPlatform->GetRoot());
	}
}

void HierarchyTreeWidget::OnPasteAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;
	
	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);

	CopyPasteController::Instance()->Paste(baseNode);
}

bool HierarchyTreeWidget::IsDeleteNodeAllowed(HierarchyTreeControlNode* selectedControlNode)
{
	if (!selectedControlNode)
	{
		return true;
	}
		
	// Check whether selected control is Subcontrol of its parent.
	UIControl* uiControl = selectedControlNode->GetUIObject();
	if (!uiControl)
	{
		return true;
	}

	// Subcontrols cannot be deleted.
	return !uiControl->IsSubcontrol();
}

HierarchyTreeNode* HierarchyTreeWidget::GetNodeFromTreeItem(QTreeWidgetItem* item)
{
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	return HierarchyTreeController::Instance()->GetTree().GetNode(id);
}

void HierarchyTreeWidget::OnTreeItemChanged(QTreeWidgetItem *item, int column)
{
	bool currentVisibleFlag = (item->checkState(column) == Qt::Checked);
	
	WidgetSignalsBlocker blocker(this);
	UpdateVisibleFlagRecursive(item, column, currentVisibleFlag);
}

void HierarchyTreeWidget::UpdateVisibleFlagRecursive(QTreeWidgetItem* rootItem, int column, bool flagValue)
{
	HierarchyTreeControlNode* controlNode = GetControlNodeByTreeItem(rootItem);
	if (!controlNode)
	{
		return;
	}

	rootItem->setCheckState(column, flagValue ? Qt::Checked : Qt::Unchecked);
	controlNode->SetVisibleFlag(flagValue);
	
	// Repeat for all children.
	int childCount = rootItem->childCount();
	for (int count = 0; count < childCount; count ++)
	{
		UpdateVisibleFlagRecursive(rootItem->child(count), column, flagValue);
	}
}

HierarchyTreeControlNode* HierarchyTreeWidget::GetControlNodeByTreeItem(QTreeWidgetItem* item)
{
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	return dynamic_cast<HierarchyTreeControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(id));
}

