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


#ifndef __QT_PROPERTY_MODEL_FILTERINGH__
#define __QT_PROPERTY_MODEL_FILTERINGH__

#include <QSortFilterProxyModel>
#include "QtPropertyModel.h"

class QtPropertyModelFiltering : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	QtPropertyModelFiltering(QtPropertyModel *_propModel, QObject *parent = NULL);
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

	QtPropertyData* rootItem() const
	{ return propModel->rootItem(); }

	QtPropertyData* itemFromIndex(const QModelIndex & index) const
	{ return propModel->itemFromIndex(mapToSource(index)); }

	QModelIndex indexFromItem(QtPropertyData *data) const
	{ return mapFromSource(propModel->indexFromItem(data)); }

	QModelIndex AppendProperty(const QString &name, QtPropertyData* data, const QModelIndex &parent = QModelIndex())
	{ return mapFromSource(propModel->AppendProperty(name, data, mapToSource(parent))); }

	QModelIndex InsertProperty(const QString &name, QtPropertyData* data, int row, const QModelIndex &parent = QModelIndex())
	{ return mapFromSource(propModel->InsertProperty(name, data, row, mapToSource(parent))); }

	bool GetEditTracking()
	{ return propModel->GetEditTracking(); }

	void SetEditTracking(bool enabled)
	{ propModel->SetEditTracking(enabled); }

	void RemoveProperty(const QModelIndex &index)
	{ propModel->RemoveProperty(mapToSource(index)); }

	void RemovePropertyAll()
	{ propModel->RemovePropertyAll(); }

	void UpdateStructure(const QModelIndex &parent = QModelIndex())
	{ propModel->UpdateStructure(mapToSource(parent)); }

signals:
	void PropertyEdited(const QModelIndex &index);
	void PropertyChanged(const QModelIndex &index);

protected:
	QtPropertyModel *propModel;

	bool selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const;

protected slots:
	void OnPropertyEdited(const QModelIndex &index);
	void OnPropertyChanged(const QModelIndex &index);
	void OnRowsInserted(const QModelIndex &, int, int);
	void OnRowsRemoved(const QModelIndex &, int, int);
};

#endif // __QT_PROPERTY_MODEL_FILTERING_H__
