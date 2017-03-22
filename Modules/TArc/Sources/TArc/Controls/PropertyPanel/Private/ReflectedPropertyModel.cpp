#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/KeyedArchiveChildCreator.h"
#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"

#include <Engine/PlatformApi.h>
#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <Base/TemplateHelpers.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

#include <QPalette>

namespace DAVA
{
namespace TArc
{
struct FavoriteItemValue
{
    DAVA_REFLECTION(FavoriteItemValue);
};

DAVA_REFLECTION_IMPL(FavoriteItemValue)
{
    ReflectionRegistrator<FavoriteItemValue>::Begin()
    .End();
}

const char* expandedItemsKey = "expandedItems";
const char* favoritedItemsKey = "favoritedItems";
const char* favoritedCountKey = "favoritesCount";
const char* favoritedElementKey = "favoritesElement_%u";
const char* isFavoritesViewOnly = "isFavoritesViewOnly";

class ReflectedPropertyModel::InsertGuard
{
public:
    InsertGuard(ReflectedPropertyModel* model_, ReflectedPropertyItem* item, int first, int last)
        : model(model_)
    {
        // When mode works in FavoritesOnly mode, view doesn't know about regular part of model data
        // so we should not to open begin\end InsertRows session
        if (model->IsFavoriteOnly())
        {
            return;
        }

        sessionOpened = true;
        QModelIndex index = model->MapItem(item);
        model->beginInsertRows(index, first, last);
    }

    ~InsertGuard()
    {
        if (sessionOpened == true)
        {
            model->endInsertRows();
        }
    }

    ReflectedPropertyModel* model;
    bool sessionOpened = false;
};

class ReflectedPropertyModel::RemoveGuard
{
public:
    RemoveGuard(ReflectedPropertyModel* model_, ReflectedPropertyItem* item, bool forceOpenSession)
        : model(model_)
    {
        // When mode works in FavoritesOnly mode, view doesn't know about regular part of model data
        // so we should not to open begin\end RemoveRows session
        // but if there is a favorite item has been removed we should remove it from view.
        if (forceOpenSession == false)
        {
            if (model->IsFavoriteOnly())
            {
                return;
            }

            if (item->GetChildCount() > 1)
            {
                return;
            }
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
    RegisterExtension(MergeValuesExtension::CreateDummy());
    RegisterExtension(EditorComponentExtension::CreateDummy());
    RegisterExtension(ModifyExtension::CreateDummy());

    RegisterExtension(std::make_shared<DefaultChildCheatorExtension>());
    RegisterExtension(std::make_shared<KeyedArchiveChildCreator>());
    RegisterExtension(std::make_shared<SubPropertyValueChildCreator>());

    RegisterExtension(std::make_shared<DefaultMergeValueExtension>());

    RegisterExtension(std::make_shared<DefaultEditorComponentExtension>(ui));
    RegisterExtension(std::make_shared<SubPropertyEditorCreator>());

    childCreator.nodeCreated.Connect(this, &ReflectedPropertyModel::ChildAdded);
    childCreator.nodeRemoved.Connect(this, &ReflectedPropertyModel::ChildRemoved);
}

ReflectedPropertyModel::~ReflectedPropertyModel()
{
    favorites.clear();
    SetObjects(Vector<DAVA::Reflection>());
    rootItem.reset();
}

//////////////////////////////////////
//       QAbstractItemModel         //
//////////////////////////////////////

int ReflectedPropertyModel::rowCount(const QModelIndex& parent) const
{
    ReflectedPropertyItem* item = MapItem(parent);
    if (item == nullptr)
    {
        return 0;
    }
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
    if (index.column() == 1)
    {
        ReflectedPropertyItem* item = MapItem(index);
        if (item != nullptr)
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

    ReflectedPropertyItem* root = GetSmartRoot();
    if (root == nullptr)
    {
        return QModelIndex();
    }

    return createIndex(row, column, root);
}

QModelIndex ReflectedPropertyModel::parent(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    ReflectedPropertyItem* item = reinterpret_cast<ReflectedPropertyItem*>(index.internalPointer());
    if (item == nullptr || item == GetSmartRoot())
    {
        return QModelIndex();
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
    Logger::Debug(" === ReflectedPropertyModel::Update : %d ===", static_cast<int32>(SystemTimer::GetMs() - start));
#endif

    RefreshFavoritesRoot();
}

void ReflectedPropertyModel::Update(ReflectedPropertyItem* item)
{
    if (item == favoritesItem)
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
    if (item == favoritesItem)
    {
        return;
    }

    if (item->GetPropertyNodesCount() == 0)
    {
        return;
    }

    if (item->GetPropertyNode(0)->field.ref.HasMeta<M::FrequentlyChangedValue>())
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
    if (node->field.ref.HasMeta<M::FrequentlyChangedValue>())
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

    RefreshFavoritesRoot();
}

void ReflectedPropertyModel::HideEditor(ReflectedPropertyItem* item)
{
    if (item->value != nullptr)
    {
        item->value->HideEditor();
    }

    for (int32 i = 0; i < item->GetChildCount(); ++i)
    {
        HideEditor(item->GetChild(i));
    }
}

void ReflectedPropertyModel::SetObjects(Vector<Reflection> objects)
{
    wrappersProcessor.Shoutdown();

    int childCount = static_cast<int>(rootItem->GetChildCount());
    if (childCount != 0)
    {
        nodeToItem.clear();
        beginResetModel();
        rootItem->RemoveChildren();
        endResetModel();

        childCreator.Clear();
        favoritesItem = nullptr;
    }
    rootItem->RemovePropertyNodes();

    for (Reflection& obj : objects)
    {
        Reflection::Field field(String("SelfRoot"), std::move(obj), nullptr);
        std::shared_ptr<PropertyNode> rootNode = childCreator.CreateRoot(std::move(field));
        nodeToItem.emplace(rootNode, rootItem.get());
        rootItem->AddPropertyNode(rootNode);
    }
    objects.clear();

    Update();
}

void ReflectedPropertyModel::ChildAdded(std::shared_ptr<const PropertyNode> parent, std::shared_ptr<PropertyNode> node, int32 childPosition)
{
    auto iter = nodeToItem.find(parent);
    DVASSERT(iter != nodeToItem.end());

    ReflectedPropertyItem* parentItem = iter->second;

    ReflectedPropertyItem* childItem = GetExtensionChain<MergeValuesExtension>()->LookUpItem(node, parentItem->children);
    if (childItem != nullptr)
    {
        childItem->AddPropertyNode(node);
        auto iter = itemToFavorite.find(childItem);
        if (iter != itemToFavorite.end())
        {
            iter->second->AddPropertyNode(node);
        }
    }
    else
    {
        std::unique_ptr<BaseComponentValue> valueComponent = GetExtensionChain<EditorComponentExtension>()->GetEditor(node);
        valueComponent->Init(this);

        if (favoritesItem != nullptr)
        {
            ++childPosition;
        }

        InsertGuard guard(this, parentItem, childPosition, childPosition);
        childItem = parentItem->CreateChild(std::move(valueComponent), childPosition);
        childItem->AddPropertyNode(node);
    }

    auto newNode = nodeToItem.emplace(node, childItem);
    DVASSERT(newNode.second);
}

void ReflectedPropertyModel::ChildRemoved(std::shared_ptr<PropertyNode> node)
{
    auto removeItemFn = [&](ReflectedPropertyItem* item, bool forceRemove) -> bool
    {
        RemoveGuard guard(this, item, forceRemove);
        item->RemovePropertyNode(node);
        bool needRemove = item->GetPropertyNodesCount() == 0;
        if (needRemove)
        {
            item->parent->RemoveChild(item->position);
        }

        return needRemove;
    };

    auto iter = nodeToItem.find(node);
    DVASSERT(iter != nodeToItem.end());
    ReflectedPropertyItem* item = iter->second;
    // We try to remove regular item first.
    // In favoritesOnly mode view does not know about this item,
    // so there is no need to call begin/end RemoveRows.
    bool itemRemoved = removeItemFn(item, false);
    if (itemRemoved == true)
    {
        nodeToItem.erase(node);
    }

    auto favoriteIter = itemToFavorite.find(item);

    if (favoriteIter != itemToFavorite.end())
    {
        // Favorite item we should remove from view anyway, so make forceRemove
        // to open begin\end RemoveRows even in favoritesOnly mode
        bool favoriteRemoved = removeItemFn(favoriteIter->second, true);
        DVASSERT(itemRemoved == favoriteRemoved);
        if (favoriteRemoved == true)
        {
            favoriteToItem.erase(favoriteIter->second);
            itemToFavorite.erase(favoriteIter);
        }
    }
}

ReflectedPropertyItem* ReflectedPropertyModel::MapItem(const QModelIndex& item) const
{
    if (item.isValid())
    {
        ReflectedPropertyItem* p = reinterpret_cast<ReflectedPropertyItem*>(item.internalPointer());
        if (p == nullptr)
        {
            p = GetSmartRoot();
        }
        return p->GetChild(item.row());
    }

    return GetSmartRoot();
}

QModelIndex ReflectedPropertyModel::MapItem(ReflectedPropertyItem* item) const
{
    DVASSERT(item != nullptr);
    if (item == GetSmartRoot())
    {
        return QModelIndex();
    }

    return createIndex(item->position, 0, item->parent);
}

ReflectedPropertyItem* ReflectedPropertyModel::GetSmartRoot() const
{
    if (isFavoriteOnly)
    {
        return favoritesItem;
    }

    return rootItem.get();
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
        reflectedPath.push_front(FastName(item->GetPropertyName().toStdString()));
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

QModelIndexList ReflectedPropertyModel::GetExpandedList() const
{
    QModelIndexList result;
    GetExpandedListImpl(result, GetSmartRoot());
    return result;
}

QModelIndexList ReflectedPropertyModel::GetExpandedChildren(const QModelIndex& index) const
{
    List<FastName> reflectedPath;
    QModelIndex currentIndex = index;
    while (currentIndex.isValid())
    {
        ReflectedPropertyItem* item = MapItem(currentIndex);
        reflectedPath.push_front(FastName(item->GetPropertyName().toStdString()));
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

    PropertiesItem favoritesListSettings = propertyRoot.CreateSubHolder(favoritedItemsKey);
    favoritesListSettings.Set(favoritedCountKey, static_cast<int32>(favorites.size()));
    for (size_t i = 0; i < favorites.size(); ++i)
    {
        favoritesListSettings.Set(Format(favoritedElementKey, static_cast<uint32>(i)), favorites[i]);
    }

    propertyRoot.Set(isFavoritesViewOnly, isFavoriteOnly);
}

void ReflectedPropertyModel::LoadState(const PropertiesItem& propertyRoot)
{
    PropertiesItem expandedItemsSettings = propertyRoot.CreateSubHolder(expandedItemsKey);
    expandedItems.Load(expandedItemsSettings);

    PropertiesItem favoritesListSettings = propertyRoot.CreateSubHolder(favoritedItemsKey);
    uint32 count = static_cast<uint32>(favoritesListSettings.Get(favoritedCountKey, int32(0)));
    favorites.reserve(count);

    for (uint32 i = 0; i < count; ++i)
    {
        favorites.push_back(favoritesListSettings.Get<Vector<FastName>>(Format(favoritedElementKey, static_cast<uint32>(i))));
    }

    isFavoriteOnly = propertyRoot.Get(isFavoritesViewOnly, false);
}

void ReflectedPropertyModel::HideEditors()
{
    HideEditor(rootItem.get());
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

void ReflectedPropertyModel::AddFavorite(const QModelIndex& index)
{
    ReflectedPropertyItem* item = MapItem(index);
    DVASSERT(item->IsFavorited() == false);
    DVASSERT(item->IsFavorite() == false);

    favorites.emplace_back();
    Vector<FastName>& currentFavorite = favorites.back();
    currentFavorite.reserve(8);
    while (item != nullptr)
    {
        currentFavorite.push_back(item->GetName());
        item = item->parent;
    }
    std::reverse(currentFavorite.begin(), currentFavorite.end());

    RefreshFavoritesRoot();
}

void ReflectedPropertyModel::RemoveFavorite(const QModelIndex& index)
{
    SCOPE_EXIT
    {
        RefreshFavoritesRoot();
    };

    ReflectedPropertyItem* item = MapItem(index);
    if (item->GetPropertyNode(0)->propertyType == PropertyNode::FavoritesProperty)
    {
        favorites.clear();
        for (auto iter = favoriteToItem.begin(); iter != favoriteToItem.end(); ++iter)
        {
            iter->second->SetFavorited(false);
        }
        favoriteToItem.clear();
        itemToFavorite.clear();
        return;
    }

    if (item->IsFavorite())
    {
        auto iter = favoriteToItem.find(item);
        DVASSERT(iter != favoriteToItem.end());
        item = iter->second;
    }

    DVASSERT(item->IsFavorited() == true);
    item->SetFavorited(false);

    { // Remove item from caches ...
        auto favoriteIter = itemToFavorite.find(item);
        DVASSERT(favoriteIter != itemToFavorite.end());

        ReflectedPropertyItem* f = favoriteIter->second;
        favoriteToItem.erase(f);
        itemToFavorite.erase(favoriteIter);

        { // and from items hierarchy
            beginRemoveRows(MapItem(f->parent), f->position, f->position);
            f->parent->RemoveChild(f->position);
            endRemoveRows();
        }
    }

    Vector<FastName> path;
    path.reserve(8);

    while (item != nullptr)
    {
        path.push_back(item->GetName());
        item = item->parent;
    }

    std::reverse(path.begin(), path.end());

    size_t pathSize = path.size();
    auto iter = favorites.begin();
    auto endIter = favorites.end();
    while (iter != endIter)
    {
        if (iter->size() != pathSize)
        {
            continue;
        }

        bool pathIsEqual = true;
        for (size_t i = 0; i < pathSize; ++i)
        {
            if ((*iter)[i] != path[i])
            {
                pathIsEqual = false;
                break;
            }
        }

        if (pathIsEqual == true)
        {
            iter = favorites.erase(iter);
            break;
        }
        else
            ++iter;
    }
}

bool ReflectedPropertyModel::IsFavoriteOnly() const
{
    return isFavoriteOnly;
}

void ReflectedPropertyModel::SetFavoriteOnly(bool isFavoriteOnly_)
{
    if (isFavoriteOnly == isFavoriteOnly_)
    {
        return;
    }

    uint32 childCount = rootItem->GetChildCount();
    beginResetModel();
    isFavoriteOnly = isFavoriteOnly_;
    endResetModel();
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
        FastName propertyName = FastName(child->GetPropertyName().toStdString());
        if (expandedItems.HasChildInCurrentRoot(propertyName))
        {
            list << MapItem(child);
            expandedItems.PushRoot(propertyName);
            GetExpandedListImpl(list, child);
            expandedItems.PopRoot();
        }
    }
}

void ReflectedPropertyModel::RefreshFavoritesRoot()
{
    if (rootItem->GetChildCount() == 0)
    {
        return;
    }

    if (favorites.empty() == true)
    {
        if (favoritesItem != nullptr)
        {
            ReflectedPropertyItem* parentItem = favoritesItem->parent;
            QModelIndex parentIndex = MapItem(parentItem);
            beginRemoveRows(parentIndex, favoritesItem->position, favoritesItem->position);
            parentItem->RemoveChild(favoritesItem->position);
            endRemoveRows();

            favoritesItem = nullptr;
        }

        return;
    }

    if (favoritesItem != nullptr && favoritesItem->GetChildCount() == favorites.size())
    {
        return;
    }

    static FavoriteItemValue favoriteValueItem;
    bool favoriteRootIsEmpty = favoritesItem == nullptr;
    if (favoriteRootIsEmpty == true)
    {
        beginInsertRows(QModelIndex(), 0, 0);
        std::unique_ptr<BaseComponentValue> emptyComponentValue = std::make_unique<EmptyComponentValue>();
        BaseComponentValue::Style style;
        style.bgColor = QPalette::AlternateBase;
        style.fontBold = true;
        style.fontColor = QPalette::ButtonText;
        emptyComponentValue->SetStyle(style);
        emptyComponentValue->Init(this);
        favoritesItem = rootItem->CreateChild(std::move(emptyComponentValue), 0);
        favoritesItem->SetFavorite(true);

        std::shared_ptr<PropertyNode> favoriteNode = std::make_shared<PropertyNode>();
        favoriteNode->field.key = String("Favorites");
        favoriteNode->field.ref = Reflection::Create(&favoriteValueItem);
        favoriteNode->propertyType = PropertyNode::FavoritesProperty;
        favoriteNode->cachedValue = favoriteNode->field.ref.GetValue();

        favoritesItem->AddPropertyNode(favoriteNode);
    }

    Set<size_t> candidates;
    for (size_t i = 0; i < favorites.size(); ++i)
    {
        candidates.insert(i);
    }

    for (int32 childIndex = 0; childIndex != favoritesItem->GetChildCount(); ++childIndex)
    {
        ReflectedPropertyItem* f = favoritesItem->GetChild(childIndex);
        ReflectedPropertyItem* item = favoriteToItem.find(f)->second;

        auto candidateIter = candidates.begin();
        while (candidateIter != candidates.end())
        {
            const Vector<FastName>& path = favorites[*candidateIter];
            bool pathIsEqual = true;
            ReflectedPropertyItem* currentItem = item;

            auto pathIter = path.rbegin();
            auto pathEndIter = path.rend();

            while (true)
            {
                bool pathFinished = pathIter == pathEndIter;
                bool itemHierarchyFinished = currentItem == nullptr;
                if (pathFinished == true && itemHierarchyFinished == true)
                {
                    break;
                }

                if (pathFinished == true || itemHierarchyFinished == true)
                {
                    pathIsEqual = false;
                    break;
                }

                if (currentItem->GetName() != *pathIter)
                {
                    pathIsEqual = false;
                    break;
                }

                currentItem = currentItem->parent;
                ++pathIter;
            }

            if (pathIsEqual)
            {
                candidateIter = candidates.erase(candidateIter);
            }
            else
            {
                ++candidateIter;
            }
        }
    }

    RefreshFavorites(rootItem.get(), 0, favoriteRootIsEmpty, candidates);

    if (favoriteRootIsEmpty == true)
    {
        endInsertRows();
    }
}

void ReflectedPropertyModel::RefreshFavorites(ReflectedPropertyItem* item, uint32 level, bool insertSessionIsOpen, const Set<size_t>& candidates)
{
    FastName itemName = item->GetName();
    bool itemFoundInPath = false;

    for (size_t pathIndex : candidates)
    {
        const Vector<FastName>& path = favorites[pathIndex];
        uint32 pathSize = static_cast<uint32>(path.size());
        if (pathSize <= level)
        {
            continue;
        }

        if (path[level] == itemName)
        {
            if (level + 1 == pathSize)
            {
                if (insertSessionIsOpen == false)
                {
                    int position = static_cast<int>(pathIndex);
                    beginInsertRows(MapItem(favoritesItem), position, position);
                }

                ReflectedPropertyItem* copyItem = CreateDeepCopy(item, favoritesItem, pathIndex);
                favoriteToItem.emplace(copyItem, item);
                itemToFavorite.emplace(item, copyItem);

                item->SetFavorited(true);
                copyItem->SetFavorite(true);

                if (insertSessionIsOpen == false)
                {
                    endInsertRows();
                }
            }
            else
            {
                itemFoundInPath = true;
            }
        }
    }

    if (itemFoundInPath == true)
    {
        int32 childCount = item->GetChildCount();
        for (int32 i = 0; i < childCount; ++i)
        {
            RefreshFavorites(item->GetChild(i), level + 1, insertSessionIsOpen, candidates);
        }
    }
}

ReflectedPropertyItem* ReflectedPropertyModel::CreateDeepCopy(ReflectedPropertyItem* itemToCopy, ReflectedPropertyItem* copyParent, size_t positionInParent)
{
    std::unique_ptr<BaseComponentValue> valueComponent = GetExtensionChain<EditorComponentExtension>()->GetEditor(itemToCopy->GetPropertyNode(0));
    valueComponent->Init(this);
    ReflectedPropertyItem* copyItem = copyParent->CreateChild(std::move(valueComponent), static_cast<int32>(positionInParent));
    for (int32 i = 0; i < itemToCopy->GetPropertyNodesCount(); ++i)
    {
        copyItem->AddPropertyNode(itemToCopy->GetPropertyNode(i));
    }

    for (int32 childIndex = 0; childIndex < itemToCopy->GetChildCount(); ++childIndex)
    {
        CreateDeepCopy(itemToCopy->GetChild(childIndex), copyItem, itemToCopy->GetChildCount());
    }

    return copyItem;
}

} // namespace TArc
} // namespace DAVA
