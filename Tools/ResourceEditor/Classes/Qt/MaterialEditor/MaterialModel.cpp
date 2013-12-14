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



#include "MaterialModel.h"
#include "MaterialItem.h"

#include "Scene/SceneEditor2.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"

MaterialModel::MaterialModel(QObject * parent)
    : QStandardItemModel(parent)
{ 
	QStringList headerLabels;
	headerLabels.append("Materials hierarchy");
	setHorizontalHeaderLabels(headerLabels);
}

MaterialModel::~MaterialModel()
{ }

void MaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());

	if(NULL != scene)
	{
		QStandardItem *root = invisibleRootItem();
		DAVA::MaterialSystem *matSys = scene->renderSystem->GetMaterialSystem();

		DAVA::Vector<DAVA::NMaterial *> materials;
		matSys->BuildMaterialList(NULL, materials);

		for(DAVA::uint32 i = 0; i < (DAVA::uint32)materials.size(); ++i)
		{
			if(materials[i]->IsSwitchable() && materials[i]->IsConfigMaterial())
			{
				MaterialItem *item = new MaterialItem(materials[i]);
				item->setDragEnabled(false);
				root->appendRow(item);
			}
		}
	}
}

DAVA::NMaterial * MaterialModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
	MaterialItem *item = (MaterialItem *) itemFromIndex(index);
    return item->GetMaterial();
}

QMimeData * MaterialModel::mimeData(const QModelIndexList & indexes) const
{
	if(indexes.size() > 0)
	{
        QVector<DAVA::NMaterial*> data;
        foreach(QModelIndex index, indexes)
        {
            data.push_back(GetMaterial(index));
        }
        
        return MimeDataHelper2<DAVA::NMaterial>::EncodeMimeData(data);
    }
    
	return NULL;
}

QStringList MaterialModel::mimeTypes() const
{
	QStringList types;
    
	types << MimeDataHelper2<DAVA::NMaterial>::GetSupportedTypeName();
    
	return types;
}

bool MaterialModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	return false;
}

MaterialFilteringModel::MaterialFilteringModel(MaterialModel *_materialModel, QObject *parent /*= NULL*/)
: QSortFilterProxyModel(parent)
, materialModel(_materialModel)
{ 
	setSourceModel(materialModel);
}

bool MaterialFilteringModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool ret = false;

	if(NULL != materialModel)
	{
		DAVA::NMaterial *materialLeft = materialModel->GetMaterial(left);
		DAVA::NMaterial *materialRight = materialModel->GetMaterial(right);

		if(materialLeft->IsConfigMaterial() && materialRight->IsConfigMaterial())
		{
			if( materialLeft->GetChildrenCount() > 0 && materialRight->GetChildrenCount() > 0 ||
				materialLeft->GetChildrenCount() == 0 && materialRight->GetChildrenCount() == 0)
			{
				ret = (strcmp(materialLeft->GetMaterialName().c_str(), materialRight->GetMaterialName().c_str()) < 0);
			}
			else if(materialLeft->GetChildrenCount() > 0 && materialRight->GetChildrenCount() == 0)
			{
				ret = true;
			}
		}
	}

	return ret;
}
