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



#include <QMimeData>
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"

#include "DockSceneTree/SceneTreeModel.h"
#include "Scene/SceneSignals.h"

#include "MaterialEditor/MaterialAssignSystem.h"


// framework
#include "Scene3d/Components/ComponentHelpers.h"

// commands
#include "Commands2/ParticleEditorCommands.h"

//mime data
#include "Tools/MimeData/MimeDataHelper2.h"

SceneTreeModel::SceneTreeModel(QObject* parent /*= 0*/ )
	: QStandardItemModel(parent)
	, curScene(NULL)
	, dropAccepted(false)
{
	setColumnCount(1);
	setSupportedDragActions(Qt::MoveAction|Qt::LinkAction);

	QStringList headerLabels;
	headerLabels.append("Scene hierarchy");
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}

SceneTreeModel::~SceneTreeModel()
{
	if(NULL != curScene)
	{
		curScene->Release();
		curScene = NULL;
	}
}

void SceneTreeModel::SetScene(SceneEditor2 *scene)
{
	// remove add rows
	removeRows(0, rowCount());

	if(NULL != curScene)
	{
		curScene->Release();
	}	

	curScene = scene;

	if(NULL != curScene)
	{
		curScene->Retain();
	}

	ResyncStructure(invisibleRootItem(), curScene);
}

SceneEditor2* SceneTreeModel::GetScene() const
{
	return curScene;
}

void SceneTreeModel::SetSolid(const QModelIndex &index, bool solid)
{
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));

	if(NULL != entity)
	{
		entity->SetSolid(solid);
	}
}

bool SceneTreeModel::GetSolid(const QModelIndex &index) const
{
	bool ret = false;

	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));
	if(NULL != entity)
	{
		ret = entity->GetSolid();
	}

	return ret;
}

void SceneTreeModel::SetLocked(const QModelIndex &index, bool locked)
{
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));

	if(NULL != entity)
	{
		entity->SetLocked(locked);
	}
}

bool SceneTreeModel::GetLocked(const QModelIndex &index) const
{
	bool ret = false;
	
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));
	if(NULL != entity)
	{
		ret = entity->GetLocked();
	}

	return ret;
}

QVector<QIcon> SceneTreeModel::GetCustomIcons(const QModelIndex &index) const
{
	static QIcon lockedIcon = QIcon(":/QtIcons/locked.png");
	static QIcon eyeIcon = QIcon(":/QtIcons/eye.png");

	QVector<QIcon> ret;
	SceneTreeItem *item = GetItem(index);

	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);
	if(NULL != entity)
	{
		if(entity->GetLocked())
		{
			ret.push_back(lockedIcon);
		}

		if(NULL != GetCamera(entity))
		{
			if(curScene->GetCurrentCamera() == GetCamera(entity))
			{
				ret.push_back(eyeIcon);
			}
		}
	}

	return ret;
}

int SceneTreeModel::GetCustomFlags(const QModelIndex &index) const
{
	int ret = 0;

	SceneTreeItem *item = GetItem(index);
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);

	if(NULL != entity)
	{
		if( entity->GetName().find("editor.") != DAVA::String::npos ||
			NULL != DAVA::GetLandscape(entity) ||
			NULL != DAVA::GetSkybox(entity))
		{
			ret |= CF_Disabled;
		}
	}
	else
	{
		DAVA::ParticleLayer *layer = SceneTreeItemParticleLayer::GetLayer(item);
		if(NULL != layer)
		{
			DAVA::Entity *emitter = SceneTreeItemEntity::GetEntity(GetItem(index.parent()));
			if(NULL != emitter)
			{
				DAVA::LodComponent *lodComp = GetLodComponent(emitter);
				if(NULL != lodComp)
				{
					if(!layer->IsLodActive(lodComp->currentLod))
					{
						ret |= CF_Invisible;
					}
				}
			}
		}
	}

	return ret;
}

QModelIndex SceneTreeModel::GetIndex(DAVA::Entity *entity) const
{
	return indexesCacheEntities.value(entity, QModelIndex());
}

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleLayer *layer) const
{
	return indexesCacheLayers.value(layer, QModelIndex());
}

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleForce *force) const
{
	return indexesCacheForces.value(force, QModelIndex());
}

SceneTreeItem* SceneTreeModel::GetItem(const QModelIndex &index) const
{
	return (SceneTreeItem *) itemFromIndex(index);
}

Qt::DropActions SceneTreeModel::supportedDropActions() const
{
	return (Qt::LinkAction | Qt::MoveAction | Qt::CopyAction);
}

QMimeData * SceneTreeModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData* ret = NULL;

	if(indexes.size() > 0)
	{
		if(AreSameType(indexes))
		{
			SceneTreeItem *firstItem = GetItem(indexes.at(0));
			if(NULL != firstItem)
			{
				switch (firstItem->ItemType())
				{
				case SceneTreeItem::EIT_Entity:
					{
						QVector<DAVA::Entity*> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemEntity::GetEntity(GetItem(index)));
						}

						ret = MimeDataHelper2<DAVA::Entity>::EncodeMimeData(data);
					}
					break;
				case SceneTreeItem::EIT_Layer:
					{
						QVector<DAVA::ParticleLayer *> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemParticleLayer::GetLayer(GetItem(index)));
						}

						ret = MimeDataHelper2<DAVA::ParticleLayer>::EncodeMimeData(data);
					}
					break;
				case SceneTreeItem::EIT_Force:
					{
						QVector<DAVA::ParticleForce *> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemParticleForce::GetForce(GetItem(index)));
						}

						ret = MimeDataHelper2<DAVA::ParticleForce>::EncodeMimeData(data);
					}
					break;
				default:
					break;
				}
			}
		}
		else
		{
			// empty mime data
			ret = new QMimeData();
		}
	}

	return ret;
}

QStringList SceneTreeModel::mimeTypes() const
{
	QStringList types;

	types << MimeDataHelper2<DAVA::Entity>::GetSupportedTypeName();
	types << MimeDataHelper2<DAVA::ParticleLayer>::GetSupportedTypeName();
	types << MimeDataHelper2<DAVA::ParticleForce>::GetSupportedTypeName();

	return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	const QtMimeData *mimeData = dynamic_cast<const QtMimeData *>(data);
	if(!mimeData)
	{
		Logger::Warning("[SceneTreeModel::dropMimeData] dropped data isn't QtMimeData");
		return false;
	}

	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);
	SceneTreeItem *beforeItem = GetItem(index(row, column, parent));

	int dropType = GetDropType(mimeData);
	switch (dropType)
	{
	case DropingEntity:
		{
			DAVA::Entity *parentEntity = SceneTreeItemEntity::GetEntity(parentItem);
			DAVA::Entity *beforeEntity = SceneTreeItemEntity::GetEntity(beforeItem);

			if(NULL == parentEntity)
			{
				parentEntity = curScene;
			}

			QVector<DAVA::Entity*> *entitiesV = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(mimeData);
			if(NULL != entitiesV && entitiesV->size() > 0)
			{
				EntityGroup entityGroup;
				for (int i = 0; i < entitiesV->size(); ++i)
				{
					entityGroup.Add((DAVA::Entity*) entitiesV->at(i));
				}

				curScene->structureSystem->Move(entityGroup, parentEntity, beforeEntity);
				ret = true;
			}

			if(NULL != entitiesV)
			{
				delete entitiesV;
			}
		}
		break;
	case DropingLayer:
		{
			
			DAVA::ParticleEmitter* emitter = NULL;
			if (parentItem->ItemType() == SceneTreeItem::EIT_Entity)
			{
				DAVA::Entity *parentEntity = SceneTreeItemEntity::GetEntity(parentItem);
				emitter = DAVA::GetEmitter(parentEntity);
			}
			else if (parentItem->ItemType() == SceneTreeItem::EIT_InnerEmmiter)
			{
				emitter = ((SceneTreeItemParticleInnerEmmiter* )parentItem)->emitter;
			}


			QVector<DAVA::ParticleLayer *> *layersV = MimeDataHelper2<DAVA::ParticleLayer>::DecodeMimeData(mimeData);

			if(NULL != emitter && NULL != layersV && layersV->size() > 0)
			{
				DAVA::ParticleLayer* beforeLayer = NULL;

				if(row >= 0 && row < (int) emitter->GetLayers().size())
				{
					beforeLayer = emitter->GetLayers()[row];
				}

				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
				for(int i = 0; i < layersV->size(); ++i)
				{
					layersGroup.push_back((DAVA::ParticleLayer *) layersV->at(i));
				}

				curScene->structureSystem->MoveLayer(layersGroup, emitter, beforeLayer);
				ret = true;
			}

			if(NULL != layersV)
			{
				delete layersV;
			}
		}
		break;
	case DropingForce:
		{
			DAVA::ParticleLayer *newLayer = SceneTreeItemParticleLayer::GetLayer(parentItem);
			QVector<DAVA::ParticleForce*> *forcesV = MimeDataHelper2<DAVA::ParticleForce>::DecodeMimeData(mimeData);

			if(NULL != newLayer && NULL != forcesV && forcesV->size() > 0)
			{
				DAVA::Vector<DAVA::ParticleForce*> forcesGroup;
				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;

				for(int i = 0; i < forcesV->size(); ++i)
				{
					QModelIndex forceIndex = GetIndex((DAVA::ParticleForce *) forcesV->at(i));
					DAVA::ParticleLayer *oldLayer = SceneTreeItemParticleLayer::GetLayer(GetItem(forceIndex.parent()));

					forcesGroup.push_back((DAVA::ParticleForce *) forcesV->at(i));
					layersGroup.push_back(oldLayer);
				}

				curScene->structureSystem->MoveForce(forcesGroup, layersGroup, newLayer);
				ret = true;
			}

			if(NULL != forcesV)
			{
				delete forcesV;
			}
		}
		break;
            
    case DropingMaterial:
        {
            DropMaterial(parentItem, mimeData);
            break;
        }

	default:
		break;
	}

	dropAccepted = ret;
	return ret;
}

bool SceneTreeModel::DropCanBeAccepted(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
	const QtMimeData *mimeData = dynamic_cast<const QtMimeData *>(data);
	if(!mimeData)
	{
		Logger::Warning("[SceneTreeModel::DropCanBeAccepted] dropped data isn't QtMimeData");
		return false;
	}

	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);

	int dropType = GetDropType(mimeData);
	switch (dropType)
	{
	case DropingEntity:
		{
			ret = true;

			// don't accept entity to be dropped anywhere except other entity
			if(NULL != parentItem && parentItem->ItemType() != SceneTreeItem::EIT_Entity)
			{
				ret = false;
			}
			// don't accept drops inside disabled (by custom flags) entity 
			else if(GetCustomFlags(parent) & CF_Disabled)
			{
				ret = false;
			}
			else
			{
				// go thought entities and check them
				QVector<DAVA::Entity *> *entities = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(mimeData);
				if(NULL != entities)
				{
					DAVA::Entity *targetEntity = SceneTreeItemEntity::GetEntity(parentItem);

					for (int i = 0; i < entities->size(); ++i)
					{
						DAVA::Entity *entity = (DAVA::Entity *) entities->at(i);
						QModelIndex entityIndex = GetIndex(entity);

						// 1. we don't accept drops if it has locked items
						if(NULL != entity && entity->GetLocked()) 
						{
							ret = false;
							break;
						}

						// 2. or it has entity with CF_Disabled flag
						if(GetCustomFlags(entityIndex) & CF_Disabled)
						{
							ret = false;
							break;
						}

						// 3. or this is self-drop
						if( targetEntity == entity || // dropping into
							entityIndex == index(row, column, parent) || // dropping above
							entityIndex == index(row - 1, column, parent)) // dropping below
						{
							ret = false;
							break;
						}

						// 4. or we are dropping last element to the bottom of the list
						if( NULL == targetEntity && row == -1 && // dropping to be bottom of the list
							!entityIndex.parent().isValid() && // no parent
							entityIndex.row() == (rowCount(entityIndex.parent()) - 1)) // dropped item is already bottom
						{
							ret = false;
							break;
						}
					}

					delete entities;
				}
			}
		}
		break;
	case DropingLayer:
		{
			// accept layer to be dropped only to entity with particle emitter
			if(NULL != parentItem) 
			{
				if (parentItem->ItemType() == SceneTreeItem::EIT_Entity)
				{
					DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(parentItem);
					if(NULL != DAVA::GetEmitter(entity))
					{
						ret = true;
					}
				}
				else if (parentItem->ItemType() == SceneTreeItem::EIT_InnerEmmiter)
				{
					ret = true;
				}
			}

		}
		break;
	case DropingForce:
		{
			// accept force to be dropped only to particle layer
			if(NULL != parentItem && parentItem->ItemType() == SceneTreeItem::EIT_Layer) 
			{
				// accept only add (no insertion)
				if(-1 == row && -1 == column)
				{
					ret = true;
				}
			}
		}
		break;
            
        case DropingMaterial:
        {
            DAVA::Entity *targetEntity = SceneTreeItemEntity::GetEntity(parentItem);
            if(targetEntity)
            {
                MaterialAssignSystem::DropTestResult result = MaterialAssignSystem::TestEntity(targetEntity, true);
                ret = (result.hasEntitiesAvailableToDrop || result.hasEntityUnavailableToDrop);
            }
            break;
        }


	default:
		break;
	}

	return ret;
}

bool SceneTreeModel::DropAccepted() const
{
	return dropAccepted;
}

void SceneTreeModel::ItemChanged(QStandardItem * item)
{
	SceneTreeItem *treeItem = dynamic_cast<SceneTreeItem *>(item);
	if(NULL != treeItem)
	{
		if(treeItem->ItemType() == SceneTreeItem::EIT_Layer)
		{
			bool isLayerEnabled = (item->checkState() == Qt::Checked);
			SceneTreeItemParticleLayer *itemLayer = (SceneTreeItemParticleLayer *) treeItem;

			CommandUpdateParticleLayerEnabled* command = new CommandUpdateParticleLayerEnabled(itemLayer->layer, isLayerEnabled);
			curScene->Exec(command);
		}
	}
}

void SceneTreeModel::ResyncStructure(QStandardItem *item, DAVA::Entity *entity)
{
	SceneTreeItemEntity::DoSync(item, entity);
	RebuildIndexesCache();
}

void SceneTreeModel::RebuildIndexesCache()
{
	indexesCacheEntities.clear();
	indexesCacheForces.clear();
	indexesCacheForces.clear();

	for(int i = 0; i < rowCount(); ++i)
	{
		AddIndexesCache(GetItem(index(i, 0)));
	}
}

void SceneTreeModel::AddIndexesCache(SceneTreeItem *item)
{
	// go thought all items and remember entities indexes
	switch(item->ItemType())
	{
	case SceneTreeItem::EIT_Entity:
		{
			DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);
			if(NULL != entity)
			{
				indexesCacheEntities.insert(entity, item->index());
			}
		}
		break;
	case SceneTreeItem::EIT_Layer:
		{
			DAVA::ParticleLayer *layer = SceneTreeItemParticleLayer::GetLayer(item);
			if(NULL != layer)
			{
				indexesCacheLayers.insert(layer, item->index());
			}
		}
		break;
	case SceneTreeItem::EIT_Force:
		{
			DAVA::ParticleForce *force = SceneTreeItemParticleForce::GetForce(item);
			if(NULL != force)
			{
				indexesCacheForces.insert(force, item->index());
			}
		}
		break;
	}

	for(int i = 0; i < item->rowCount(); ++i)
	{
		AddIndexesCache((SceneTreeItem *) item->child(i));
	}
}

void SceneTreeModel::ResetFilterAcceptFlag()
{
	for(int i = 0; i < invisibleRootItem()->rowCount(); ++i)
	{
		ResetFilterAcceptFlagInternal(GetItem(index(i, 0)));
	}
}

void SceneTreeModel::ResetFilterAcceptFlagInternal(SceneTreeItem *item)
{
	if(NULL != item)
	{
		item->SetAcceptedByFilter(false);
		for(int i = 0; i < item->rowCount(); ++i)
		{
			ResetFilterAcceptFlagInternal((SceneTreeItem *) item->child(i));
		}
	}
}

bool SceneTreeModel::AreSameType(const QModelIndexList & indexes) const
{
	bool ret = true;

	SceneTreeItem *firstItem = GetItem(indexes.at(0));

	// check if all items are same type
	if(NULL != firstItem)
	{
		foreach(QModelIndex index, indexes)
		{
			SceneTreeItem *item = GetItem(index);
			if(NULL != item && firstItem->ItemType() != item->ItemType())
			{
				ret = false;
				break;
			}
		}
	}

	return ret;
}

int SceneTreeModel::GetDropType(const QtMimeData *data) const
{
	int ret = DropingUnknown;

	if(NULL != data)
	{
        if(MimeDataHelper2<DAVA::Entity>::IsDataSupportType(data))
        {
			ret = DropingEntity;
        }
        else if(MimeDataHelper2<DAVA::ParticleLayer>::IsDataSupportType(data))
        {
			ret = DropingLayer;
        }
        else if(MimeDataHelper2<DAVA::ParticleForce>::IsDataSupportType(data))
        {
			ret = DropingForce;
        }
        else if(MimeDataHelper2<DAVA::NMaterial>::IsDataSupportType(data))
        {
			ret = DropingMaterial;
        }
	}

	return ret;
}

void SceneTreeModel::DropMaterial(SceneTreeItem *parentItem, const QtMimeData *mimeData) const
{
    DAVA::Entity *targetEntity = SceneTreeItemEntity::GetEntity(parentItem);
    QVector<DAVA::NMaterial*> *materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(mimeData);
    if(curScene && targetEntity && (materials->size() == 1))
    {
        MaterialAssignSystem::AssignMaterialToEntity(curScene, targetEntity, materials->at(0));
    }
}



SceneTreeFilteringModel::SceneTreeFilteringModel(SceneTreeModel *_treeModel, QObject *parent /* = NULL */)
	: QSortFilterProxyModel(parent)
	, treeModel(_treeModel)
{
	setSourceModel(treeModel);
}

bool SceneTreeFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(NULL != treeModel)
	{
		// check self accept
		if(selfAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}

		//accept if any of the parents is accepted
		QModelIndex parent = sourceParent;
		while(parent.isValid()) 
		{
			if(selfAcceptRow(parent.row(), parent.parent()))
			{
				return true;
			}

			parent = parent.parent();
		}

		// accept if any child is accepted
		if(childrenAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}
	}

	return false;
}

bool SceneTreeFilteringModel::selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool accepted = QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
	SceneTreeItem *item = treeModel->GetItem(treeModel->index(sourceRow, 0, sourceParent));

	if(NULL != item)
	{
		if(accepted && !filterRegExp().isEmpty())
		{
			item->SetAcceptedByFilter(accepted);
		}
		else
		{
			item->SetAcceptedByFilter(false);
		}
	}

	return accepted;
}

bool SceneTreeFilteringModel::childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool ret = false;

	QModelIndex index = treeModel->index(sourceRow, 0, sourceParent);
	if(treeModel->rowCount(index) > 0)
	{
		for(int i = 0; i < treeModel->rowCount(index); i++)
		{
			if(selfAcceptRow(i, index) || childrenAcceptRow(i, index))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}
