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



#ifndef __QT_PROPERTY_MODEL_H__
#define __QT_PROPERTY_MODEL_H__

#include <QPair>
#include <QStandardItemModel>
#include "Base/Introspection.h"

#include "QtPropertyItem.h"
#include "QtPropertyData.h"

class QtPropertyModel : public QStandardItemModel
{
	Q_OBJECT

	friend class QtPropertyItem;

public:
	QtPropertyModel(QObject* parent = 0);
	~QtPropertyModel();

	QPair<QtPropertyItem*, QtPropertyItem*> AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent = NULL);
	QPair<QtPropertyItem*, QtPropertyItem*> GetProperty(const QString &name, QtPropertyItem* parent = NULL);

	bool GetEditTracking();
	void SetEditTracking(bool enabled);

	void RemoveProperty(QtPropertyItem* item);
	void RemovePropertyAll();

	void UpdateStructure(const QModelIndex &parent = QModelIndex());

signals:
	void ItemEdited(const QString &name, QtPropertyData *data);
	void ItemChanged(const QString &name, QtPropertyData *data);

protected:
	bool trackEdit;

	void EmitDataEdited(QtPropertyItem *item);
	void UpdateStructureInternal(const QModelIndex &i);
};

#endif // __QT_PROPERTY_MODEL_H__
