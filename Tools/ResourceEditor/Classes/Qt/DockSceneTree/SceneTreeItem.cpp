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

QIcon SceneTreeItem::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/node.png");
	return icon;
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

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Entity)
	{
		SceneTreeItemEntity *itemEntity = (SceneTreeItemEntity *) item;
		ret = itemEntity->entity;
	}

	return ret;
}

QString SceneTreeItemEntity::ItemName() const
{
	QString ret; 

	if(NULL != entity)
	{
		ret = entity->GetName().c_str();
	}

	return ret;
}

QVariant SceneTreeItemEntity::ItemData() const
{
	return qVariantFromValue(entity);
}

QIcon SceneTreeItemEntity::ItemIcon() const
{
	static QIcon effectIcon(":/QtIcons/effect.png");
	static QIcon emitterIcon(":/QtIcons/emitter_particle.png");
	static QIcon renderobjIcon(":/QtIcons/render_object.png");
	static QIcon lodobjIcon(":/QtIcons/lod_object.png");
	static QIcon userobjIcon(":/QtIcons/user_object.png");
	static QIcon landscapeIcon(":/QtIcons/heightmapeditor.png");
	static QIcon cameraIcon(":/QtIcons/camera.png");
	static QIcon lightIcon(":/QtIcons/light.png");
	static QIcon shadowIcon(":/QtIcons/shadow.png");
	static QIcon switchIcon(":/QtIcons/switch.png");
	static QIcon windIcon(":/QtIcons/wind.png");
    static QIcon soIcon(":/QtIcons/so.png");
    static QIcon pathIcon(":/QtIcons/path.png");
    static QIcon grassIcon(":/QtIcons/grass.png");

	QIcon ret;

	if(NULL != entity)
	{	
        if(NULL != entity->GetComponent(DAVA::Component::STATIC_OCCLUSION_COMPONENT))
        {
            ret = soIcon;
        }
		if(NULL != DAVA::GetEffectComponent(entity))
		{
			ret = effectIcon;
		}
		else if(NULL != DAVA::GetLandscape(entity))
		{
			ret = landscapeIcon;
		}
		else if(NULL != GetLodComponent(entity))
		{
			ret = lodobjIcon;
		}
		else if(NULL != GetSwitchComponent(entity))
		{
			ret = switchIcon;
		}
        else if (NULL != DAVA::GetVegetation(entity))
        {
            ret = grassIcon;
        }
		else if(NULL != DAVA::GetRenderObject(entity))
		{
			ret = renderobjIcon;
		}
		else if(NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT))
		{
			ret = userobjIcon;
		}
		else if(NULL != DAVA::GetCamera(entity))
		{
			ret = cameraIcon;
		}
		else if(NULL != DAVA::GetLight(entity))
		{
			ret = lightIcon;
		}
		else if(NULL != DAVA::GetWindComponent(entity))
		{
			ret = windIcon;
		}
        else if(NULL != DAVA::GetPathComponent(entity))
        {
            ret = pathIcon;
        }
	}

	if(ret.isNull())
	{
		ret = SceneTreeItem::ItemIcon();
	}

	return ret;
}


void SceneTreeItemEntity::DoSync(QStandardItem *rootItem, DAVA::Entity *entity)
{
	if(NULL != rootItem && NULL != entity)
	{
		DAVA::int32 i;
		QSet<DAVA::Entity *> entitiesSet;
		QSet<DAVA::ParticleEmitter *> emitterSet;

		DAVA::ParticleEffectComponent *effect = DAVA::GetEffectComponent(entity);		

		// remember all entity childs
		for(i = 0; i < entity->GetChildrenCount(); ++i)
		{
			entitiesSet.insert(entity->GetChild(i));
		}

		// remember all particle emitters
		if(NULL != effect)
		{			
			for(DAVA::int32 i = 0; i < effect->GetEmittersCount(); ++i)
			{
				emitterSet.insert(effect->GetEmitter(i));
			}
		}

		// remove items that are not in set
		for(int i = 0; i < rootItem->rowCount(); ++i)
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

	if(NULL != item && ((item->ItemType() == SceneTreeItem::EIT_Emitter)||(item->ItemType() == SceneTreeItem::EIT_InnerEmitter)))
	{
		SceneTreeItemParticleEmitter *itemEmitter = (SceneTreeItemParticleEmitter *) item;
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
	if(NULL != rootItem && NULL != emitter)
	{
		SceneTreeItemParticleEmitter *rootEmitterItem = (SceneTreeItemParticleEmitter *) rootItem;

		for(int i = 0; i < rootItem->rowCount(); )
		{	
			DVASSERT(((SceneTreeItem*)rootItem->child(i))->ItemType() == SceneTreeItem::EIT_Layer);			
			rootItem->removeRow(i);						
		}
		for (DAVA::uint32 i=0; i<(DAVA::uint32)emitter->layers.size(); ++i)
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


QIcon SceneTreeItemParticleEmitter::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/emitter_particle.png");
	return icon;
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
	if(NULL != layer)
	{
		setCheckable(true);

		if(layer->isDisabled)
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

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Layer)
	{
		SceneTreeItemParticleLayer *itemLayer = (SceneTreeItemParticleLayer *) item;
		ret = itemLayer->layer;
	}

	return ret;
}

QString SceneTreeItemParticleLayer::ItemName() const
{
	QString ret; 

	if(NULL != layer)
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
	if(NULL != rootItem && NULL != layer)
	{	
		SceneTreeItemParticleLayer *rootLayerItem = (SceneTreeItemParticleLayer *) rootItem;
		bool hadInnerEmmiter = false;
		for (int i=0; i<rootItem->rowCount(); i++)
		{
			bool keepItem = false;
			SceneTreeItem*item = (SceneTreeItem*)rootItem->child(i);
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

QIcon SceneTreeItemParticleLayer::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/layer_particle.png");
	return icon;
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

	if(NULL != item && item->ItemType() == SceneTreeItem::EIT_Force)
	{
		SceneTreeItemParticleForce *itemForce = (SceneTreeItemParticleForce *) item;
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

QIcon SceneTreeItemParticleForce::ItemIcon() const
{
	static QIcon icon = QIcon(":/QtIcons/force.png");
	return icon;
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


