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


#ifndef HIERARCHYTREECONTROL_H
#define HIERARCHYTREECONTROL_H

#include <QTreeWidget>
#include "HierarchyTreeNode.h"

#include "Classes/UI/controllist.h"

class HierarchyTreeControlMimeData: public QObjectUserData
{
public:
	HierarchyTreeControlMimeData(const QList<QTreeWidgetItem*> items);
	virtual ~HierarchyTreeControlMimeData();
	
	bool IsDropEnable(const HierarchyTreeNode* parentItem) const;
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST GetItems() const;
	
private:
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST items;
};

class HierarchyTreeControl : public QTreeWidget
{
    Q_OBJECT
public:
    explicit HierarchyTreeControl(QWidget *parent = 0);
	
protected:
	virtual void contextMenuEvent(QContextMenuEvent * event);
	
	virtual QMimeData *mimeData(const QList<QTreeWidgetItem*> items) const;
	
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
	
	// Custom Drag&Drop handlers for different mime data.
	void HandleDragEnterControlMimeData(QDragEnterEvent *event, const ControlMimeData* mimeData);
	void HandleDragEnterHierarchyMimeData(QDragEnterEvent *event, const HierarchyTreeControlMimeData* mimeData);
	
	void HandleDragMoveControlMimeData(QDragMoveEvent *event, const ControlMimeData* mimeData);
	void HandleDragMoveHierarchyMimeData(QDragMoveEvent *event, const HierarchyTreeControlMimeData* mimeData);

	void HandleDropControlMimeData(QDropEvent *event, const ControlMimeData* mimeData);
	void HandleDropHierarchyMimeData(QDropEvent *event, const HierarchyTreeControlMimeData* mimeData);

signals:
	void ShowCustomMenu(const QPoint& pos);
    
private:
	bool GetMoveItemID(QDropEvent *event, HierarchyTreeNode::HIERARCHYTREENODEID &insertInTo, HierarchyTreeNode::HIERARCHYTREENODEID &insertAfter);

	Vector<int32> GetPositionKey(QTreeWidgetItem* item) const;
	
	struct SortedItems {
		QTreeWidgetItem* item;
		Vector<int32> positionKey;
        
		SortedItems(QTreeWidgetItem* item, const Vector<int32>& positionKey)
		{
			this->item = item;
			this->positionKey = positionKey;
		}
	};
	static bool SortByInternalIndex(const SortedItems &first, const SortedItems &second);
};

#endif // HIERARCHYTREECONTROL_H
