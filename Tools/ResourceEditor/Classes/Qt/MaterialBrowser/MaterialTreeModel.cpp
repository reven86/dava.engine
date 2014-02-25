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



#include <QtGui>
#include "MaterialBrowser/MaterialTreeModel.h"

#include "../Qt/CubemapEditor/MaterialHelper.h"

MaterialTreeModel::MaterialTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
	, scene(NULL)
	, rootFiltredMaterial(NULL)
{ }

MaterialTreeModel::~MaterialTreeModel()
{
}

void MaterialTreeModel::SetScene(DAVA::Scene *scene)
{
	this->scene = scene;
	SearchMaterialsInScene();
}

MaterialTreeItem* MaterialTreeModel::Item(const QModelIndex &index) const
{
	MaterialTreeItem *item = NULL;

	if(index.isValid())
	{
		item = (MaterialTreeItem*) index.internalPointer();
	}

	return item;
}

int MaterialTreeModel::columnCount(const QModelIndex &parent) const
{
	int count = 0;

	if(parent.isValid())
	{
		MaterialTreeItem* item = (MaterialTreeItem*) parent.internalPointer();
		count = item->ColumnCount();
	}
	else if(NULL != rootFiltredMaterial)
	{
		count = rootFiltredMaterial->ColumnCount();
	}

	return count;
}

int MaterialTreeModel::rowCount(const QModelIndex &parent) const
{
	int count = 0;

	if(parent.isValid())
	{
		MaterialTreeItem* item = (MaterialTreeItem*) parent.internalPointer();
		count = item->ChildCount();
	}
	else if(NULL != rootFiltredMaterial)
	{
		count = rootFiltredMaterial->ChildCount();
	}

	return count;
}


QVariant MaterialTreeModel::data(const QModelIndex &index, int role) const
{
	QVariant v;

	if (index.isValid())
	{
		MaterialTreeItem* item = (MaterialTreeItem*) index.internalPointer();

		switch(role)
		{
		case Qt::DisplayRole:
			v = item->Data(index.column());
			break;

		default:
			break;
		}
	}

	return v;
}

QModelIndex MaterialTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	QModelIndex index;

	if (hasIndex(row, column, parent))
	{
		MaterialTreeItem* item;

		if (parent.isValid())
		{
			item = (MaterialTreeItem *) parent.internalPointer();
		}
		else
		{
			item = rootFiltredMaterial;
		}

		if(NULL != item)
		{
			MaterialTreeItem *childItem = item->Child(row);
			if(NULL != childItem)
			{
				index = createIndex(row, column, childItem);
			}
		}
	}

	return index;
}

QModelIndex MaterialTreeModel::parent(const QModelIndex &index) const
{
	QModelIndex parentIndex;

	if (index.isValid())
	{
		MaterialTreeItem *item = (MaterialTreeItem *) index.internalPointer();
		if(NULL != item && item != rootFiltredMaterial)
		{
			MaterialTreeItem *parent = item->Parent();

			if(NULL != parent)
			{
				parentIndex = createIndex(parent->Row(), 0, parent);
			}
		}
	}

	return parentIndex;
}

void MaterialTreeModel::SearchMaterialsInScene()
{
	if(NULL != scene)
	{
		allMaterials.clear();
		scene->GetDataNodes(allMaterials);		
		//VI: remove skybox materials so they not to appear in the lists
		DAVA::MaterialHelper::FilterMaterialsByType(allMaterials, DAVA::Material::MATERIAL_SKYBOX);

		ApplyFilterAndSort();
	}
}

void MaterialTreeModel::SearchMaterialsInFolder()
{

}

void MaterialTreeModel::ApplyFilterAndSort()
{
	DAVA::SafeDelete(rootFiltredMaterial);

	rootFiltredMaterial = new MaterialTreeItem("Root");
	MaterialTreeItem *rootFolder = new MaterialTreeItem("Materials", rootFiltredMaterial);

	// No actual filtering. Plain tree
	for(int i = 0; i < (int) allMaterials.size(); ++i)
	{
		DAVA::Material *material = allMaterials[i];

		if(NULL != material)
		{
			rootFolder->AddChild(new MaterialTreeItem(material));
		}
	}

	emit dataChanged(index(0, 0), index(rootFiltredMaterial->ChildCount(), rootFiltredMaterial->ColumnCount()));
}


