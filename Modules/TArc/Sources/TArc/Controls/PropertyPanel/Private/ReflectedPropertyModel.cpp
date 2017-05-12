#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"

#include <Engine/PlatformApi.h>
#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <Base/TemplateHelpers.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

#include <QPalette>

//#define REPORT_UPDATE_TIME

namespace DAVA
{
namespace TArc
{
const char* expandedItemsKey = "expandedItems";

class ReflectedPropertyModel::InsertGuard final
{
public:
    InsertGuard(ReflectedPropertyModel* model_, ReflectedPropertyItem* item, int first, int last)
        : model(model_)
    {
        QModelIndex index = model->MapItem(item);
        model->beginInsertRows(index, first, last);
    }

    ~InsertGuard()
    {
        model->endInsertRows();
    }

    ReflectedPropertyModel* model;
    bool sessionOpened = false;
};

class ReflectedPropertyModel::RemoveGuard final
{
public:
    RemoveGuard(ReflectedPropertyModel* model_, ReflectedPropertyItem* item)
        : model(model_)
    {
        if (item->GetChildCount() > 1)
        {
            return;
        }

        sessionOpened = true;
        QModelIndex index = model->MapItem(item->parent);
        model->beginRemoveRows(index, item->position, item->position);
    }

    ~RemoveGuard()
    {
        if (sessionOpened == true)
        {
            model->endRemoveRows();
        }
    }

    ReflectedPropertyModel* model;
    bool sessionOpened = false;
};

ReflectedPropertyModel::ReflectedPropertyModel(WindowKey wndKey_, ContextAccessor* accessor_, OperationInvoker* invoker_, UI* ui_)
    : wndKey(wndKey_)
    , accessor(accessor_)
    , invoker(invoker_)
    , ui(ui_)
    , expandedItems(FastName("Root"))
{
    rootItem.reset(new ReflectedPropertyItem(this, std::make_unique<EmptyComponentValue>()));

    RegisterExtension(ChildCreatorExtension::CreateDummy());
    RegisterExtension(EditorComponentExtension::CreateDummy());
    RegisterExtension(ModifyExtension::CreateDummy());

    RegisterExtension(std::make_shared<DefaultChildCheatorExtension>());
    RegisterExtension(std::make_shared<SubPropertyValueChildCreator>());

    RegisterExtension(std::make_shared<DefaultEditorComponentExtension>(ui));
    RegisterExtension(std::make_shared<SubPropertyEditorCreator>());

    childCreator.nodeCreated.Connect(this, &ReflectedPropertyModel::OnChildAdded);
    childCreator.nodeRemoved.Connect(this, &ReflectedPropertyModel::OnChildRemoved);
    childCreator.dataChanged.Connect(this, &ReflectedPropertyModel::OnDataChange);
    childCreator.nodeCreated.Connect(&favoritesController, &FavoritesController::OnChildAdded);
    childCreator.nodeRemoved.Connect(&favoritesController, &FavoritesController::OnChildRemoved);

    favoritesController.favoritedCreated.Connect(this, &ReflectedPropertyModel::OnFavoritedAdded);
    favoritesController.favoriteDeleted.Connect(this, &ReflectedPropertyModel::OnFavoritedRemoved);
}

ReflectedPropertyModel::~ReflectedPropertyModel()
{
    SetObjects(Vector<DAVA::Reflection>());
    rootItem.reset();
}

//////////////////////////////////////
//       QAbstractItemModel         //
//////////////////////////////////////

int ReflectedPropertyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
    {
        return 0;
    }

    ReflectedPropertyItem* item = MapItem(parent);
    DVASSERT(item != nullptr);
    return item->GetChildCount();
}

int ReflectedPropertyModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ReflectedPropertyModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        ReflectedPropertyItem* item = MapItem(index);
        if (item != nullptr)
        {
            return item->GetPropertyName();
        }
    }

    return QVariant();
}

bool ReflectedPropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    DVASSERT(false);
    return false;
}

QVariant ReflectedPropertyModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const
{
    if (role != Qt::DisplayRole || orientation == Qt::Vertical)
    {
        return QVariant();
    }

    return section == 0 ? QStringLiteral("Property") : QStringLiteral("Value");
}

Qt::ItemFlags ReflectedPropertyModel::flags(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ReflectedPropertyItem* item = MapItem(index);
    if (item != nullptr)
    {
        if (index.column() == 1 || item->value->IsSpannedControl() == true)
        {
            std::shared_ptr<const PropertyNode> node = item->GetPropertyNode(0);
            if (!node->field.ref.IsReadonly())
            {
                flags |= Qt::ItemIsEditable;
            }
        }
    }

    return flags;
}

QModelIndex ReflectedPropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        ReflectedPropertyItem* item = MapItem(parent);
        if (item != nullptr && row < item->GetChildCount())
            return createIndex(row, column, item);

        return QModelIndex();
    }

    return MapItem(rootItem.get());
}

QModelIndex ReflectedPropertyModel::parent(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    ReflectedPropertyItem* item = reinterpret_cast<ReflectedPropertyItem*>(index.internalPointer());
    if (item == nullptr)
    {
        return QModelIndex();
    }

    if (item == rootItem.get())
    {
        return MapItem(rootItem.get());
    }

    return createIndex(item->position, 0, item->parent);
}

//////////////////////////////////////
//       QAbstractItemModel         //
//////////////////////////////////////

void ReflectedPropertyModel::Update()
{
    int64 start = SystemTimer::GetMs();
    Update(rootItem.get());
    fastWrappersProcessor.Sync();
    wrappersProcessor.Sync();
#if defined(REPORT_UPDATE_TIME)
    Logger::Debug(" === ReflectedPropertyModel::Update : %d for %d objects ===", static_cast<int32>(SystemTimer::GetMs() - start), rootItem->GetPropertyNodesCount());
#endif
}

void ReflectedPropertyModel::Update(ReflectedPropertyItem* item)
{
    if (item->IsFavorite())
    {
        return;
    }

    int32 propertyNodesCount = item->GetPropertyNodesCount();
    for (int32 i = 0; i < propertyNodesCount; ++i)
    {
        childCreator.UpdateSubTree(item->GetPropertyNode(i));
    }

    for (const std::unique_ptr<ReflectedPropertyItem>& child : item->children)
    {
        Update(child.get());
    }
}

void ReflectedPropertyModel::UpdateFastImpl(ReflectedPropertyItem* item)
{
    if (item->IsFavorite())
    {
        return;
    }

    if (item->GetPropertyNodesCount() == 0)
    {
        return;
    }

    if (nullptr != item->GetPropertyNode(0)->field.ref.GetMeta<M::FrequentlyChangedValue>())
    {
        Update(item);
    }

    for (int32 i = 0; i < item->GetChildCount(); ++i)
    {
        UpdateFastImpl(item->GetChild(i));
    }
}

DataWrappersProcessor* ReflectedPropertyModel::GetWrappersProcessor(const std::shared_ptr<PropertyNode>& node)
{
    if (nullptr != node->field.ref.GetMeta<M::FrequentlyChangedValue>())
    {
        return &fastWrappersProcessor;
    }

    return &wrappersProcessor;
}

void ReflectedPropertyModel::UpdateFast()
{
    int64 start = SystemTimer::GetMs();
    UpdateFastImpl(rootItem.get());
    fastWrappersProcessor.Sync();
#if defined(REPORT_UPDATE_TIME)
    Logger::Debug(" === ReflectedPropertyModel::UpdateFast : %d ===", static_cast<int32>(SystemTimer::GetMs() - start));
#endif
}

void ReflectedPropertyModel::SetObjects(Vector<Reflection> objects)
{
    wrappersProcessor.Shoutdown();
    fastWrappersProcessor.Shoutdown();

    int childCount = static_cast<int>(rootItem->GetChildCount());
    if (childCount != 0)
    {
        nodeToItem.clear();
        nodeToFavorite.clear();
        beginResetModel();
        rootItem->RemoveChildren();
        favoritesController.Reset();
        endResetModel();

        childCreator.Clear();
    }
    rootItem->RemovePropertyNodes();

    bool rootSet = false;
    for (Reflection& obj : objects)
    {
        Reflection::Field field(String("SelfRoot"), std::move(obj), nullptr);
        std::shared_ptr<PropertyNode> rootNode = childCreator.CreateRoot(std::move(field));
        nodeToItem.emplace(rootNode, rootItem.get());
        rootItem->AddPropertyNode(rootNode);
        if (rootSet == false)
        {
            favoritesController.SetModelRoot(rootNode);
            rootSet = true;
        }
    }
    objects.clear();

    Update();
}

void ReflectedPropertyModel::OnChildAdded(const std::shared_ptr<PropertyNode>& parent, const std::shared_ptr<PropertyNode>& node)
{
    auto iter = nodeToItem.find(parent);
    DVASSERT(iter != nodeToItem.end());

    ReflectedPropertyItem* parentItem = iter->second;

    ReflectedPropertyItem* childItem = LookUpItem(node, node->BuildID(), parentItem->children);
    if (childItem != nullptr)
    {
        childItem->AddPropertyNode(node);
    }
    else
    {
        std::unique_ptr<BaseComponentValue> valueComponent = GetExtensionChain<EditorComponentExtension>()->GetEditor(node);
        valueComponent->Init(this);

        int32 childPosition = parentItem->LookupChildPosition(node->sortKey);
        InsertGuard guard(this, parentItem, childPosition, childPosition);
        childItem = parentItem->CreateChild(std::move(valueComponent), childPosition, node->sortKey);
        childItem->AddPropertyNode(node);
    }

    auto newNode = nodeToItem.emplace(node, childItem);
    DVASSERT(newNode.second);
}

void ReflectedPropertyModel::OnChildRemoved(const std::shared_ptr<PropertyNode>& node)
{
    auto iter = nodeToItem.find(node);
    DVASSERT(iter != nodeToItem.end());
    ReflectedPropertyItem* item = iter->second;

    RemoveGuard guard(this, item);
    item->RemovePropertyNode(node);
    bool needRemove = item->GetPropertyNodesCount() == 0;
    if (needRemove)
    {
        item->parent->RemoveChild(item->position);
    }

    if (needRemove == true)
    {
        nodeToItem.erase(node);
    }
}

void ReflectedPropertyModel::OnDataChange(const std::shared_ptr<PropertyNode>& node)
{
    auto emitSignal = [&](const QModelIndex& index)
    {
        QModelIndex valueColumnIndex = index.sibling(index.row(), 1);
        emit dataChanged(index, valueColumnIndex);
    };
    {
        auto iter = nodeToItem.find(node);
        DVASSERT(iter != nodeToItem.end());
        emitSignal(MapItem(iter->second));
    }

    {
        auto iter = nodeToFavorite.find(node);
        if (iter != nodeToFavorite.end())
        {
            emitSignal(MapItem(iter->second));
        }
    }
}

void ReflectedPropertyModel::OnFavoritedAdded(const std::shared_ptr<PropertyNode>& parent, const std::shared_ptr<PropertyNode>& node, const DAVA::String& id, int32 sortKey, bool isRoot)
{
    auto itemIter = nodeToItem.find(node);
    if (itemIter != nodeToItem.end() && isRoot == true)
    {
        itemIter->second->SetFavorited(true);
    }

    auto favoriteParentIter = nodeToFavorite.find(parent);
    if (favoriteParentIter == nodeToFavorite.end())
    {
        favoriteParentIter = nodeToItem.find(parent);
        DVASSERT(favoriteParentIter != nodeToItem.end());
    }

    ReflectedPropertyItem* parentItem = favoriteParentIter->second;

    ReflectedPropertyItem* childItem = LookUpItem(node, id, parentItem->children);
    if (childItem != nullptr)
    {
        childItem->AddPropertyNode(node, FastName(id));
    }
    else
    {
        std::unique_ptr<BaseComponentValue> valueComponent = GetExtensionChain<EditorComponentExtension>()->GetEditor(node);
        valueComponent->Init(this);

        int32 childPosition = parentItem->LookupChildPosition(sortKey);
        InsertGuard guard(this, parentItem, childPosition, childPosition);
        childItem = parentItem->CreateChild(std::move(valueComponent), childPosition, node->sortKey);
        childItem->SetFavorite(isRoot);
        childItem->AddPropertyNode(node, FastName(id));
    }

    auto newNode = nodeToFavorite.emplace(node, childItem);
    DVASSERT(newNode.second);
}

void ReflectedPropertyModel::OnFavoritedRemoved(const std::shared_ptr<PropertyNode>& node, bool unfavorited)
{
    auto fvIter = nodeToFavorite.find(node);
    DVASSERT(fvIter != nodeToFavorite.end());

    ReflectedPropertyItem* item = fvIter->second;

    RemoveGuard guard(this, item);
    item->RemovePropertyNode(node);
    if (item->GetPropertyNodesCount() == 0)
    {
        item->parent->RemoveChild(item->position);
    }

    if (unfavorited == true)
    {
        auto itemIter = nodeToItem.find(node);
        if (itemIter != nodeToItem.end())
        {
            itemIter->second->SetFavorited(false);
        }
    }
    nodeToFavorite.erase(node);
}

ReflectedPropertyItem* ReflectedPropertyModel::MapItem(const QModelIndex& item) const
{
    if (item.isValid() == false)
    {
        return nullptr;
    }

    ReflectedPropertyItem* p = reinterpret_cast<ReflectedPropertyItem*>(item.internalPointer());
    if (p == nullptr)
    {
        return rootItem.get();
    }
    return p->GetChild(item.row());
}

QModelIndex ReflectedPropertyModel::MapItem(ReflectedPropertyItem* item) const
{
    DVASSERT(item != nullptr);
    if (item == rootItem.get())
    {
        return createIndex(0, 0, nullptr);
    }

    return createIndex(item->position, 0, item->parent);
}

void ReflectedPropertyModel::RegisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    const Type* extType = extension->GetType();
    if (extType == Type::Instance<ChildCreatorExtension>())
    {
        childCreator.RegisterExtension(PolymorphCast<ChildCreatorExtension>(extension));
        return;
    }

    auto iter = extensions.find(extType);
    if (iter == extensions.end())
    {
        extensions.emplace(extType, extension);
        return;
    }

    iter->second = ExtensionChain::AddExtension(iter->second, extension);
}

void ReflectedPropertyModel::UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    const Type* extType = extension->GetType();
    if (extType == Type::Instance<ChildCreatorExtension>())
    {
        childCreator.UnregisterExtension(PolymorphCast<ChildCreatorExtension>(extension));
        return;
    }

    auto iter = extensions.find(extType);
    if (iter == extensions.end())
    {
        /// you do something wrong
        DVASSERT(false);
        return;
    }

    iter->second = ExtensionChain::RemoveExtension(iter->second, extension);
}

DAVA::TArc::BaseComponentValue* ReflectedPropertyModel::GetComponentValue(const QModelIndex& index) const
{
    ReflectedPropertyItem* item = MapItem(index);
    DVASSERT(item != nullptr);
    return item->value.get();
}

void ReflectedPropertyModel::SetExpanded(bool expanded, const QModelIndex& index)
{
    List<FastName> reflectedPath;
    QModelIndex currentIndex = index;
    while (currentIndex.isValid())
    {
        ReflectedPropertyItem* item = MapItem(currentIndex);
        reflectedPath.push_front(item->GetID());
        currentIndex = currentIndex.parent();
    }

    if (expanded == true)
    {
        expandedItems.AddLeaf(std::move(reflectedPath));
    }
    else
    {
        expandedItems.RemoveLeaf(std::move(reflectedPath));
    }
}

QModelIndexList ReflectedPropertyModel::GetExpandedList(const QModelIndex& rootIndex) const
{
    QModelIndexList result;
    if (expandedItems.PushRoot(rootItem->GetID()))
    {
        GetExpandedListImpl(result, rootItem.get());
        expandedItems.PopRoot();
    }
    return result;
}

QModelIndexList ReflectedPropertyModel::GetExpandedChildren(const QModelIndex& index) const
{
    List<FastName> reflectedPath;
    QModelIndex currentIndex = index;
    while (currentIndex.isValid())
    {
        ReflectedPropertyItem* item = MapItem(currentIndex);
        reflectedPath.push_front(item->GetID());
        currentIndex = currentIndex.parent();
    }

    for (const FastName& name : reflectedPath)
    {
        bool result = expandedItems.PushRoot(name);
        DVASSERT(result);
    }
    QModelIndexList result;
    GetExpandedListImpl(result, MapItem(index));
    for (size_t i = 0; i < reflectedPath.size(); ++i)
    {
        expandedItems.PopRoot();
    }

    return result;
}

void ReflectedPropertyModel::SaveState(PropertiesItem& propertyRoot) const
{
    PropertiesItem expandedItemsSettings = propertyRoot.CreateSubHolder(expandedItemsKey);
    expandedItems.Save(expandedItemsSettings);

    favoritesController.Save(propertyRoot);
}

void ReflectedPropertyModel::LoadState(const PropertiesItem& propertyRoot)
{
    PropertiesItem expandedItemsSettings = propertyRoot.CreateSubHolder(expandedItemsKey);
    expandedItems.Load(expandedItemsSettings);

    favoritesController.Load(propertyRoot);
}

bool ReflectedPropertyModel::IsFavorite(const QModelIndex& index) const
{
    ReflectedPropertyItem* item = MapItem(index);
    if (item != nullptr)
    {
        return item->IsFavorite() || item->IsFavorited();
    }

    return false;
}

bool ReflectedPropertyModel::IsInFavoriteHierarchy(const QModelIndex& index) const
{
    ReflectedPropertyItem* item = MapItem(index);
    bool isFavorite = false;
    while (isFavorite == false && item != nullptr)
    {
        isFavorite |= item->IsFavorite();
        isFavorite |= item->IsFavorited();
        item = item->parent;
    }

    return isFavorite;
}

void ReflectedPropertyModel::AddFavorite(const QModelIndex& index)
{
    ReflectedPropertyItem* item = MapItem(index);
    DVASSERT(item->IsFavorited() == false);
    DVASSERT(item->IsFavorite() == false);
    DVASSERT(item->GetPropertyNodesCount() > 0);

    favoritesController.AddFavorite(item->GetPropertyNode(0));
}

void ReflectedPropertyModel::RemoveFavorite(const QModelIndex& index)
{
    ReflectedPropertyItem* item = MapItem(index);
    if (item->GetPropertyNode(0)->propertyType == PropertyNode::FavoritesProperty)
    {
        favoritesController.ClearFavorites();
        return;
    }

    DVASSERT(item->IsFavorited() == true || item->IsFavorite() == true);
    DVASSERT(item->GetPropertyNodesCount() > 0);

    favoritesController.RemoveFavorite(item->GetPropertyNode(0));
}

bool ReflectedPropertyModel::IsDeveloperMode() const
{
    DVASSERT(extensions.empty() == false);
    return extensions.begin()->second->IsDeveloperMode();
}

void ReflectedPropertyModel::SetDeveloperMode(bool isDevMode)
{
    childCreator.SetDevMode(isDevMode);
    for (auto& iter : extensions)
    {
        iter.second->SetDevelopertMode(isDevMode);
    }
}

QModelIndex ReflectedPropertyModel::GetRegularRootIndex() const
{
    return MapItem(rootItem.get());
}

QModelIndex ReflectedPropertyModel::GetFavoriteRootIndex() const
{
    if (rootItem->GetChildCount() < 1)
    {
        return QModelIndex();
    }

    ReflectedPropertyItem* item = rootItem->GetChild(0);
    if (item->GetPropertyNode(0)->propertyType == PropertyNode::FavoritesProperty)
    {
        return MapItem(item);
    }

    return QModelIndex();
}

Vector<FastName> ReflectedPropertyModel::GetIndexPath(const QModelIndex& index) const
{
    Vector<FastName> path;
    if (index.isValid() == false)
    {
        return path;
    }

    path.reserve(8);
    ReflectedPropertyItem* item = MapItem(index);
    while (item != nullptr)
    {
        path.push_back(item->value->GetID());
        item = item->parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

QModelIndex ReflectedPropertyModel::LookIndex(const Vector<FastName>& path) const
{
    ReflectedPropertyItem* item = rootItem.get();
    if (path.empty() || path.front() != item->value->GetID())
    {
        return QModelIndex();
    }

    for (size_t i = 1; i < path.size(); ++i)
    {
        if (item == nullptr)
        {
            return QModelIndex();
        }

        FastName itemID = path[i];
        size_t childCount = item->children.size();
        ReflectedPropertyItem* nextItem = nullptr;
        for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
        {
            ReflectedPropertyItem* currentItem = item->children[childIndex].get();
            if (itemID == currentItem->value->GetID())
            {
                nextItem = currentItem;
                break;
            }
        }

        item = nextItem;
    }

    if (item == nullptr)
    {
        return QModelIndex();
    }

    return MapItem(item);
}

void ReflectedPropertyModel::GetExpandedListImpl(QModelIndexList& list, ReflectedPropertyItem* item) const
{
    if (item == nullptr)
    {
        return;
    }

    int32 childCount = item->GetChildCount();
    for (int32 i = 0; i < childCount; ++i)
    {
        ReflectedPropertyItem* child = item->GetChild(i);
        FastName propertyName = FastName(child->GetID());
        if (expandedItems.HasChildInCurrentRoot(propertyName))
        {
            list << MapItem(child);
            expandedItems.PushRoot(propertyName);
            GetExpandedListImpl(list, child);
            expandedItems.PopRoot();
        }
    }
}

ReflectedPropertyItem* ReflectedPropertyModel::LookUpItem(const std::shared_ptr<PropertyNode>& node, const DAVA::String& lookupID, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items)
{
    DVASSERT(node->field.ref.IsValid());

    ReflectedPropertyItem* result = nullptr;
    FastName nodeID = FastName(lookupID);

    for (const std::unique_ptr<ReflectedPropertyItem>& item : items)
    {
        DVASSERT(item->GetPropertyNodesCount() > 0);
        if (item->GetID() == nodeID)
        {
            result = item.get();
            break;
        }
    }

    return result;
}

} // namespace TArc
} // namespace DAVA
