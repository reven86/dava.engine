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


#include <QSet>
#include "DockSceneTree/SceneTreeItem.h"
#include "Commands2/ConvertToShadowCommand.h"

// framework
#include "Scene3d/Components/ComponentHelpers.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

SceneTreeItem::SceneTreeItem(eItemType _type)
	: type(_type)
    , isAcceptedByFilter(false)
    , isHighlighted(false)
{ 
}

SceneTreeItem::~SceneTreeItem()
{ }

QVariant SceneTreeItem::data(int role) const
{
	QVariant v;

	switch(role)
	{
	case Qt::DecorationRole:
		v = ItemIcon();
		break;
	case Qt::DisplayRole:
		v = ItemName();
		break;
	case EIDR_Type:
		v = ItemType();
		break;
	case EIDR_Data:
		v = ItemData();
		break;
	default:
		break;
	}

	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}


int SceneTreeItem::ItemType() const
{
	return type;
}

const QIcon& SceneTreeItem::ItemIcon() const
{
    return SharedIcon(":/QtIcons/node.png");
}

bool SceneTreeItem::IsAcceptedByFilter() const
{
    return isAcceptedByFilter;
}

void SceneTreeItem::SetAcceptByFilter(bool state)
{
    isAcceptedByFilter = state;
}

bool SceneTreeItem::IsHighlighed() const
{
    return isHighlighted;
}

void SceneTreeItem::SetHighlight(bool state)
{
    isHighlighted = state;
}


// =========================================================================================
// SceneTreeItemEntity
// =========================================================================================

SceneTreeItemEntity::SceneTreeItemEntity(DAVA::Entity* _entity)
	: SceneTreeItem(SceneTreeItem::EIT_Entity)
	, entity(_entity)
{
	DoSync(this, entity);
}

SceneTreeItemEntity::~SceneTreeItemEntity()
{ }

DAVA::Entity* SceneTreeItemEntity::GetEntity(SceneTreeItem *item)
{
	DAVA::Entity *ret = NULL;

    if (nullptr != item && item->ItemType() == SceneTreeItem::EIT_Entity)
    {
        SceneTreeItemEntity* itemEntity = (SceneTreeItemEntity*)item;
        ret = itemEntity->entity;
    }

    return ret;
}

QString SceneTreeItemEntity::ItemName() const
{
    QString ret;

    if (nullptr != entity)
    {
        ret = entity->GetName().c_str();
    }

    return ret;
}

QVariant SceneTreeItemEntity::ItemData() const
{
	return qVariantFromValue(entity);
}

const QIcon& SceneTreeItemEntity::ItemIcon() const
{
    if (nullptr != entity)
    {
        if (nullptr != entity->GetComponent(DAVA::Component::STATIC_OCCLUSION_COMPONENT))
        {
            return SharedIcon(":/QtIcons/so.png");
        }
        if (nullptr != DAVA::GetEffectComponent(entity))
        {
            return SharedIcon(":/QtIcons/effect.png");
        }
        else if (nullptr != DAVA::GetLandscape(entity))
        {
            return SharedIcon(":/QtIcons/heightmapeditor.png");
        }
        else if (nullptr != GetLodComponent(entity))
        {
            return SharedIcon(":/QtIcons/lod_object.png");
        }
        else if (nullptr != GetSwitchComponent(entity))
        {
            return SharedIcon(":/QtIcons/switch.png");
        }
        else if (nullptr != DAVA::GetVegetation(entity))
        {
            return SharedIcon(":/QtIcons/grass.png");
        }
        else if (nullptr != DAVA::GetRenderObject(entity))
        {
            return SharedIcon(":/QtIcons/render_object.png");
        }
        else if (nullptr != entity->GetComponent(DAVA::Component::USER_COMPONENT))
        {
            return SharedIcon(":/QtIcons/user_object.png");
        }
        else if (nullptr != DAVA::GetCamera(entity))
        {
            return SharedIcon(":/QtIcons/camera.png");
        }
        else if (nullptr != DAVA::GetLight(entity))
        {
            return SharedIcon(":/QtIcons/light.png");
        }
        else if (nullptr != DAVA::GetWindComponent(entity))
        {
            return SharedIcon(":/QtIcons/wind.png");
        }
        else if (nullptr != DAVA::GetPathComponent(entity))
        {
            return SharedIcon(":/QtIcons/path.png");
        }
	}

    return SceneTreeItem::ItemIcon();
}

void SceneTreeItemEntity::DoSync(QStandardItem *rootItem, DAVA::Entity *entity)
{
    if (nullptr != rootItem && nullptr != entity)
    {
        DAVA::int32 i;
        QSet<DAVA::Entity*> entitiesSet;
        QSet<DAVA::ParticleEmitter*> emitterSet;

        DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);

        // remember all entity childs
        for (i = 0; i < entity->GetChildrenCount(); ++i)
        {
            entitiesSet.insert(entity->GetChild(i));
        }

        // remember all particle emitters
        if (nullptr != effect)
        {
            for (DAVA::int32 i = 0; i < effect->GetEmittersCount(); ++i)
            {
                emitterSet.insert(effect->GetEmitter(i));
            }
        }

        // remove items that are not in set
        for (int i = 0; i < rootItem->rowCount(); ++i)
        {
			bool doRemove = true;
			SceneTreeItem *childItem = (SceneTreeItem *) rootItem->child(i);

			if(childItem->ItemType() == SceneTreeItem::EIT_Entity)
			{
				SceneTreeItemEntity *entityItem = (SceneTreeItemEntity *) childItem;
				if(entitiesSet.contains(entityItem->entity))
				{
					doRemove = false;
				}
			}
			else if(childItem->ItemType() == SceneTreeItem::EIT_Emitter)
			{
				SceneTreeItemParticleEmitter *emitterItem = (SceneTreeItemParticleEmitter *) childItem;
				if(emitterSet.contains(emitterItem->emitter))
				{
					doRemove = false;
				}
			}

			if(doRemove)
			{
				rootItem->removeRow(i);
				i--;
			}
		}

		entitiesSet.clear();
		emitterSet.clear();

		// add entities
		int row = 0;

		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			bool repeatStep;
			DAVA::Entity *childEntity = entity->GetChild(i);

			do
			{
				SceneTreeItem *item = (SceneTreeItem *) rootItem->child(row);
				DAVA::Entity *itemEntity = SceneTreeItemEntity::GetEntity(item);

				repeatStep = false;

				// remove items that we already add
				while(entitiesSet.contains(itemEntity))
				{
					rootItem->removeRow(row);

					item = (SceneTreeItem *) rootItem->child(row);
					itemEntity = SceneTreeItemEntity::GetEntity(item);
				}

				// append entity that isn't in child items list
				if(NULL == item)
				{
					rootItem->appendRow(new SceneTreeItemEntity(childEntity));
				}
				else if(childEntity != itemEntity)
				{
					// now we should decide what to do: remove item or insert it

					// calc len until itemEntity will be found in real entity childs
					int lenUntilRealEntity = 0;
					for(int j = i; j < entity->GetChildrenCount(); ++j)
					{
						if(entity->GetChild(j) == itemEntity)
						{
							lenUntilRealEntity = j - i;
							break;
						}
					}

					// calc len until current real entity child will be found in current item childs
					int lenUntilItem = 0;
					for(int j = i; j < rootItem->rowCount(); ++j)
					{
						SceneTreeItem *itm = (SceneTreeItem *) rootItem->child(j);
						DAVA::Entity *itmEn = SceneTreeItemEntity::GetEntity(itm);

						if(childEntity == itmEn)
						{
							lenUntilItem = j - i;
							break;
						}
					}

					if(lenUntilRealEntity >= lenUntilItem)
					{
						rootItem->removeRow(row);
						repeatStep = true;
					}
					else
					{
						rootItem->insertRow(row, new SceneTreeItemEntity(childEntity));
					}
				}
				else
				{
					DoSync(item, itemEntity);
				}
			}
			while(repeatStep);

			// remember that we add that entity
			entitiesSet.insert(childEntity);

			row++;
		}
		
		if(effect)
		{
			for(DAVA::int32 i = 0; i < effect->GetEmittersCount(); ++i)
			{
				bool repeatStep;
				DAVA::ParticleEmitter* emitter = effect->GetEmitter(i);

				do 
				{
					SceneTreeItem *item = (SceneTreeItem *) rootItem->child(row);
					DAVA::ParticleEmitter *itemEmitter = SceneTreeItemParticleEmitter::GetEmitter(item);

					repeatStep = false;

					// remove items that we already add
					if (emitterSet.contains(itemEmitter))
					{
						rootItem->removeRow(row);
					}

					if(NULL == item)
					{
						rootItem->appendRow(new SceneTreeItemParticleEmitter(effect, emitter));
					}
					else if(emitter != itemEmitter)
					{
						// now we should decide what to do: remove layer or insert it

						// calc len until itemEntity will be found in real
						int lenUntilRealLayer = 0;
						for(int j = i; j < (int)effect->GetEmittersCount(); ++j)
						{
							if(effect->GetEmitter(j) == itemEmitter)
							{
								lenUntilRealLayer = j - i;
								break;
							}
						}

						// calc len until current real entity child will be found in current item childs
						int lenUntilItem = 0;
						for(int j = i; j < rootItem->rowCount(); ++j)
						{
							SceneTreeItem *itm = (SceneTreeItem *) rootItem->child(j);
							DAVA::ParticleEmitter *itmEmm = SceneTreeItemParticleEmitter::GetEmitter(itm);

							if(emitter == itmEmm)
							{
								lenUntilItem = j - i;
								break;
							}
						}

						if(lenUntilRealLayer >= lenUntilItem)
						{
							rootItem->removeRow(row);
							repeatStep = true;
						}
						else
						{
							rootItem->insertRow(row, new SceneTreeItemParticleEmitter(effect, emitter));
						}
					}
					else
					{
						SceneTreeItemParticleEmitter::DoSync(item, itemEmitter);
					}
				} while (repeatStep);

				row++;
				emitterSet.insert(emitter);
			}
		}

		// remove all other rows
		if(row < rootItem->rowCount())
		{
			rootItem->removeRows(row, rootItem->rowCount() - row);
		}
	}
}


SceneTreeItemParticleEmitter::SceneTreeItemParticleEmitter(DAVA::ParticleEffectComponent *_effect, DAVA::ParticleEmitter* _emitter) : SceneTreeItem(SceneTreeItem::EIT_Emitter), effect(_effect), emitter(_emitter)
{
	DoSync(this, emitter);
}
SceneTreeItemParticleEmitter::~SceneTreeItemParticleEmitter()
{

}

DAVA::ParticleEmitter* SceneTreeItemParticleEmitter::GetEmitter(SceneTreeItem *item)
{
	DAVA::ParticleEmitter *ret = NULL;

    if (nullptr != item && ((item->ItemType() == SceneTreeItem::EIT_Emitter) || (item->ItemType() == SceneTreeItem::EIT_InnerEmitter)))
    {
        SceneTreeItemParticleEmitter* itemEmitter = (SceneTreeItemParticleEmitter*)item;
        ret = itemEmitter->emitter;
    }
    return ret;
}

DAVA::ParticleEmitter* SceneTreeItemParticleEmitter::GetEmitterStrict(SceneTreeItem *item)
{
    DAVA::ParticleEmitter *ret = nullptr;

    if (nullptr != item && (item->ItemType() == SceneTreeItem::EIT_Emitter))
    {
        SceneTreeItemParticleEmitter *itemEmitter = (SceneTreeItemParticleEmitter *) item;
        ret = itemEmitter->emitter;
    }
    return ret;
}
void SceneTreeItemParticleEmitter::DoSync(QStandardItem *rootItem, DAVA::ParticleEmitter *emitter)
{
    if (nullptr != rootItem && nullptr != emitter)
    {
        SceneTreeItemParticleEmitter* rootEmitterItem = (SceneTreeItemParticleEmitter*)rootItem;

        for (int i = 0; i < rootItem->rowCount();)
        {
            DVASSERT(((SceneTreeItem*)rootItem->child(i))->ItemType() == SceneTreeItem::EIT_Layer);
            rootItem->removeRow(i);
        }
        for (DAVA::uint32 i = 0; i < (DAVA::uint32)emitter->layers.size(); ++i)
        {
			rootItem->appendRow(new SceneTreeItemParticleLayer(rootEmitterItem->effect, emitter, emitter->layers[i]));
		}					
	}
}

QString SceneTreeItemParticleEmitter::ItemName() const
{
	return QString(emitter->name.c_str());
}

QVariant SceneTreeItemParticleEmitter::ItemData() const
{
	return qVariantFromValue(emitter);
}

const QIcon& SceneTreeItemParticleEmitter::ItemIcon() const
{
    return SharedIcon(":/QtIcons/emitter_particle.png");
}

// =========================================================================================
// SceneTreeItemParticleLayer
// =========================================================================================

SceneTreeItemParticleLayer::SceneTreeItemParticleLayer(DAVA::ParticleEffectComponent *_effect, DAVA::ParticleEmitter* _emitter, DAVA::ParticleLayer *_layer)
	: SceneTreeItem(SceneTreeItem::EIT_Layer)
	, effect(_effect)
	, emitter(_emitter)
	, layer(_layer)
	, hasInnerEmmiter(false)
{
    if (nullptr != layer)
    {
        setCheckable(true);

        if (layer->isDisabled)
        {
            setCheckState(Qt::Unchecked);
        }
        else
        {
            setCheckState(Qt::Checked);
		}
		hasInnerEmmiter = (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES); //layer can still have inner emitter cached
	}

	DoSync(this, layer);
}

SceneTreeItemParticleLayer::~SceneTreeItemParticleLayer()
{ }

DAVA::ParticleLayer* SceneTreeItemParticleLayer::GetLayer(SceneTreeItem *item)
{
	DAVA::ParticleLayer *ret = NULL;

    if (nullptr != item && item->ItemType() == SceneTreeItem::EIT_Layer)
    {
        SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)item;
        ret = itemLayer->layer;
    }

    return ret;
}

QString SceneTreeItemParticleLayer::ItemName() const
{
    QString ret;

    if (nullptr != layer)
    {
        ret = layer->layerName.c_str();
    }

    return ret;
}

QVariant SceneTreeItemParticleLayer::ItemData() const
{
	return qVariantFromValue(layer);
}

void SceneTreeItemParticleLayer::DoSync(QStandardItem *rootItem, DAVA::ParticleLayer *layer)
{
    if (nullptr != rootItem && nullptr != layer)
    {
        SceneTreeItemParticleLayer* rootLayerItem = (SceneTreeItemParticleLayer*)rootItem;
        bool hadInnerEmmiter = false;
        for (int i = 0; i < rootItem->rowCount(); i++)
        {
            bool keepItem = false;
            SceneTreeItem* item = (SceneTreeItem*)rootItem->child(i);
            if ((item)->ItemType() == SceneTreeItem::EIT_InnerEmitter)
            {
				hadInnerEmmiter = true;
				if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
				{
					keepItem = true;
					((SceneTreeItemParticleInnerEmitter*)item)->DoSync(item, layer->innerEmitter);
				}
			}
			if (!keepItem)
			{
				rootItem->removeRow(i);
				i--;
			}
		}

		size_t itemsCount = layer->forces.size();
		QList<QStandardItem*> items;
		
		for(size_t i = 0; i < itemsCount; ++i)
		{
			items.push_back(new SceneTreeItemParticleForce(layer, layer->forces[i]));
		}

		if(items.size() > 0)
		{
			rootItem->insertRows(0, items);
		}

		if (!hadInnerEmmiter&&(layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES))
		{
			rootItem->appendRow(new SceneTreeItemParticleInnerEmitter(rootLayerItem->effect, layer->innerEmitter, layer));
		}
	}
}

const QIcon& SceneTreeItemParticleLayer::ItemIcon() const
{
    return SharedIcon(":/QtIcons/layer_particle.png");
}

// =========================================================================================
// SceneTreeItemParticleForce
// =========================================================================================

SceneTreeItemParticleForce::SceneTreeItemParticleForce(DAVA::ParticleLayer *_layer, DAVA::ParticleForce *_force)
	: SceneTreeItem(SceneTreeItem::EIT_Force)
	, layer(_layer)
	, force(_force)
{ }

DAVA::ParticleForce* SceneTreeItemParticleForce::GetForce(SceneTreeItem *item)
{
	DAVA::ParticleForce *ret = NULL;

    if (nullptr != item && item->ItemType() == SceneTreeItem::EIT_Force)
    {
        SceneTreeItemParticleForce* itemForce = (SceneTreeItemParticleForce*)item;
        ret = itemForce->force;
    }

    return ret;
}

SceneTreeItemParticleForce::~SceneTreeItemParticleForce()
{ }

QString SceneTreeItemParticleForce::ItemName() const
{
	return "force";
}

QVariant SceneTreeItemParticleForce::ItemData() const
{
	return qVariantFromValue(force);
}

const QIcon& SceneTreeItemParticleForce::ItemIcon() const
{
    return SharedIcon(":/QtIcons/force.png");
}



SceneTreeItemParticleInnerEmitter::SceneTreeItemParticleInnerEmitter(DAVA::ParticleEffectComponent *_effect, DAVA::ParticleEmitter *_emitter, DAVA::ParticleLayer *_parentLayer)
	: SceneTreeItemParticleEmitter(_effect, _emitter)
	, parent(_parentLayer)	
{
	type = SceneTreeItem::EIT_InnerEmitter;
}



SceneTreeItemParticleInnerEmitter::~SceneTreeItemParticleInnerEmitter()
{ }



QString SceneTreeItemParticleInnerEmitter::ItemName() const
{
	return "innerEmmiter";
}


