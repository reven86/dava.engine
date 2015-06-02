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
#include "ControlCommands.h"
#include "CommandsController.h"
#include "LibraryController.h"
#include "CopyPasteController.h"
#include "IconHelper.h"
#include "SubcontrolsHelper.h"
#include "ResourcesManageHelper.h"
#include "WidgetSignalsBlocker.h"
#include "LibraryController.h"

#include "regexpinputdialog.h"

#include <QVariant>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>

#define ITEM_ID 0, Qt::UserRole

#define MENU_ITEM_DELETE tr("Delete")
#define MENU_ITEM_COPY tr("Copy")
#define MENU_ITEM_RENAME tr("Rename")
#define MENU_ITEM_PASTE tr("Paste")
#define MENU_ITEM_CREATE_SCREEN tr("Create screen")
#define MENU_ITEM_CREATE_AGGREGATOR tr("Create aggregator")
#define MENU_ITEM_IMPORT_SCREEN_OR_AGGREGATOR tr("Import screen or aggregator")

#define DEFAULT_CONTROL_FONT_COLOR QApplication::palette().text().color()
#define SUBCONTROL_FONT_COLOR QApplication::palette().dark().color()

HierarchyTreeWidget::HierarchyTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyTreeWidget)
{
    ui->setupUi(this);
	
	connect(HierarchyTreeController::Instance(), SIGNAL(HierarchyTreeUpdated(bool)), this, SLOT(OnTreeUpdated(bool)));
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &, HierarchyTreeController::eExpandControlType)),
			this,
			SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &, HierarchyTreeController::eExpandControlType)));
	
	connect(ui->treeWidget, SIGNAL(ShowCustomMenu(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
	connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTreeItemChanged(QTreeWidgetItem*, int)));
	
	InitializeTreeWidgetActions();
    installEventFilter(this);

	internalSelectionChanged = false;
}

HierarchyTreeWidget::~HierarchyTreeWidget()
{
    delete ui;
}

void HierarchyTreeWidget::ScrollTo(HierarchyTreeNode *node)
{
    TREEITEMS items = GetAllItems();
	TREEITEMS::iterator itemIter = items.find(node->GetId());
    if(itemIter != items.end())
    {
        QTreeWidgetItem *item = itemIter->second;
        ui->treeWidget->setCurrentItem(item);
		HandleItemSelectionChanged();
    }
}

void HierarchyTreeWidget::HighlightScreenNodes(const QList<HierarchyTreeScreenNode*>& foundNodesList)
{
	TREEITEMS items = GetAllItems();
    // Highlight screen nodes
    WidgetSignalsBlocker blocker(ui->treeWidget);
  	foreach(HierarchyTreeScreenNode* foundNode, foundNodesList)
  	{
		TREEITEMS::iterator itemIter = items.find(foundNode->GetId());
    	if(itemIter != items.end())
    	{
        	QTreeWidgetItem *item = itemIter->second;
            item->setSelected(true);
    	}
	}
}

void HierarchyTreeWidget::InitializeTreeWidgetActions()
{
	QAction* deleteNodeAction = new QAction(MENU_ITEM_DELETE, ui->treeWidget);
	connect(deleteNodeAction, SIGNAL(triggered()), this, SLOT(OnDeleteControlAction()));
	deleteNodeAction->setShortcut(Qt::Key_Delete);
	ui->treeWidget->addAction(deleteNodeAction);
	
	QAction* copyNodeAction = new QAction(MENU_ITEM_COPY, ui->treeWidget);
	connect(copyNodeAction, SIGNAL(triggered()), this, SLOT(OnCopyAction()));
	copyNodeAction->setShortcut(Qt::CTRL + Qt::Key_C);
	ui->treeWidget->addAction(copyNodeAction);
	
	QAction* pasteNodeAction = new QAction(MENU_ITEM_PASTE, ui->treeWidget);
	connect(pasteNodeAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
	pasteNodeAction->setShortcut(Qt::CTRL + Qt::Key_V);
	ui->treeWidget->addAction(pasteNodeAction);
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
			HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
			HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
			HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);

			if(selectedPlatform || selectedScreen || selectedControl)
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
	
    LibraryController::Instance()->UpdateLibrary();
    
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
        ui->treeWidget->setCurrentItem(parentItem);
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
	HandleItemSelectionChanged();
}

void HierarchyTreeWidget::HandleItemSelectionChanged()
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
	
	internalSelectionChanged = false;
	
	if (selectedControl)
	{
        // UI Control is selected.
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

		// Yuri Coder, 2012/12/19. The focus is on Hierarchy Tree here, so can't ask InputSystem
		// whether Shift is pressed. Use Qt functions instead. If Shift is pressed - select multiple
        // items in the tree, from the one previously selected to the current selection.
		if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
        {
            Select(ui->treeWidget->selectedItems());
        }
        else
        {
            // Switch the selection state of the control instead of just selecting it (see DF-2838).
            HierarchyTreeController::Instance()->ChangeItemSelection(selectedControl, HierarchyTreeController::NoExpand);
        }
	}
    else
    {
        // Platform/Screen/Aggregator is selected.
        HierarchyTreeController::Instance()->UpdateSelection(baseNode);
    }
}

void HierarchyTreeWidget::Select(const QList<QTreeWidgetItem*>& selectedItems)
{
    HierarchyTreeControlNode* firstNode = NULL;
    bool needReselectScreen = false;
    QList<HierarchyTreeControlNode*> nodesToBeSelected;
    QList<QTreeWidgetItem*> itemsToBeSelected;
    
    // Selection is changed programmatically in this method, so block the signals to avoid
    // recursive calls.
    WidgetSignalsBlocker blocker(ui->treeWidget);
    
    // There can be situation where first and last items selected belong to the different screens.
    // So have to implement two-pass approach here - firstly determine the nodes to be selected,
    // then re-activate the screen from the first node and apply the selection.
    foreach (QTreeWidgetItem* multiSelectItem, selectedItems)
    {
        QVariant data = multiSelectItem->data(ITEM_ID);
        HierarchyTreeControlNode* multiSelectControlNode = dynamic_cast<HierarchyTreeControlNode* >(HierarchyTreeController::Instance()->GetTree().GetNode(data.toInt()));
        if (!multiSelectControlNode)
        {
            // For sure don't allow to multiselect anything but controls
            multiSelectItem->setSelected(false);
            continue;
        }
        
        if (!firstNode)
        {
            firstNode = multiSelectControlNode;
            nodesToBeSelected.append(multiSelectControlNode);
            itemsToBeSelected.append(multiSelectItem);
            continue;
        }
        
        // Select only the nodes which belong to the same screen as first node.
        if (multiSelectControlNode->GetScreenNode() == firstNode->GetScreenNode())
        {
            nodesToBeSelected.append(multiSelectControlNode);
            itemsToBeSelected.append(multiSelectItem);
        }
        else
        {
            // There are selected controls belong to different screens. Need to unselect them in a tree
            // and re-activate the screen first node belongs to prior to apply actual selection.
            needReselectScreen = true;
            multiSelectItem->setSelected(false);
        }
    }
    
    if (!firstNode || !firstNode->GetScreenNode() || !firstNode->GetScreenNode()->GetPlatform())
    {
        return;
    }
    
    if (needReselectScreen)
    {
        HierarchyTreeController::Instance()->UpdateSelection(firstNode->GetScreenNode()->GetPlatform(), firstNode->GetScreenNode());

        // Screen selection changed the tree selected item, so reselect all the tree items to be selected.
        foreach(QTreeWidgetItem* reselectedItem, itemsToBeSelected)
        {
            reselectedItem->setSelected(true);
        }
    }
    
    // Second pass - select the controls remembered before. Note - selected controls
    // are already handled by the tree, so block OnSelectedControlNodesChanged.
    internalSelectionChanged = true;
    HierarchyTreeController::Instance()->SynchronizeSelection(nodesToBeSelected);
    internalSelectionChanged = false;
}

void HierarchyTreeWidget::ResetSelection()
{
	QList<QTreeWidgetItem*> selectedList = ui->treeWidget->selectedItems();
	for (QList<QTreeWidgetItem*>::iterator iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
		QTreeWidgetItem* item = (*iter);
		item->setSelected(false);
	}
}

void HierarchyTreeWidget::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedControls, HierarchyTreeController::eExpandControlType expandType)
{
	if (internalSelectionChanged)
		return;
	
	internalSelectionChanged = true;
	ResetSelection();
	ui->treeWidget->StopExpandTimer();

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
			
			// Force show selected item, if requested.
            if (expandType == HierarchyTreeController::ImmediateExpand)
            {
                ui->treeWidget->ExpandItemAndScrollTo(item);
            }
		}
	}

    // Check whether we need to perform deferred expand.
    if ((selectedControls.size() == 1) &&
        (expandType == HierarchyTreeController::DeferredExpand ||
        expandType == HierarchyTreeController::DeferredExpandWithMouseCheck))
    {
        const HierarchyTreeControlNode* node = selectedControls.front();
		TREEITEMS::iterator itemIter = items.find(node->GetId());
        if (itemIter != items.end())
        {
            ui->treeWidget->StartExpandTimer(itemIter->second, expandType == HierarchyTreeController::DeferredExpandWithMouseCheck);
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
		// Show rename option only if one item is selected
		if (items.size() == 1)
		{
			QAction* renameControlAction = new QAction(MENU_ITEM_RENAME, &menu);
			connect(renameControlAction, SIGNAL(triggered()), this, SLOT(OnRenameControlAction()));
			menu.addAction(renameControlAction);
		}
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
        // Currently don't allow to paste anything to Aggregators.
		if ((dynamic_cast<HierarchyTreeAggregatorControlNode*>(selectedControl) == NULL) &&
             CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeControl)
		{
			QAction* pasteScreenAction = new QAction(MENU_ITEM_PASTE, &menu);
			connect(pasteScreenAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
			menu.addAction(pasteScreenAction);
		}
	}
	menu.exec(pos);
}

void HierarchyTreeWidget::OnRenameControlAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size() || (items.size() > 1))
		return;
		
	// We are sure that we have only one selected item
	QTreeWidgetItem* item = items.at(0);	
	HierarchyTreeNode* node = GetNodeFromTreeItem(item);
	QString itemName = node->GetName();

    bool isAccepted = false;
    QString newName = RegExpInputDialog::getText(this, tr("Rename control"),
                                              "Enter new control name",
                                              itemName, HierarchyTreeNode::GetNameRegExp(),
                                              &isAccepted);

	if (isAccepted && !newName.isEmpty() && (newName != itemName))
	{
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(node);
		// Check item name for uniqueness only for screens or platfroms
		if (screenNode || platformNode)
		{
			// Do not allow to rename to existing name
			if (HierarchyTreeController::Instance()->GetActivePlatform()->IsAggregatorOrScreenNamePresent(newName) ||
				HierarchyTreeController::Instance()->GetTree().IsPlatformNamePresent(newName))
			{
				QMessageBox msgBox;
				msgBox.setText("Agreggator, screen or platform with the same name already exist. Please fill the name field with unique value.");
				msgBox.exec();
				return;
			}
             // Do not allow library control name
            if(LibraryController::Instance()->IsControlNameExists(newName))
            {
                QMessageBox msgBox;
                msgBox.setText("A library control with the same name already exist. Please fill the name field with unique value.");
                msgBox.exec();
                return;
            }
		}
		
		ControlRenameCommand* cmd = new ControlRenameCommand(node->GetId(), itemName, newName);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void HierarchyTreeWidget::OnDeleteControlAction()
{
    // Do not handle to avoid double delete action
    if (!ui->treeWidget->hasFocus())
    {
        return;
    }

    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if (!items.size())
        return;

    // DF-1273 - Remove all child nodes. We don't have to remove them here.
    // Convert nodes to items.
    HierarchyTreeNode::HIERARCHYTREENODESLIST selectedNodes;
    for (QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
    {
        HierarchyTreeNode* node = GetNodeFromTreeItem(*iter);
        selectedNodes.push_back(node);
    }

    DeleteNodes(selectedNodes);
}

void HierarchyTreeWidget::OnDeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& selectedNodes)
{
    DeleteNodes(selectedNodes);
}

void HierarchyTreeWidget::DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& selectedNodes)
{
    // Remove the child nodes - leave parent ones only. Need to work with separate list to don't break iterators.
    HierarchyTreeNode::HIERARCHYTREENODESLIST parentNodes;
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER parentIter = selectedNodes.begin();
        parentIter != selectedNodes.end(); ++parentIter)
    {
        HierarchyTreeNode *parentNode = (*parentIter);
        HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parentNode);
        if (controlNode && !IsDeleteNodeAllowed(controlNode))
        {
            // Can't delete this node.
            continue;
        }

        bool useParentNode = true;
        for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER innerIter = selectedNodes.begin();
             innerIter != selectedNodes.end(); ++innerIter)
        {
            if (parentNode->IsHasChild(*innerIter))
            {
                useParentNode = false;
                break;
            }
        }

        if (useParentNode)
        {
            parentNodes.push_back(parentNode);
        }
    }

    bool needConfirm = false;
    bool needDeleteFiles = false;
    HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
    HierarchyTreeNode::HIERARCHYTREENODESLIST agregatorNodes;
    for (HierarchyTreeNode::HIERARCHYTREENODESITER iter = parentNodes.begin(); iter != parentNodes.end(); ++iter)
    {
        HierarchyTreeNode* node = (*iter);

        HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
        if (aggregatorNode)
        {
            const HierarchyTreeAggregatorNode::CHILDS& childs = aggregatorNode->GetChilds();
            needConfirm |= (childs.size() > 0);
            for (HierarchyTreeAggregatorNode::CHILDS::const_iterator innerIter = childs.begin(); innerIter != childs.end(); ++innerIter)
            {
                agregatorNodes.push_back((*innerIter));
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

        nodes.push_back(node);
    }
    nodes.insert(nodes.end(), agregatorNodes.begin(), agregatorNodes.end());

    if (needConfirm)
    {
        if (QMessageBox::No == QMessageBox::information(this,
            "",
            "Selected aggregator control has child controls. Do you want delete aggregator with all child controls?",
            QMessageBox::Yes | QMessageBox::No))
            return;
    }

    if (nodes.empty())
    {
        // Nothing to delete.
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
	// Do not handle to avoid double copy action
	if (!ui->treeWidget->hasFocus())
	{
		return;
	}

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
	// Do not handle to avoid double paste action
	if (!ui->treeWidget->hasFocus())
	{
		return;
	}
	
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
    HierarchyTreeControlNode* controlNode = GetControlNodeByTreeItem(item);
	if (controlNode)
	{
        controlNode->SetVisibleFlag(currentVisibleFlag);
    }
}

HierarchyTreeControlNode* HierarchyTreeWidget::GetControlNodeByTreeItem(QTreeWidgetItem* item)
{
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	return dynamic_cast<HierarchyTreeControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(id));
}

bool HierarchyTreeWidget::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() != QEvent::KeyPress || target != this)
    {
        return QWidget::eventFilter(target, event);
    }

    QKeyEvent* keyEvent = (QKeyEvent*)event;
    if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
    {
        OnDeleteControlAction();
    }

	return QWidget::eventFilter(target, event);
}

