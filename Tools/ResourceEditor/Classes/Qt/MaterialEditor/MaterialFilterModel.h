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


#ifndef __MATERIALS_FILTER_MODEL_H__
#define __MATERIALS_FILTER_MODEL_H__

#include "Render/Material/NMaterial.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QString>

class QMimeData;
class QStandardItem;
class SceneEditor2;
class MaterialItem;
class Command2;
class EntityGroup;
class MaterialModel;

class MaterialFilteringModel
    : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum eFilterType
    {
        SHOW_ALL,
        SHOW_ONLY_INSTANCES,
        SHOW_INSTANCES_AND_MATERIALS,
        SHOW_NOTHING,
    };

public:
	MaterialFilteringModel(MaterialModel *treeModel, QObject *parent = NULL);

	void Sync();

	void SetScene(SceneEditor2 * scene);
    SceneEditor2 *GetScene();
	void SetSelection(const EntityGroup *group);
	DAVA::NMaterial * GetMaterial(const QModelIndex & index) const;
	QModelIndex GetIndex(DAVA::NMaterial *material, const QModelIndex &parent = QModelIndex()) const ;
	bool dropCanBeAccepted(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    void setFilterType( int type );
    int getFilterType() const;
	
    // QSortFilterProxyModel
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    // QStandardItemModel
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

private:
    int compareNames( DAVA::NMaterial *left, DAVA::NMaterial *right ) const;

	MaterialModel *materialModel;
    eFilterType filterType;
};


#endif // __MATERIALS_FILTER_MODEL_H__
