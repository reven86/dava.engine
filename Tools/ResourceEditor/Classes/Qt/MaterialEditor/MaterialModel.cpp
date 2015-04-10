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
#include "Scene/EntityGroup.h"

#include "Main/QtUtils.h"
#include "Tools/MimeData/MimeDataHelper2.h"
#include "Commands2/MaterialSwitchParentCommand.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Systems/MaterialSystem.h"

#include "TextureBrowser/TextureCache.h"
#include "TextureBrowser/TextureConvertor.h"
#include "TextureBrowser/TextureInfo.h"

#include "Settings/SettingsManager.h"

#include <QPainter>



MaterialModel::MaterialModel(QObject * parent)
    : QStandardItemModel(parent)
	, curScene(NULL)
{ 
    QStringList headerLabels;
    headerLabels.append("Materials hierarchy");
    headerLabels.append("L");
    headerLabels.append("S");
    setHorizontalHeaderLabels(headerLabels);
    horizontalHeaderItem(1)->setToolTip("Material LOD index");
    horizontalHeaderItem(2)->setToolTip("Material Switch index");

    setColumnCount(3);
}

MaterialModel::~MaterialModel()
{ }

QVariant MaterialModel::data(const QModelIndex & index, int role) const
{
    QVariant ret;

    if(index.column() == 0)
    {
        MaterialItem* item = itemFromIndex(index);
        DVASSERT(item);

        switch (role)
        {
        case Qt::BackgroundRole:
            {
                const bool toDel = item->GetFlag(MaterialItem::IS_MARK_FOR_DELETE);
                if (toDel)
                    ret = QBrush(QColor(255, 0, 0, 20));
            }
            break;
        case Qt::FontRole:
            {
                const bool isSelection = item->GetFlag(MaterialItem::IS_PART_OF_SELECTION);
                ret = QStandardItemModel::data(index, role);
                if (isSelection)
                {
                    QFont font = ret.value<QFont>();
                    font.setBold(true);
                    ret = font;
                }
            }
            break;
        default:
            ret = QStandardItemModel::data(index, role);
            break;
        }
    }
    // LOD
    else if(index.isValid() && index.column() < columnCount())
    {
        MaterialItem *item = itemFromIndex(index.sibling(index.row(), 0));
        if(NULL != item)
        {
            int lodIndex = item->GetLodIndex();
            int switchIndex = item->GetSwitchIndex();

            if(index.column() == 1)
            {
                switch(role)
                {
                    case Qt::DisplayRole:
                        {
                            if(-1 != lodIndex)
                            {
                                ret = lodIndex;
                            }
                        }
                        break;
                    
                    case Qt::TextAlignmentRole:
                        ret = (int) (Qt::AlignCenter | Qt::AlignVCenter);
                        break;

                    case Qt::BackgroundRole:
                        if(lodIndex >= 0 && lodIndex < supportedLodColorsCount)
                        {
                            ret = lodColors[lodIndex];
                        }
                        break;
                    default:
                        break;
                }
            }
            // Switch
            else if(index.column() == 2)
            {
                switch(role)
                {
                    case Qt::DisplayRole:
                        {
                            if(-1 != switchIndex)
                            {
                                ret = switchIndex;
                            }
                        }
                        break;

                    case Qt::TextAlignmentRole:
                        ret = (int) (Qt::AlignCenter | Qt::AlignVCenter);
                        break;

                    case Qt::BackgroundRole:
                        if(switchIndex >= 0 && switchIndex < supportedSwColorsCount)
                        {
                            ret = switchColors[switchIndex];
                        }
                        break;

                    default:
                        break;
                }
            }

        }
    }
    

    return ret;
}

MaterialItem* MaterialModel::itemFromIndex(const QModelIndex & index) const
{
    MaterialItem *ret = NULL;

    if(index.isValid())
    {
        ret = (MaterialItem *) QStandardItemModel::itemFromIndex(index.sibling(index.row(), 0));
    }

    return ret;
}

void MaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());
	curScene = scene;

    ReloadLodSwColors();

	if(NULL != scene)
	{
		Sync();
	}
}

SceneEditor2* MaterialModel::GetScene()
{
    return curScene;
}

void MaterialModel::SetSelection(const EntityGroup *group)
{
	QStandardItem *root = invisibleRootItem();
	for(int i = 0; i < root->rowCount(); ++i)
	{
		MaterialItem *topItem = (MaterialItem *) root->child(i);

        bool hasSelectedItems = false;
		for(int j = 0; j < topItem->rowCount(); ++j)
		{
			MaterialItem *childItem = (MaterialItem *) topItem->child(j);
            if ( SetItemSelection( childItem, group ) )
                hasSelectedItems = true;
		}
        topItem->SetFlag( MaterialItem::IS_PART_OF_SELECTION, hasSelectedItems );
	}
}

bool MaterialModel::SetItemSelection( MaterialItem *item, const EntityGroup *group )
{
    if ( group == NULL )
    {
        item->SetFlag( MaterialItem::IS_PART_OF_SELECTION, false );
        return false;
    }

	DAVA::NMaterial *material = item->GetMaterial();
	DAVA::Entity *entity = curScene->materialSystem->GetEntity(material);

	entity = curScene->selectionSystem->GetSelectableEntity(entity);
    const bool select = group->ContainsEntity(entity);
	item->SetFlag( MaterialItem::IS_PART_OF_SELECTION, select );
    
    return select;
}

void MaterialModel::Sync()
{
	if(NULL != curScene)
	{
		DAVA::Map<DAVA::NMaterial*, DAVA::Set<DAVA::NMaterial *> > materialsTree;
		curScene->materialSystem->BuildMaterialsTree(materialsTree);

        if(NULL != curScene->GetGlobalMaterial())
        {
            materialsTree[curScene->GetGlobalMaterial()];
        }

		// remove items, that are not in set
		QStandardItem *root = invisibleRootItem();
		for(int i = 0; i < root->rowCount(); ++i)
		{
			MaterialItem *item = (MaterialItem *) root->child(i);
			if(0 == materialsTree.count(item->GetMaterial()))
			{
				root->removeRow(i--);
			}
			else
			{
				// there is same material, so we should check it childs
				// and remove those that are not in tree

				const DAVA::Set<DAVA::NMaterial *> &childsList = materialsTree[item->GetMaterial()];

				for(int j = 0; j < item->rowCount(); ++j)
				{
					MaterialItem *child = (MaterialItem *) item->child(j);
					if(0 == childsList.count(child->GetMaterial()))
					{
						item->removeRow(j--);
					}
				}
			}
		}

		// add items, that are not added yet
		auto it = materialsTree.begin();
		auto end = materialsTree.end();
		for(; it != end; ++it)
		{
			DAVA::NMaterial *toAdd = it->first;
			QModelIndex index = GetIndex(toAdd);
            MaterialItem *item = NULL;

			// still no such material in model?
			if(!index.isValid())
			{
				item = new MaterialItem(toAdd);

				// and it childs
				auto cit = it->second.begin();
				auto cend = it->second.end();
				for(; cit != cend; ++cit)
				{
					item->appendRow(new MaterialItem(*cit));
				}

				// add created item
				root->appendRow(item);
			}
			else
			{
				// there is already such material in model
				// we should sync it childs

				item = itemFromIndex(index);
				auto cit = it->second.begin();
				auto cend = it->second.end();

				for(; cit != cend; ++cit)
				{
					DAVA::NMaterial *childMaterial = *cit;

					// no such item?
					if(!GetIndex(childMaterial, index).isValid())
					{
						item->appendRow(new MaterialItem(childMaterial));
					}
				}
			}
		}

		// mark materials that can be deleted
        // setup lod/switch indexes
		for(int i = 0; i < root->rowCount(); ++i)
		{
			MaterialItem *item = (MaterialItem *) root->child(i);
            const bool toDel = (item->rowCount() == 0);
			item->SetFlag(MaterialItem::IS_MARK_FOR_DELETE, toDel);

            for(int j = 0; j < item->rowCount(); ++j)
            {
                MaterialItem *instanceItem = (MaterialItem *) item->child(j);
                
                const DAVA::RenderBatch *rb = curScene->materialSystem->GetRenderBatch(instanceItem->GetMaterial());
                
                if(rb)
                {
                    const DAVA::RenderObject *ro = rb->GetRenderObject();
                    
                    for(DAVA::uint32 k = 0; k < ro->GetRenderBatchCount(); ++k)
                    {
                        int lodIndex, swIndex;
                        DAVA::RenderBatch *batch = ro->GetRenderBatch(k, lodIndex, swIndex);
                        if(rb == batch)
                        {
                            instanceItem->SetLodIndex(lodIndex);
                            instanceItem->SetSwitchIndex(swIndex);
                            break;
                        }
                    }
                }
            }
		}

        const EntityGroup& selection = curScene->selectionSystem->GetSelection();
        SetSelection( &selection );
	}
}

DAVA::NMaterial * MaterialModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
	MaterialItem *item = itemFromIndex(index);
    return item->GetMaterial();
}

QModelIndex MaterialModel::GetIndex(DAVA::NMaterial *material, const QModelIndex &parent) const
{
	QModelIndex ret = QModelIndex();

	MaterialItem* item = itemFromIndex(parent);
	if(NULL != item && item->GetMaterial() == material)
	{
		ret = parent;
	}
	else
	{
		QStandardItem *sItem = (NULL != item) ? item : invisibleRootItem();
		if(NULL != sItem)
		{
			for(int i = 0; i < sItem->rowCount(); ++i)
			{
				ret = GetIndex(material, index(i, 0, parent));
				if(ret.isValid())
				{
					break;
				}
			}
		}
	}

	return ret;
}

QMimeData * MaterialModel::mimeData(const QModelIndexList & indexes) const
{
	if (indexes.size() > 0)
	{
        QVector<DAVA::NMaterial*> data;
        foreach(QModelIndex index, indexes)
        {
            if(0 == index.column())
            {
                DAVA::NMaterial *material = GetMaterial(index);
                data.push_back(material);
            }
        }
        
        return MimeDataHelper2<DAVA::NMaterial>::EncodeMimeData(data);
    }
    
	return NULL;
}

QStringList MaterialModel::mimeTypes() const
{
	QStringList types;
	types << MimeDataHelper2<DAVA::NMaterial>::GetMimeType();
    
	return types;
}

bool MaterialModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QModelIndex targetIndex = parent;
    if ( !targetIndex.isValid() )
        return false;

	QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);
	if ( materials.size() <= 0 )
        return false;

    if ( dropCanBeAccepted(data, action, targetIndex.row(), targetIndex.column(), targetIndex.parent()) )
    {
		MaterialItem *targetMaterialItem = itemFromIndex(targetIndex);
		DAVA::NMaterial *targetMaterial = targetMaterialItem->GetMaterial();

		if ( materials.size() > 1 )
		{
			curScene->BeginBatch("Change materials parent");
		}

		// change parent material
		// NOTE: model synchronization will be done in OnCommandExecuted handler
		for(int i = 0; i < materials.size(); ++i)
		{
			MaterialItem *sourceMaterialItem = itemFromIndex(GetIndex(materials[i]));
			if (NULL != sourceMaterialItem)
			{
				curScene->Exec(new MaterialSwitchParentCommand(materials[i], targetMaterial));
			}
		}

		if ( materials.size() > 1 )
        {
			curScene->EndBatch();
        }
    }

	return true;
}

bool MaterialModel::dropCanBeAccepted(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if(0 != column)
        return false;

	const QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);

    if ( materials.size() <= 0 )
        return false;

    QModelIndex targetIndex = index(row, column, parent);
    if ( !targetIndex.isValid() )
        return false;

    DAVA::NMaterial *targetMaterial = GetMaterial(targetIndex);
    if ( targetMaterial == NULL )
        return false;

    if( targetMaterial->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_MATERIAL )
        return false;

	for( int i = 0; i < materials.size(); i++ )
	{
		if ( materials[i]->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_INSTANCE )
            return false;
	}

    return true;
}

void MaterialModel::ReloadLodSwColors()
{
    QString key;

    for(int i = 0; i < supportedLodColorsCount; ++i)
    {
        key.sprintf("General/MaterialEditor/LodColor%d", i);

        DAVA::VariantType val = SettingsManager::GetValue(key.toStdString());
        if(val.type == DAVA::VariantType::TYPE_COLOR)
        {
            lodColors[i] = ColorToQColor(val.AsColor());
        }
        else
        {
            lodColors[i] = QColor();
        }
    }

    for(int i = 0; i < supportedSwColorsCount; ++i)
    {
        key.sprintf("General/MaterialEditor/SwitchColor%d", i);

        DAVA::VariantType val = SettingsManager::GetValue(key.toStdString());
        if(val.type == DAVA::VariantType::TYPE_COLOR)
        {
            switchColors[i] = ColorToQColor(val.AsColor());
        }
        else
        {
            switchColors[i] = QColor();
        }
    }
}
