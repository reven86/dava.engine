#include <QSet>
#include "DockSceneTree/SceneTreeItem.h"
#include "Commands2/ConvertToShadowCommand.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

SceneTreeItem::SceneTreeItem(eItemType _type, DAVA::BaseObject* object_)
    : object(object_)
    , type(_type)
{
}

DAVA::BaseObject* SceneTreeItem::GetItemObject() const
{
    return static_cast<DAVA::BaseObject*>(object.GetContainedObject());
}

QVariant SceneTreeItem::data(int role) const
{
    QVariant v;

    switch (role)
    {
    case Qt::DecorationRole:
        v = ItemIcon();
        break;
    case Qt::DisplayRole:
        v = ItemName();
        break;
    case Qt::ToolTipRole:
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

    if (v.isNull())
    {
        v = QStandardItem::data(role);
    }

    return v;
}

DAVA::uint32 SceneTreeItem::ItemType() const
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
    : SceneTreeItem(SceneTreeItem::EIT_Entity, _entity)
{
    DoSync(this, _entity);
}

DAVA::Entity* SceneTreeItemEntity::GetEntity() const
{
    return object.AsEntity();
}

DAVA::Entity* SceneTreeItemEntity::GetEntity(SceneTreeItem* item)
{
    if ((nullptr != item) && (item->ItemType() == SceneTreeItem::EIT_Entity))
    {
        SceneTreeItemEntity* itemEntity = static_cast<SceneTreeItemEntity*>(item);
        return itemEntity->GetEntity();
    }

    return nullptr;
}

QString SceneTreeItemEntity::ItemName() const
{
    QString ret;

    if (object.ContainsObject())
    {
        ret = GetEntity()->GetName().c_str();
    }

    return ret;
}

QVariant SceneTreeItemEntity::ItemData() const
{
    return qVariantFromValue(GetEntity());
}

const QIcon& SceneTreeItemEntity::ItemIcon() const
{
    DAVA::Entity* entity = GetEntity();

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

void SceneTreeItemEntity::DoSync(QStandardItem* rootItem, DAVA::Entity* entity)
{
    if (nullptr != rootItem && nullptr != entity)
    {
        DAVA::int32 i;
        QSet<DAVA::Entity*> entitiesSet;
        QSet<DAVA::ParticleEmitterInstance*> emitterSet;

        DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);

        // remember all entity childs
        for (i = 0; i < entity->GetChildrenCount(); ++i)
        {
            entitiesSet.insert(entity->GetChild(i));
        }

        // remember all particle emitters
        if (nullptr != effect)
        {
            for (DAVA::uint32 i = 0; i < effect->GetEmittersCount(); ++i)
            {
                emitterSet.insert(effect->GetEmitterInstance(i));
            }
        }

        // remove items that are not in set
        for (int i = 0; i < rootItem->rowCount(); ++i)
        {
            bool doRemove = true;
            SceneTreeItem* childItem = (SceneTreeItem*)rootItem->child(i);

            if (childItem->ItemType() == SceneTreeItem::EIT_Entity)
            {
                SceneTreeItemEntity* entityItem = (SceneTreeItemEntity*)childItem;
                if (entitiesSet.contains(entityItem->GetEntity()))
                {
                    doRemove = false;
                }
            }
            else if (childItem->ItemType() == SceneTreeItem::EIT_Emitter)
            {
                SceneTreeItemParticleEmitter* emitterItem = (SceneTreeItemParticleEmitter*)childItem;
                if (emitterSet.contains(emitterItem->GetEmitterInstance()))
                {
                    doRemove = false;
                }
            }

            if (doRemove)
            {
                rootItem->removeRow(i);
                i--;
            }
        }

        entitiesSet.clear();
        emitterSet.clear();

        // add entities
        int row = 0;

        for (int i = 0; i < entity->GetChildrenCount(); ++i)
        {
            bool repeatStep;
            DAVA::Entity* childEntity = entity->GetChild(i);

            do
            {
                SceneTreeItem* item = (SceneTreeItem*)rootItem->child(row);
                DAVA::Entity* itemEntity = SceneTreeItemEntity::GetEntity(item);

                repeatStep = false;

                // remove items that we already add
                while (entitiesSet.contains(itemEntity))
                {
                    rootItem->removeRow(row);

                    item = (SceneTreeItem*)rootItem->child(row);
                    itemEntity = SceneTreeItemEntity::GetEntity(item);
                }

                // append entity that isn't in child items list
                if (NULL == item)
                {
                    rootItem->appendRow(new SceneTreeItemEntity(childEntity));
                }
                else if (childEntity != itemEntity)
                {
                    // now we should decide what to do: remove item or insert it

                    // calc len until itemEntity will be found in real entity childs
                    int lenUntilRealEntity = 0;
                    for (int j = i; j < entity->GetChildrenCount(); ++j)
                    {
                        if (entity->GetChild(j) == itemEntity)
                        {
                            lenUntilRealEntity = j - i;
                            break;
                        }
                    }

                    // calc len until current real entity child will be found in current item childs
                    int lenUntilItem = 0;
                    for (int j = i; j < rootItem->rowCount(); ++j)
                    {
                        SceneTreeItem* itm = (SceneTreeItem*)rootItem->child(j);
                        DAVA::Entity* itmEn = SceneTreeItemEntity::GetEntity(itm);

                        if (childEntity == itmEn)
                        {
                            lenUntilItem = j - i;
                            break;
                        }
                    }

                    if (lenUntilRealEntity >= lenUntilItem)
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
            while (repeatStep);

            // remember that we add that entity
            entitiesSet.insert(childEntity);

            row++;
        }

        if (effect)
        {
            for (DAVA::uint32 i = 0; i < effect->GetEmittersCount(); ++i)
            {
                bool repeatStep = false;

                DAVA::ParticleEmitterInstance* emitter = effect->GetEmitterInstance(i);
                do
                {
                    repeatStep = false;
                    SceneTreeItem* item = (SceneTreeItem*)rootItem->child(row);
                    DAVA::ParticleEmitterInstance* itemEmitter = SceneTreeItemParticleEmitter::GetEmitterInstance(item);

                    // remove items that we already add
                    if (emitterSet.contains(itemEmitter))
                    {
                        rootItem->removeRow(row);
                    }

                    if (item == nullptr)
                    {
                        rootItem->appendRow(new SceneTreeItemParticleEmitter(effect, emitter));
                    }
                    else if (emitter != itemEmitter)
                    {
                        // now we should decide what to do: remove layer or insert it

                        // calc len until itemEntity will be found in real
                        DAVA::int32 lenUntilRealLayer = 0;
                        for (DAVA::uint32 j = i; j < effect->GetEmittersCount(); ++j)
                        {
                            if (effect->GetEmitterInstance(j) == itemEmitter)
                            {
                                lenUntilRealLayer = j - i;
                                break;
                            }
                        }

                        // calc len until current real entity child will be found in current item childs
                        int lenUntilItem = 0;
                        for (int j = i; j < rootItem->rowCount(); ++j)
                        {
                            SceneTreeItem* itm = (SceneTreeItem*)rootItem->child(j);
                            DAVA::ParticleEmitterInstance* itmEmm = SceneTreeItemParticleEmitter::GetEmitterInstance(itm);

                            if (emitter == itmEmm)
                            {
                                lenUntilItem = j - i;
                                break;
                            }
                        }

                        if (lenUntilRealLayer >= lenUntilItem)
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
        if (row < rootItem->rowCount())
        {
            rootItem->removeRows(row, rootItem->rowCount() - row);
        }
    }
}

SceneTreeItemParticleEmitter::SceneTreeItemParticleEmitter(DAVA::ParticleEffectComponent* _effect, DAVA::ParticleEmitterInstance* _emitter)
    : SceneTreeItem(SceneTreeItem::EIT_Emitter, _emitter)
    , effect(_effect)
{
    DoSync(this, _emitter);
}

DAVA::ParticleEmitterInstance* SceneTreeItemParticleEmitter::GetEmitterInstance() const
{
    return object.Cast<DAVA::ParticleEmitterInstance>();
}

DAVA::ParticleEmitterInstance* SceneTreeItemParticleEmitter::GetEmitterInstance(SceneTreeItem* item)
{
    if ((nullptr != item) && ((item->ItemType() == SceneTreeItem::EIT_Emitter) || (item->ItemType() == SceneTreeItem::EIT_InnerEmitter)))
    {
        SceneTreeItemParticleEmitter* itemEmitter = static_cast<SceneTreeItemParticleEmitter*>(item);
        return itemEmitter->GetEmitterInstance();
    }

    return nullptr;
}

DAVA::ParticleEmitterInstance* SceneTreeItemParticleEmitter::GetEmitterInstanceStrict(SceneTreeItem* item)
{
    if ((nullptr != item) && (item->ItemType() == SceneTreeItem::EIT_Emitter))
    {
        SceneTreeItemParticleEmitter* itemEmitter = (SceneTreeItemParticleEmitter*)item;
        return itemEmitter->GetEmitterInstance();
    }

    return nullptr;
}

void SceneTreeItemParticleEmitter::DoSync(QStandardItem* rootItem, DAVA::ParticleEmitterInstance* emitter)
{
    if ((nullptr == rootItem) || (nullptr == emitter))
        return;

    SceneTreeItemParticleEmitter* rootEmitterItem = static_cast<SceneTreeItemParticleEmitter*>(rootItem);

    for (int i = 0; i < rootItem->rowCount();)
    {
        DVASSERT(((SceneTreeItem*)rootItem->child(i))->ItemType() == SceneTreeItem::EIT_Layer);
        rootItem->removeRow(i);
    }
    for (auto layer : emitter->GetEmitter()->layers)
    {
        rootItem->appendRow(new SceneTreeItemParticleLayer(rootEmitterItem->effect, emitter, layer));
    }
}

QString SceneTreeItemParticleEmitter::ItemName() const
{
    return QString(GetEmitterInstance()->GetEmitter()->name.c_str());
}

QVariant SceneTreeItemParticleEmitter::ItemData() const
{
    return qVariantFromValue(GetEmitterInstance());
}

const QIcon& SceneTreeItemParticleEmitter::ItemIcon() const
{
    return SharedIcon(":/QtIcons/emitter_particle.png");
}

// =========================================================================================
// SceneTreeItemParticleLayer
// =========================================================================================

SceneTreeItemParticleLayer::SceneTreeItemParticleLayer(DAVA::ParticleEffectComponent* effect_,
                                                       DAVA::ParticleEmitterInstance* instance_, DAVA::ParticleLayer* layer_)
    : SceneTreeItem(SceneTreeItem::EIT_Layer, layer_)
    , effect(effect_)
    , emitterInstance(instance_)
    , hasInnerEmmiter(false)
{
    if (nullptr != layer_)
    {
        setCheckable(true);

        if (layer_->isDisabled)
        {
            setCheckState(Qt::Unchecked);
        }
        else
        {
            setCheckState(Qt::Checked);
        }
        hasInnerEmmiter = (layer_->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES); // layer can still have inner emitter cached
    }

    DoSync(this, layer_);
}

DAVA::ParticleLayer* SceneTreeItemParticleLayer::GetLayer() const
{
    return object.Cast<DAVA::ParticleLayer>();
}

DAVA::ParticleLayer* SceneTreeItemParticleLayer::GetLayer(SceneTreeItem* item)
{
    DAVA::ParticleLayer* ret = NULL;

    if ((nullptr != item) && (item->ItemType() == SceneTreeItem::EIT_Layer))
    {
        SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)item;
        ret = static_cast<DAVA::ParticleLayer*>(itemLayer->GetItemObject());
    }

    return ret;
}

QString SceneTreeItemParticleLayer::ItemName() const
{
    QString ret;

    if (object.ContainsObject())
    {
        ret = GetLayer()->layerName.c_str();
    }

    return ret;
}

QVariant SceneTreeItemParticleLayer::ItemData() const
{
    return qVariantFromValue(GetLayer());
}

void SceneTreeItemParticleLayer::DoSync(QStandardItem* rootItem, DAVA::ParticleLayer* layer)
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
                    DAVA::ScopedPtr<DAVA::ParticleEmitterInstance> instance(new DAVA::ParticleEmitterInstance(nullptr, layer->innerEmitter, true));
                    ((SceneTreeItemParticleInnerEmitter*)item)->DoSync(item, instance);
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

        for (size_t i = 0; i < itemsCount; ++i)
        {
            items.push_back(new SceneTreeItemParticleForce(layer, layer->forces[i]));
        }

        if (items.size() > 0)
        {
            rootItem->insertRows(0, items);
        }

        if (!hadInnerEmmiter && (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES))
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

SceneTreeItemParticleForce::SceneTreeItemParticleForce(DAVA::ParticleLayer* _layer, DAVA::ParticleForce* _force)
    : SceneTreeItem(SceneTreeItem::EIT_Force, _force)
    , layer(_layer)
{
}

DAVA::ParticleForce* SceneTreeItemParticleForce::GetForce() const
{
    return object.Cast<DAVA::ParticleForce>();
}

DAVA::ParticleForce* SceneTreeItemParticleForce::GetForce(SceneTreeItem* item)
{
    if ((nullptr != item) && (item->ItemType() == SceneTreeItem::EIT_Force))
    {
        SceneTreeItemParticleForce* itemForce = (SceneTreeItemParticleForce*)item;
        return itemForce->GetForce();
    }

    return nullptr;
}

QString SceneTreeItemParticleForce::ItemName() const
{
    return "force";
}

QVariant SceneTreeItemParticleForce::ItemData() const
{
    return qVariantFromValue(GetForce());
}

const QIcon& SceneTreeItemParticleForce::ItemIcon() const
{
    return SharedIcon(":/QtIcons/force.png");
}

SceneTreeItemParticleInnerEmitter::SceneTreeItemParticleInnerEmitter(DAVA::ParticleEffectComponent* effect_, DAVA::ParticleEmitter* emitter_,
                                                                     DAVA::ParticleLayer* parentLayer_)
    : SceneTreeItemParticleEmitter(effect_, new DAVA::ParticleEmitterInstance(effect_, emitter_, true)) // local instance will be initialized after
    , parent(parentLayer_)
    , localInstance(object.Cast<DAVA::ParticleEmitterInstance>())
{
    type = SceneTreeItem::EIT_InnerEmitter;
    DoSync(this, localInstance);
}

QString SceneTreeItemParticleInnerEmitter::ItemName() const
{
    return "innerEmmiter";
}
