#include <QMimeData>
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"

#include "DockSceneTree/SceneTreeModel.h"
#include "Scene/SceneSignals.h"

#include "MaterialEditor/MaterialAssignSystem.h"

#include "Classes/Selection/Selection.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"

// commands
#include "Commands2/ParticleEditorCommands.h"

//mime data
#include "Tools/MimeData/MimeDataHelper2.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

SceneTreeModel::SceneTreeModel(QObject* parent /*= 0*/)
    : QStandardItemModel(parent)
{
    SetScene(nullptr);
    QObject::connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(ItemChanged(QStandardItem*)));
}

SceneTreeModel::~SceneTreeModel()
{
    if (NULL != curScene)
    {
        curScene->Release();
        curScene = NULL;
    }
}

Qt::DropActions SceneTreeModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::LinkAction;
}

void SceneTreeModel::SetScene(SceneEditor2* scene)
{
    clear();
    setColumnCount(1);
    setHorizontalHeaderLabels(QStringList() << "Scene hierarchy");

    if (NULL != curScene)
    {
        curScene->Release();
    }

    curScene = scene;

    if (NULL != curScene)
    {
        curScene->Retain();
    }

    ResyncStructure(invisibleRootItem(), curScene);
    SetFilter(filterText); // Apply filter to new model
}

SceneEditor2* SceneTreeModel::GetScene() const
{
    return curScene;
}

void SceneTreeModel::SetSolid(const QModelIndex& index, bool solid)
{
    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(GetItem(index));

    if (NULL != entity)
    {
        entity->SetSolid(solid);
    }
}

bool SceneTreeModel::GetSolid(const QModelIndex& index) const
{
    bool ret = false;

    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(GetItem(index));
    if (NULL != entity)
    {
        ret = entity->GetSolid();
    }

    return ret;
}

void SceneTreeModel::SetLocked(const QModelIndex& index, bool locked)
{
    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(GetItem(index));

    if (NULL != entity)
    {
        entity->SetLocked(locked);
    }
}

bool SceneTreeModel::GetLocked(const QModelIndex& index) const
{
    bool ret = false;

    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(GetItem(index));
    if (NULL != entity)
    {
        ret = entity->GetLocked();
    }

    return ret;
}

QVector<QIcon> SceneTreeModel::GetCustomIcons(const QModelIndex& index) const
{
    QVector<QIcon> ret;
    SceneTreeItem* item = GetItem(index);

    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(item);
    if (NULL != entity)
    {
        if (entity->GetLocked())
        {
            ret.push_back(SharedIcon(":/QtIcons/locked.png"));
        }

        if (NULL != GetCamera(entity))
        {
            if (curScene->GetCurrentCamera() == GetCamera(entity))
            {
                ret.push_back(SharedIcon(":/QtIcons/eye.png"));
            }
        }
    }

    return ret;
}

int SceneTreeModel::GetCustomFlags(const QModelIndex& index) const
{
    int ret = 0;

    SceneTreeItem* item = GetItem(index);
    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(item);

    if (NULL != entity)
    {
        if (entity->GetName().find("editor.") != DAVA::String::npos)
        {
            ret |= CF_Disabled;
        }
    }
    else
    {
        DAVA::ParticleLayer* layer = SceneTreeItemParticleLayer::GetLayer(item);
        if (NULL != layer)
        {
            DAVA::Entity* emitter = SceneTreeItemEntity::GetEntity(GetItem(index.parent()));
            if (NULL != emitter)
            {
                DAVA::LodComponent* lodComp = GetLodComponent(emitter);
                if (NULL != lodComp)
                {
                    if (!layer->IsLodActive(lodComp->GetCurrentLod()))
                    {
                        ret |= CF_Invisible;
                    }
                }
            }
        }
    }

    return ret;
}

QModelIndex SceneTreeModel::GetIndex(Selectable::Object* object) const
{
    auto i = indexesCache.find(object);
    return (i == indexesCache.end()) ? QModelIndex() : i->second;
}

SceneTreeItem* SceneTreeModel::GetItem(const QModelIndex& index) const
{
    return (SceneTreeItem*)itemFromIndex(index);
}

Qt::DropActions SceneTreeModel::supportedDropActions() const
{
    return (Qt::LinkAction | Qt::MoveAction | Qt::CopyAction);
}

QMimeData* SceneTreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* ret = NULL;

    if (indexes.size() > 0)
    {
        if (AreSameType(indexes))
        {
            SceneTreeItem* firstItem = GetItem(indexes.at(0));
            if (NULL != firstItem)
            {
                switch (firstItem->ItemType())
                {
                case SceneTreeItem::EIT_Entity:
                {
                    QVector<DAVA::Entity*> data;
                    foreach (QModelIndex index, indexes)
                    {
                        data.push_back(SceneTreeItemEntity::GetEntity(GetItem(index)));
                    }

                    ret = MimeDataHelper2<DAVA::Entity>::EncodeMimeData(data);
                }
                break;
                case SceneTreeItem::EIT_Emitter:
                {
                    QVector<DAVA::ParticleEmitterInstance*> data;
                    foreach (QModelIndex index, indexes)
                    {
                        data.push_back(SceneTreeItemParticleEmitter::GetEmitterInstanceStrict(GetItem(index)));
                    }

                    ret = MimeDataHelper2<DAVA::ParticleEmitterInstance>::EncodeMimeData(data);
                }
                break;
                case SceneTreeItem::EIT_Layer:
                {
                    QVector<DAVA::ParticleLayer*> data;
                    foreach (QModelIndex index, indexes)
                    {
                        data.push_back(SceneTreeItemParticleLayer::GetLayer(GetItem(index)));
                    }

                    ret = MimeDataHelper2<DAVA::ParticleLayer>::EncodeMimeData(data);
                }
                break;
                case SceneTreeItem::EIT_Force:
                {
                    QVector<DAVA::ParticleForce*> data;
                    foreach (QModelIndex index, indexes)
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
    types << MimeDataHelper2<DAVA::ParticleEmitterInstance>::GetMimeType();
    types << MimeDataHelper2<DAVA::ParticleLayer>::GetMimeType();
    types << MimeDataHelper2<DAVA::ParticleForce>::GetMimeType();

    return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    bool ret = false;

    SceneTreeItem* parentItem = GetItem(parent);
    SceneTreeItem* beforeItem = GetItem(index(row, column, parent));

    int dropType = GetDropType(data);
    switch (dropType)
    {
    case DropingEntity:
    {
        DAVA::Entity* parentEntity = SceneTreeItemEntity::GetEntity(parentItem);
        DAVA::Entity* beforeEntity = SceneTreeItemEntity::GetEntity(beforeItem);

        if (NULL == parentEntity)
        {
            parentEntity = curScene;
        }

        QVector<DAVA::Entity*> entitiesV = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(data);
        if (entitiesV.size() > 0)
        {
            SelectableGroup objects;
            for (int i = 0; i < entitiesV.size(); ++i)
            {
                objects.Add(entitiesV[i], DAVA::AABBox3());
            }

            curScene->structureSystem->Move(objects, parentEntity, beforeEntity);
            ret = true;
        }
    }
    break;

    case DropingEmitter:
    {
        DAVA::ParticleEffectComponent* effect = GetEffectComponent(SceneTreeItemEntity::GetEntity(parentItem));
        QVector<DAVA::ParticleEmitterInstance*> emittersV = MimeDataHelper2<DAVA::ParticleEmitterInstance>::DecodeMimeData(data);
        if (NULL != effect && emittersV.size() > 0)
        {
            DAVA::Vector<DAVA::ParticleEmitterInstance*> emittersGroup;
            DAVA::Vector<DAVA::ParticleEffectComponent*> effectsGroup;
            emittersGroup.reserve(emittersV.size());
            effectsGroup.reserve(emittersV.size());
            for (int i = 0; i < emittersV.size(); ++i)
            {
                emittersGroup.push_back(static_cast<DAVA::ParticleEmitterInstance*>(emittersV[i]));
                QModelIndex emitterIndex = GetIndex((DAVA::ParticleEmitter*)emittersV[i]);
                DAVA::ParticleEffectComponent* oldEffect = GetEffectComponent(SceneTreeItemEntity::GetEntity(GetItem(emitterIndex.parent())));
                effectsGroup.push_back(oldEffect);
            }

            curScene->structureSystem->MoveEmitter(emittersGroup, effectsGroup, effect, row);
            ret = true;
        }
    }
    break;

    case DropingLayer:
    {
        DAVA::ParticleEmitterInstance* emitter = NULL;

        if ((parentItem->ItemType() == SceneTreeItem::EIT_Emitter) || (parentItem->ItemType() == SceneTreeItem::EIT_InnerEmitter))
        {
            emitter = static_cast<SceneTreeItemParticleEmitter*>(parentItem)->GetEmitterInstance();
        }

        QVector<DAVA::ParticleLayer*> layersV = MimeDataHelper2<DAVA::ParticleLayer>::DecodeMimeData(data);
        if (NULL != emitter && layersV.size() > 0)
        {
            DAVA::ParticleLayer* beforeLayer = NULL;

            if (row >= 0 && row < (int)emitter->GetEmitter()->layers.size())
            {
                beforeLayer = emitter->GetEmitter()->layers[row];
            }

            DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
            DAVA::Vector<DAVA::ParticleEmitterInstance*> emittersGroup;
            layersGroup.reserve(layersV.size());
            emittersGroup.reserve(layersV.size());
            for (int i = 0; i < layersV.size(); ++i)
            {
                layersGroup.push_back((DAVA::ParticleLayer*)layersV[i]);
                QModelIndex emitterIndex = GetIndex((DAVA::ParticleLayer*)layersV[i]);
                DAVA::ParticleEmitterInstance* oldEmitter = SceneTreeItemParticleEmitter::GetEmitterInstance(GetItem(emitterIndex.parent()));
                emittersGroup.push_back(oldEmitter);
            }

            curScene->structureSystem->MoveLayer(layersGroup, emittersGroup, emitter, beforeLayer);
            ret = true;
        }
    }
    break;
    case DropingForce:
    {
        DAVA::ParticleLayer* newLayer = SceneTreeItemParticleLayer::GetLayer(parentItem);

        QVector<DAVA::ParticleForce*> forcesV = MimeDataHelper2<DAVA::ParticleForce>::DecodeMimeData(data);
        if (NULL != newLayer && forcesV.size())
        {
            DAVA::Vector<DAVA::ParticleForce*> forcesGroup;
            forcesGroup.reserve(forcesV.size());

            DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
            layersGroup.reserve(forcesV.size());

            for (int i = 0; i < forcesV.size(); ++i)
            {
                QModelIndex forceIndex = GetIndex((DAVA::ParticleForce*)forcesV[i]);
                DAVA::ParticleLayer* oldLayer = SceneTreeItemParticleLayer::GetLayer(GetItem(forceIndex.parent()));

                forcesGroup.push_back((DAVA::ParticleForce*)forcesV[i]);
                layersGroup.push_back(oldLayer);
            }

            curScene->structureSystem->MoveForce(forcesGroup, layersGroup, newLayer);
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

bool SceneTreeModel::DropCanBeAccepted(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
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
        if (NULL != parentItem && parentItem->ItemType() != SceneTreeItem::EIT_Entity)
        {
            ret = false;
        }
        // don't accept drops inside disabled (by custom flags) entity
        else if (GetCustomFlags(parent) & CF_Disabled)
        {
            ret = false;
        }
        else
        {
            // go thought entities and check them
            QVector<DAVA::Entity*> entities = MimeDataHelper2<DAVA::Entity>::DecodeMimeData(data);
            if (entities.size() > 0)
            {
                DAVA::Entity* targetEntity = SceneTreeItemEntity::GetEntity(parentItem);

                for (int i = 0; i < entities.size(); ++i)
                {
                    DAVA::Entity* entity = (DAVA::Entity*)entities[i];
                    QModelIndex entityIndex = GetIndex(entity);

                    // 1. we don't accept drops if it has locked items
                    if (NULL != entity && entity->GetLocked())
                    {
                        ret = false;
                        break;
                    }

                    // 2. or it has entity with CF_Disabled flag
                    if (GetCustomFlags(entityIndex) & CF_Disabled)
                    {
                        ret = false;
                        break;
                    }

                    // 3. or this is self-drop
                    if (targetEntity == entity || // dropping into
                        entityIndex == index(row, column, parent) || // dropping above
                        entityIndex == index(row - 1, column, parent)) // dropping below
                    {
                        ret = false;
                        break;
                    }

                    // 4. or we are dropping last element to the bottom of the list
                    if (NULL == targetEntity && row == -1 && // dropping to be bottom of the list
                        !entityIndex.parent().isValid() && // no parent
                        entityIndex.row() == (rowCount(entityIndex.parent()) - 1)) // dropped item is already bottom
                    {
                        ret = false;
                        break;
                    }

                    // 5. or we are dropping waypoint outside of its path
                    if (GetWaypointComponent(entity))
                    {
                        if (entity->GetParent() != targetEntity)
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
        if (NULL != parentItem)
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
        if (NULL != parentItem)
        {
            if ((parentItem->ItemType() == SceneTreeItem::EIT_Emitter) || (parentItem->ItemType() == SceneTreeItem::EIT_InnerEmitter))
            {
                ret = true;
            }
        }
    }
    break;
    case DropingForce:
    {
        // accept force to be dropped only to particle layer
        if (NULL != parentItem && parentItem->ItemType() == SceneTreeItem::EIT_Layer)
        {
            // accept only add (no insertion)
            if (-1 == row && -1 == column)
            {
                ret = true;
            }
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

void SceneTreeModel::ItemChanged(QStandardItem* item)
{
    SceneTreeItem* treeItem = dynamic_cast<SceneTreeItem*>(item);
    if (NULL != treeItem)
    {
        if (treeItem->ItemType() == SceneTreeItem::EIT_Layer)
        {
            bool isLayerEnabled = (item->checkState() == Qt::Checked);
            SceneTreeItemParticleLayer* itemLayer = (SceneTreeItemParticleLayer*)treeItem;

            curScene->Exec(std::unique_ptr<DAVA::Command>(new CommandUpdateParticleLayerEnabled(itemLayer->GetLayer(), isLayerEnabled)));
            curScene->MarkAsChanged();
        }
    }
}

void SceneTreeModel::ResyncStructure(QStandardItem* item, DAVA::Entity* entity)
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
    SceneTreeItem* item = GetItem(_index);
    const QString& name = item->ItemName();
    DAVA::uint32 id = 0xFFFFFFFF;

    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(item);
    if (nullptr != entity)
    {
        id = entity->GetID();
    }

    if (!item->IsAcceptedByFilter())
    {
        const bool match = (text.isEmpty() || name.contains(text, Qt::CaseInsensitive) || text == QString::number(id));

        item->SetAcceptByFilter(match);
        item->SetHighlight(match);

        if (match)
        {
            QModelIndex p = _index.parent();
            while (p.isValid())
            {
                SceneTreeItem* parentItem = GetItem(p);
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
    for (int i = 0; i < n; i++)
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
        SceneTreeItem* item = GetItem(_index);
        item->SetAcceptByFilter(false);
        item->SetHighlight(false);
        ResetFilter(_index);
    }
}

void SceneTreeModel::RebuildIndexesCache()
{
    indexesCache.clear();

    for (int i = 0; i < rowCount(); ++i)
    {
        AddIndexesCache(GetItem(index(i, 0)));
    }
}

void SceneTreeModel::AddIndexesCache(SceneTreeItem* item)
{
    indexesCache.emplace(item->GetItemObject(), item->index());
    for (int i = 0; i < item->rowCount(); ++i)
    {
        AddIndexesCache((SceneTreeItem*)item->child(i));
    }
}

bool SceneTreeModel::AreSameType(const QModelIndexList& indexes) const
{
    bool ret = true;

    SceneTreeItem* firstItem = GetItem(indexes.at(0));

    // check if all items are same type
    if (NULL != firstItem)
    {
        foreach (QModelIndex index, indexes)
        {
            SceneTreeItem* item = GetItem(index);
            if (NULL != item && firstItem->ItemType() != item->ItemType())
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}

int SceneTreeModel::GetDropType(const QMimeData* data) const
{
    int ret = DropingUnknown;

    if (NULL != data)
    {
        if (MimeDataHelper2<DAVA::Entity>::IsValid(data))
        {
            ret = DropingEntity;
        }
        else if (MimeDataHelper2<DAVA::ParticleEmitterInstance>::IsValid(data))
        {
            ret = DropingEmitter;
        }
        else if (MimeDataHelper2<DAVA::ParticleLayer>::IsValid(data))
        {
            ret = DropingLayer;
        }
        else if (MimeDataHelper2<DAVA::ParticleForce>::IsValid(data))
        {
            ret = DropingForce;
        }
    }

    return ret;
}

Qt::ItemFlags SceneTreeModel::flags(const QModelIndex& index) const
{
    const Qt::ItemFlags f = QStandardItemModel::flags(index);
    DAVA::Entity* entity = SceneTreeItemEntity::GetEntity(GetItem(index));
    if (nullptr != entity)
    {
        if (Selection::IsEntitySelectable(entity) == false)
        {
            return (f & ~Qt::ItemIsSelectable);
        }
    }

    return f;
}

QVariant SceneTreeModel::data(const QModelIndex& _index, int role) const
{
    switch (role)
    {
    case Qt::BackgroundRole:
    {
        SceneTreeItem* item = GetItem(_index);
        DAVA::ParticleEmitterInstance* emitter = SceneTreeItemParticleEmitter::GetEmitterInstanceStrict(item);
        if (nullptr != emitter && emitter->GetEmitter()->shortEffect)
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

SceneTreeFilteringModel::SceneTreeFilteringModel(SceneTreeModel* _treeModel, QObject* parent /* = NULL */)
    : QSortFilterProxyModel(parent)
    , treeModel(_treeModel)
{
    setSourceModel(treeModel);
}

bool SceneTreeFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!treeModel->IsFilterSet())
        return true;

    const QModelIndex& _index = treeModel->index(sourceRow, 0, sourceParent);
    SceneTreeItem* item = treeModel->GetItem(_index);
    DVASSERT(item);

    return item->IsAcceptedByFilter();
}

QVariant SceneTreeFilteringModel::data(const QModelIndex& _index, int role) const
{
    if (!treeModel->IsFilterSet())
        return QSortFilterProxyModel::data(_index, role);

    switch (role)
    {
    case Qt::BackgroundRole:
    {
        SceneTreeItem* item = treeModel->GetItem(mapToSource(_index));
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
