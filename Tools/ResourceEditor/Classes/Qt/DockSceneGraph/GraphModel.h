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



#ifndef __GRAPH_MODEL_H__
#define __GRAPH_MODEL_H__

#include <QAbstractItemModel>

#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QItemSelectionModel>


class GraphItem;
class QTreeView;
class GraphModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    GraphModel(QObject *parent = 0);
    virtual ~GraphModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent);

    
    void * ItemData(const QModelIndex &index) const;
    
    virtual void Rebuild() = 0;
    
    virtual bool MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndext) = 0;
    
    
    QItemSelectionModel *GetSelectionModel();
    
    void Activate(QTreeView *view);
    void Deactivate();
    
protected slots:
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {};

    
protected:
    
    GraphItem * ParentItem(const QModelIndex &parent = QModelIndex()) const;
    
    GraphItem * ItemForData(void * usedData);
    GraphItem * ItemForData(GraphItem * item, void * usedData);
    
protected:

    GraphItem *rootItem;
    
    QItemSelectionModel *itemSelectionModel;
    
    QTreeView *attachedTreeView;
};

#endif // __GRAPH_MODEL_H__
