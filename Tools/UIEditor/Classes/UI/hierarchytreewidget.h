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


#ifndef HIERARCHYTREEWIDGET_H
#define HIERARCHYTREEWIDGET_H

#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeController.h"

#include <QWidget>
#include <QTreeWidgetItem>
#include <QStandardItem>

namespace Ui {
class HierarchyTreeWidget;
}

class HierarchyTreeWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit HierarchyTreeWidget(QWidget *parent = 0);
    ~HierarchyTreeWidget();
    void ScrollTo(HierarchyTreeNode *node);
    void HighlightScreenNodes(const QList<HierarchyTreeScreenNode*>& foundNodesList);

    // Delete the requested nodes.
    void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& selectedNodes);

private:
	typedef Set<HierarchyTreeNode::HIERARCHYTREENODEID> EXPANDEDITEMS;
	void AddControlItem(QTreeWidgetItem* parent, const HierarchyTreeNode::HIERARCHYTREENODESLIST& items);

	// Restore the selected/expanded tree item state.
	void RestoreTreeItemSelectedStateRecursive(QTreeWidgetItem* parentItem, const EXPANDEDITEMS& selectedItems);
	void RestoreTreeItemExpandedStateRecursive(QTreeWidgetItem* parentItem, const EXPANDEDITEMS& expandedItems);
    
    void HandleItemSelectionChanged();
signals:
	void CreateNewScreen();
	void CreateNewAggregator();

	void ImportScreenOrAggregator();

public slots:
	void OnDeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& selectedNodes);

protected slots:
	void OnTreeUpdated(bool needRestoreSelection);
	void OnTreeItemChanged(QTreeWidgetItem *item, int column);

private slots:
    void on_treeWidget_itemSelectionChanged();
	void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &, HierarchyTreeController::eExpandControlType expandType);
	void OnShowCustomMenu(const QPoint& pos);
	void OnDeleteControlAction();
	void OnRenameControlAction();
	void OnCreateScreenAction();
	void OnCreateAggregatorAction();
	void OnCopyAction();
	void OnPasteAction();
	void OnImportScreenOrAggregatorAction();

private:
	typedef Map<int, QTreeWidgetItem*> TREEITEMS;
	TREEITEMS GetAllItems();
	void GetChildItems(const QTreeWidgetItem* parent, Map<int, QTreeWidgetItem*> &items);

    // Select multiple items in the Hierarchy Tree which belong to the same screen.
    void Select(const QList<QTreeWidgetItem*>& selectedItems);

	void ResetSelection();
	
	// Initialize and setup copy/paste/delete actions for tree widget
	void InitializeTreeWidgetActions();

	// Get hierarchy tree node from selected tree item
	HierarchyTreeNode* GetNodeFromTreeItem(QTreeWidgetItem* item);

	// Apply the icon, font color etc to the tree item.
	void Decorate(QTreeWidgetItem* item, UIControl* uiControl);
	bool IsDeleteNodeAllowed(HierarchyTreeControlNode* selectedControlNode);

	// "UI Editor Visible Flag" behavior.
	HierarchyTreeControlNode* GetControlNodeByTreeItem(QTreeWidgetItem* item);

    virtual bool eventFilter(QObject *target, QEvent *event);

private:
	bool internalSelectionChanged;
    Ui::HierarchyTreeWidget *ui;
};

#endif // HIERARCHYTREEWIDGET_H
