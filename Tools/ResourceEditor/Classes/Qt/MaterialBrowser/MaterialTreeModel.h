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



#ifndef __MATERIAL_TREE_MODEL_H__
#define __MATERIAL_TREE_MODEL_H__

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "DAVAEngine.h"
#include "MaterialBrowser/MaterialTreeItem.h"


class MaterialTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	MaterialTreeModel(QObject *parent = 0);
	~MaterialTreeModel();

	void SetScene(DAVA::Scene *scene);
	MaterialTreeItem* Item(const QModelIndex &index) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
	DAVA::Scene *scene;
	MaterialTreeItem *rootFiltredMaterial;

	DAVA::Vector<DAVA::Material *> allMaterials;

	void SearchMaterialsInScene();
	void SearchMaterialsInFolder();
	void ApplyFilterAndSort();
};

#endif // __MATERIAL_TREE_MODEL_H__
