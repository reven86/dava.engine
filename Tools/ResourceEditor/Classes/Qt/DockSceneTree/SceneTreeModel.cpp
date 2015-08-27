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
    SetScene(NULL);
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

Qt::DropActions SceneTreeModel::supportedDragActions() const
{
	return Qt::MoveAction|Qt::LinkAction;
}

void SceneTreeModel::SetScene(SceneEditor2 *scene)
{
    clear();
    setColumnCount(1);
    setHorizontalHeaderLabels( QStringList() << "Scene hierarchy" );

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
    SetFilter(filterText);  // Apply filter to new model
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
		SceneSignals::Instance()->EmitSolidChanged(curScene, entity, solid);
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
		if(entity->GetName().find("editor.") != DAVA::String::npos)
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

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleEmitter *emitter) const
{
	return indexesCacheEmitters.value(emitter, QModelIndex());
}

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleForce *force) const
{
	return indexesCacheForces.value(force, QModelIndex());
}

SceneTreeItem* SceneTreeModel::GetItem(const QModelIndex &index) const
{
	return (SceneTreeItem *)itemFromIndex(index);
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
                case SceneTreeItem::EIT_Emitter:
                    {
                        QVector<DAVA::ParticleEmitter *> data;
                        foreach(QModelIndex index, indexes)
                        {
                            data.push_back(SceneTreeItemParticleEmitter::GetEmitterStrict(GetItem(index)));
                        }

                        ret = MimeDataHelper2<DAVA::ParticleEmitter>::EncodeMimeData(data);
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

	types << MimeDataHelper2<DAVA::Entity>::GetMimeType();
	types << MimeDataHelper2<DAVA::ParticleEmitter>::GetMimeType();
    types << MimeDataHelper2<DAVA::ParticleLayer>::GetMimeType();
	types << MimeDataHelper2<DAVA::ParticleForce>::GetMimeType();

	return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);
	SceneTreeItem *beforeItem = GetItem(index(row, column, parent));

	int dropType = GetDropType(data);
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

			QVector<DAVA::Entity*> entitiesV = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(data);
			if(entitiesV.size() > 0)
			{
				EntityGroup entityGroup;
				for (int i = 0; i < entitiesV.size(); ++i)
				{
					entityGroup.Add((DAVA::Entity*) entitiesV[i]);
				}

				curScene->structureSystem->Move(entityGroup, parentEntity, beforeEntity);
				ret = true;
			}
		}
		break;

    case DropingEmitter:
        {

            DAVA::ParticleEffectComponent *effect = GetEffectComponent(SceneTreeItemEntity::GetEntity(parentItem));                
            QVector<DAVA::ParticleEmitter *> emittersV = MimeDataHelper2<DAVA::ParticleEmitter>::DecodeMimeData(data);
            if(NULL != effect && emittersV.size() > 0)
            {                
                DAVA::Vector<DAVA::ParticleEmitter*> emittersGroup;
                DAVA::Vector<DAVA::ParticleEffectComponent*> effectsGroup;
                emittersGroup.reserve(emittersV.size());
                effectsGroup.reserve(emittersV.size());
                for(int i = 0; i < emittersV.size(); ++i)
                {
                    emittersGroup.push_back((DAVA::ParticleEmitter *) emittersV[i]);
                    QModelIndex emitterIndex = GetIndex((DAVA::ParticleEmitter *) emittersV[i]);                    
                    DAVA::ParticleEffectComponent *oldEffect = GetEffectComponent(SceneTreeItemEntity::GetEntity(GetItem(emitterIndex.parent())));
                    effectsGroup.push_back(oldEffect);
                }

                curScene->structureSystem->MoveEmitter(emittersGroup, effectsGroup, effect, row);

				if (emittersV.size() == 1)
				{
					// moved only one emitter - keep it selected
					curScene->particlesSystem->SetEmitterSelected(effect->GetEntity(), emittersV.front());
				}
				else 
				{
					// moved several emitters - reset selection
					curScene->particlesSystem->SetEmitterSelected(nullptr, nullptr);
				}

                ret = true;
            }
        }
        break;

	case DropingLayer:
		{
			
			DAVA::ParticleEmitter* emitter = NULL;
			
			if ((parentItem->ItemType() == SceneTreeItem::EIT_Emitter)||(parentItem->ItemType() == SceneTreeItem::EIT_InnerEmitter))
			{
				emitter = ((SceneTreeItemParticleEmitter* )parentItem)->emitter;
			}

			QVector<DAVA::ParticleLayer *> layersV = MimeDataHelper2<DAVA::ParticleLayer>::DecodeMimeData(data);
			if(NULL != emitter && layersV.size() > 0)
			{
				DAVA::ParticleLayer* beforeLayer = NULL;

				if(row >= 0 && row < (int) emitter->layers.size())
				{
					beforeLayer = emitter->layers[row];
				}

				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
                DAVA::Vector<DAVA::ParticleEmitter*> emittersGroup;
                layersGroup.reserve(layersV.size());
                emittersGroup.reserve(layersV.size());
				for(int i = 0; i < layersV.size(); ++i)
				{
					layersGroup.push_back((DAVA::ParticleLayer *) layersV[i]);
                    QModelIndex emitterIndex = GetIndex((DAVA::ParticleLayer *) layersV[i]);
                    DAVA::ParticleEmitter *oldEmitter = SceneTreeItemParticleEmitter::GetEmitter(GetItem(emitterIndex.parent()));                    
                    emittersGroup.push_back(oldEmitter);
				}

				curScene->structureSystem->MoveLayer(layersGroup, emittersGroup, emitter, beforeLayer);
				ret = true;
			}
		}
		break;
	case DropingForce:
		{
			DAVA::ParticleLayer *newLayer = SceneTreeItemParticleLayer::GetLayer(parentItem);
			
			QVector<DAVA::ParticleForce*> forcesV = MimeDataHelper2<DAVA::ParticleForce>::DecodeMimeData(data);
			if(NULL != newLayer && forcesV.size())
			{
				DAVA::Vector<DAVA::ParticleForce*> forcesGroup;
                forcesGroup.reserve(forcesV.size());
                
				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
                layersGroup.reserve(forcesV.size());

				for(int i = 0; i < forcesV.size(); ++i)
				{
					QModelIndex forceIndex = GetIndex((DAVA::ParticleForce *) forcesV[i]);
					DAVA::ParticleLayer *oldLayer = SceneTreeItemParticleLayer::GetLayer(GetItem(forceIndex.parent()));

					forcesGroup.push_back((DAVA::ParticleForce *) forcesV[i]);
					layersGroup.push_back(oldLayer);
				}

				curScene->structureSystem->MoveForce(forcesGroup, layersGroup, newLayer);
				ret = true;
			}
		}
		break;
            
    case DropingMaterial:
        {
			DAVA::Entity *targetEntity = SceneTreeItemEntity::GetEntity(parentItem);

			QVector<DAVA::NMaterial*> materialsV = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);
			if(NULL != targetEntity && materialsV.size() == 1)
			{
				MaterialAssignSystem::AssignMaterialToEntity(curScene, targetEntity, materialsV[0]);
				ret = true;
			}
        }
		break;

	default:
		break;
	}

	dropAccepted = ret;
	return ret;
}

bool SceneTreeModel::DropCanBeAccepted(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);

	int dropType = GetDropType(data);
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
				QVector<DAVA::Entity *> entities = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(data);
				if(entities.size() > 0)
				{
					DAVA::Entity *targetEntity = SceneTreeItemEntity::GetEntity(parentItem);

					for (int i = 0; i < entities.size(); ++i)
					{
						DAVA::Entity *entity = (DAVA::Entity *) entities[i];
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
                        
                        // 5. or we are dropping waypoint outside of its path
                        if(GetWaypointComponent(entity))
                        {
                            if(entity->GetParent() != targetEntity)
                            {
                                ret = false;
                                break;
                            }
                        }

                        // 6. or we are dropping path inside of another path or waypoint
                        if (GetPathComponent(entity) && (GetPathComponent(targetEntity) || GetWaypointComponent(targetEntity)))
                        {
                            ret = false;
                            break;
                        }
					}
				}
			}
		}
		break;
    case DropingEmitter:
        {
            // accept layer to be dropped only to entity with particle emitter
            if(NULL != parentItem) 
            {
                if (GetEffectComponent(SceneTreeItemEntity::GetEntity(parentItem)))
                {
                    ret = true;
                }
            }

        }
        break;
	case DropingLayer:
		{
			// accept layer to be dropped only to entity with particle emitter
			if(NULL != parentItem) 
			{
				if ((parentItem->ItemType() == SceneTreeItem::EIT_Emitter)||(parentItem->ItemType() == SceneTreeItem::EIT_InnerEmitter))
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
                ret = true;
    		}
		}
        break;


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
			curScene->MarkAsChanged();
		}
	}
}

void SceneTreeModel::ResyncStructure(QStandardItem *item, DAVA::Entity *entity)
{
	SceneTreeItemEntity::DoSync(item, entity);
	RebuildIndexesCache();
}

void SceneTreeModel::SetFilter(const QString& text)
{
	filterText = text;
    ReloadFilter();
}

void SceneTreeModel::ReloadFilter()
{
	ResetFilter();

    if (!filterText.isEmpty())
    {
        const int n = rowCount();
        for (int i = 0; i < n; i++)
        {
            const QModelIndex _index = index(i, 0);
            SetFilterInternal(_index, filterText);
        }
    }
}

bool SceneTreeModel::IsFilterSet() const
{
    return !filterText.isEmpty();
}

void SceneTreeModel::SetFilterInternal(const QModelIndex& _index, const QString& text)
{
    SceneTreeItem *item = GetItem(_index);
    const QString& name = item->ItemName();
    uint32 id = 0xFFFFFFFF;

    DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);
    if(nullptr != entity)
    {
        id = entity->GetID();
    }

    if (!item->IsAcceptedByFilter())
    {
        const bool match = (text.isEmpty() || name.contains(text, Qt::CaseInsensitive) || text == QString::number(id));
        const bool isChild = _index.parent().isValid();

        item->SetAcceptByFilter(isChild || match);
        item->SetHighlight(match);
        
        if (match)
        {
            QModelIndex p = _index.parent();
            while (p.isValid())
            {
                SceneTreeItem *parentItem = GetItem(p);
                if (parentItem->IsAcceptedByFilter())
                {
                    break;
                }
                parentItem->SetAcceptByFilter(true);
                p = p.parent();
            }
        }
    }

    const int n = rowCount(_index);
    for ( int i = 0; i < n; i++)
    {
        const QModelIndex child = _index.child(i, 0);
        SetFilterInternal(child, text);
    }
}

void SceneTreeModel::ResetFilter(const QModelIndex& parent)
{
    const int n = rowCount(parent);
    for (int i = 0; i < n; i++)
    {
        const QModelIndex _index = index(i, 0, parent);
        SceneTreeItem *item = GetItem(_index);
        item->SetAcceptByFilter(false);
        item->SetHighlight(false);
        ResetFilter(_index);
    }
}

void SceneTreeModel::RebuildIndexesCache()
{
	indexesCacheEntities.clear();
	indexesCacheEmitters.clear();
    indexesCacheLayers.clear();
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
    case SceneTreeItem::EIT_Emitter:
    case SceneTreeItem::EIT_InnerEmitter:
        {
            DAVA::ParticleEmitter *emitter = SceneTreeItemParticleEmitter::GetEmitter(item);
            if (emitter)
                indexesCacheEmitters.insert(emitter, item->index());
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

int SceneTreeModel::GetDropType(const QMimeData *data) const
{
	int ret = DropingUnknown;

	if(NULL != data)
	{
        if(MimeDataHelper2<DAVA::Entity>::IsValid(data))
        {
			ret = DropingEntity;
        }
        else if(MimeDataHelper2<DAVA::ParticleEmitter>::IsValid(data))
        {
            ret = DropingEmitter;
        }
        else if(MimeDataHelper2<DAVA::ParticleLayer>::IsValid(data))
        {
			ret = DropingLayer;
        }
        else if(MimeDataHelper2<DAVA::ParticleForce>::IsValid(data))
        {
			ret = DropingForce;
        }
        else if(MimeDataHelper2<DAVA::NMaterial>::IsValid(data))
        {
			ret = DropingMaterial;
        }
	}

	return ret;
}


Qt::ItemFlags SceneTreeModel::flags ( const QModelIndex & index ) const
{
	const Qt::ItemFlags f = QStandardItemModel::flags(index);
    DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));
    if(NULL != entity)
    {
		if (!curScene->selectionSystem->IsEntitySelectable(entity))
        {
            return (f & ~Qt::ItemIsSelectable);
        }
    }

	return f;
}

QVariant SceneTreeModel::data(const QModelIndex &_index, int role) const
{
	switch (role)
    {
    case Qt::BackgroundRole:
        {
            SceneTreeItem *item = GetItem(_index);
            ParticleEmitter *emitter = SceneTreeItemParticleEmitter::GetEmitterStrict(item);
            if (nullptr != emitter && emitter->shortEffect)
            {
				static const QVariant brush(QBrush(QColor(255, 0, 0, 20)));
                return brush;
            }
        }
        break;
    default:
        break;
    }

    return QStandardItemModel::data(_index, role);
}


SceneTreeFilteringModel::SceneTreeFilteringModel(SceneTreeModel *_treeModel, QObject *parent /* = NULL */)
	: QSortFilterProxyModel(parent)
	, treeModel(_treeModel)
{
	setSourceModel(treeModel);
}

bool SceneTreeFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if (!treeModel->IsFilterSet())
        return true;

    const QModelIndex& _index = treeModel->index(sourceRow, 0, sourceParent);
    SceneTreeItem *item = treeModel->GetItem(_index);
    DVASSERT(item);

	return item->IsAcceptedByFilter();
}

QVariant SceneTreeFilteringModel::data(const QModelIndex& _index, int role) const
{
	if (!treeModel->IsFilterSet())
        return QSortFilterProxyModel::data(_index, role);

    switch ( role )
    {
    case Qt::BackgroundRole:
        {
            SceneTreeItem *item = treeModel->GetItem(mapToSource(_index));
            if (item->IsHighlighed())
            {
				static const QVariant brush(QBrush(QColor(0, 255, 0, 20)));
				return brush;
            }
        }
        break;
    default:
        break;
    }

    return QSortFilterProxyModel::data(_index, role);
}


