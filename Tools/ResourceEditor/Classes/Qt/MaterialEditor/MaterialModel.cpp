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

DAVA::NMaterial* MaterialModel::GetGlobalMaterial() const
{
    DAVA::NMaterial *ret = nullptr;
    if (nullptr != curScene)
    {
        ret = curScene->GetGlobalMaterial();
    }

    return ret;
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
        DAVA::NMaterial *globalMaterial = GetGlobalMaterial();
        const DAVA::Set<DAVA::NMaterial*> &sceneMaterials = curScene->materialSystem->GetTopParents();
        Map<NMaterial *, bool> processedList;
        
        // init processed list
        for (auto it : sceneMaterials)
        {
            processedList[it] = false;
        }

        // add global material into list
        if (nullptr != globalMaterial)
        {
            processedList[globalMaterial] = false;
        }

        // remove items, that are not in set
        QStandardItem *root = invisibleRootItem();
        for(int i = 0; i < root->rowCount(); ++i)
        {
            MaterialItem *item = (MaterialItem *) root->child(i);
            DAVA::NMaterial *material = item->GetMaterial();

            // no such material in scene - remove it from tree
            if (0 == processedList.count(material))
            {
                root->removeRow(i--);
            }
            else
            {
                // sync material with material item
                if (material != globalMaterial)
                    Sync(item);

                // mark processed material 
                processedList[material] = true;
            }
        }

        // add items, that are not added yet
        // that are thous material that are not market as processed
        for (auto it : processedList)
        {
            if (!it.second)
            {
                bool dragEnabled = true;
                bool dropEnabled = true;

                if (it.first == globalMaterial)
                {
                    dragEnabled = false;
                    dropEnabled = false;
                }

                MaterialItem *newItem = new MaterialItem(it.first, dragEnabled, dropEnabled);
                root->appendRow(newItem);

                if (it.first != globalMaterial)
                    Sync(newItem);
            }
        }

        const EntityGroup& selection = curScene->selectionSystem->GetSelection();
        SetSelection( &selection );
    }
}

void MaterialModel::Sync(MaterialItem *item)
{
    DAVA::NMaterial* material = item->GetMaterial();
    const Vector<NMaterial *>& materialChildren = material->GetChildren();

    Map<NMaterial *, bool> processedList;

    // init processed list
    for (auto it : materialChildren)
    {
        processedList[it] = false;
    }

    // remove all items that are not in hierarchy
    for (int i = 0; i < item->rowCount(); ++i)
    {
        MaterialItem *childItem = (MaterialItem *)item->child(i);
        DAVA::NMaterial* childMaterial = childItem->GetMaterial();
        if (0 != processedList.count(childMaterial))
        {
            item->removeRow(i--);
        }
        else
        {
            processedList[childMaterial] = true;
            Sync(childItem);
        }
    }

    // add materials that are in hierarchy but not in model yet
    for (auto it : processedList)
    {
        if (!it.second)
        {
            MaterialItem *newItem = new MaterialItem(it.first, true, true);
            item->appendRow(newItem);
            Sync(newItem);
        }
    }

    bool can_be_deleted = (0 == item->rowCount());

    // set item lod/switch flags
    const DAVA::RenderBatch *rb = curScene->materialSystem->GetRenderBatch(material);
    if (nullptr != rb)
    {
        const DAVA::RenderObject *ro = rb->GetRenderObject();
        for (DAVA::uint32 k = 0; k < ro->GetRenderBatchCount(); ++k)
        {
            int lodIndex, swIndex;
            DAVA::RenderBatch *batch = ro->GetRenderBatch(k, lodIndex, swIndex);
            if (rb == batch)
            {
                item->SetLodIndex(lodIndex);
                item->SetSwitchIndex(swIndex);
                break;
            }
        }

        can_be_deleted = false;
    }

    // set 'unused' material mark 
    item->SetFlag(MaterialItem::IS_MARK_FOR_DELETE, can_be_deleted);
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

    if( targetMaterial == curScene->GetGlobalMaterial())
        return false;

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
