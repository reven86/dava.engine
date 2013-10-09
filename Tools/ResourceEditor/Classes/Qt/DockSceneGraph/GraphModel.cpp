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



#include "GraphModel.h"
#include "GraphItem.h"

#include <QTreeView>
#include <QMimeData>
#include <QApplication>

#include "Main/QtUtils.h"
#include "DockSceneGraph/PointerHolder.h"

#include "DAVAEngine.h"
using namespace DAVA;

GraphModel::GraphModel(QObject *parent)
    :   QAbstractItemModel(parent)
    ,   rootItem(NULL)
    ,   attachedTreeView(NULL)
{
    itemSelectionModel = new QItemSelectionModel(this);
    
    connect(itemSelectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
}

GraphModel::~GraphModel()
{
    SafeDelete(itemSelectionModel);
	SafeRelease(rootItem);
}

GraphItem * GraphModel::ParentItem(const QModelIndex &parent) const
{
	if (parent.isValid())
	{
		return static_cast<GraphItem*>(parent.internalPointer());
	}

    return rootItem;
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

    GraphItem *parentItem = ParentItem(parent);
    GraphItem *childItem = parentItem->Child(row);
    if (childItem)
	{
		return createIndex(row, column, childItem);
	}

	return QModelIndex();
}

QModelIndex GraphModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return QModelIndex();
	}

    GraphItem *childItem = static_cast<GraphItem*>(index.internalPointer());
    GraphItem *parentItem = childItem->GetParent();

    if(parentItem && (parentItem != rootItem))
    {
        return createIndex(parentItem->Row(), 0, parentItem);
    }

    return QModelIndex();
}

int GraphModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
	{
		return 0;
	}

    GraphItem *parentItem = ParentItem(parent);
    if(parentItem)
    {
        return parentItem->ChildrenCount();
    }

    return 0;
}

int GraphModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
	{
		return static_cast<GraphItem*>(parent.internalPointer())->ColumnCount();
	}

	return rootItem->ColumnCount();
}

QVariant GraphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
	{
		return QVariant();
	}

    if (Qt::DisplayRole == role)
	{
        GraphItem *item = static_cast<GraphItem*>(index.internalPointer());
        return item->Data(index.column());
	}

    return QVariant();
}

Qt::ItemFlags GraphModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return 0;
	}

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant GraphModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && rootItem)
	{
		return rootItem->Data(section);
	}

    return QVariant();
}

QItemSelectionModel *GraphModel::GetSelectionModel()
{
    return itemSelectionModel;
}


GraphItem * GraphModel::ItemForData(void * data)
{
    if(rootItem)
    {
        return ItemForData(rootItem, data);
    }
    
    return NULL;
}

GraphItem * GraphModel::ItemForData(GraphItem *item, void * data)
{
    if(item->GetUserData() == data)
    {
        return item;
    }
    
    for(int32 i = 0; i < item->ChildrenCount(); ++i)
    {
        GraphItem *foundItem = ItemForData(item->Child(i), data);
        if(foundItem)
        {
            return foundItem;
        }
    }
    
    return NULL;
}

void GraphModel::Deactivate()
{
    attachedTreeView = NULL;
}

void GraphModel::Activate(QTreeView *view)
{
    DVASSERT((NULL == attachedTreeView) && "View must be deactivated")
    
    attachedTreeView = view;
    
    attachedTreeView->setModel(this);
    attachedTreeView->setSelectionModel(itemSelectionModel);
}

void * GraphModel::ItemData(const QModelIndex &index) const
{
    if (index.isValid())
	{
        GraphItem *item = static_cast<GraphItem*>(index.internalPointer());
        return item->GetUserData();
	}
    
    return NULL;
}



QStringList GraphModel::mimeTypes() const
{
    QStringList types;
    types << "application/tree.userdata";
    return types;
}

QMimeData *GraphModel::mimeData(const QModelIndexList &indexes) const
{
	if(QApplication::keyboardModifiers() & Qt::ShiftModifier)
	{
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;

		QDataStream stream(&encodedData, QIODevice::WriteOnly);

		foreach (const QModelIndex &index, indexes)
		{
			if (index.isValid())
			{
				GraphItem *item = static_cast<GraphItem *>(index.internalPointer());
				QVariant uData = PointerHolder<GraphItem *>::ToQVariant(item);
				stream << uData;
			}
		}

		mimeData->setData("application/tree.userdata", encodedData);
		return mimeData;
	}
	
	return NULL;
}

bool GraphModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                          int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;
    
    if (!data->hasFormat("application/tree.userdata") || (column > 0))
        return false;
    
    QByteArray encodedData = data->data("application/tree.userdata");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

	bool movePerformed = false;
    while (!stream.atEnd())
    {
        QVariant uData;
        stream >> uData;

        GraphItem *movedItem = PointerHolder<GraphItem *>::ToPointer(uData);
        movePerformed = MoveItemToParent(movedItem, parent);
    }
    
	if (movePerformed)
	{
		Rebuild();
	}

    return movePerformed;
}

