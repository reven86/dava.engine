#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/KeyedArchiveChildCreator.h"
#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"

#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

namespace DAVA
{
namespace TArc
{
ReflectedPropertyModel::ReflectedPropertyModel(ContextAccessor* accessor_, OperationInvoker* invoker_, UI* ui_)
    : accessor(accessor_)
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
    SetObjects(Vector<DAVA::Reflection>());
    rootItem.reset();
}

//////////////////////////////////////
//       QAbstractItemModel         //
//////////////////////////////////////

int ReflectedPropertyModel::rowCount(const QModelIndex& parent) const
{
    return MapItem(parent)->GetChildCount();
}

int ReflectedPropertyModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ReflectedPropertyModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return MapItem(index)->GetPropertyName();
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
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 1)
    {
        ReflectedPropertyItem* item = MapItem(index);
        std::shared_ptr<const PropertyNode> node = item->GetPropertyNode(0);
        if (!node->field.ref.IsReadonly())
        {
            flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
}

QModelIndex ReflectedPropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        ReflectedPropertyItem* item = MapItem(parent);
        if (row < item->GetChildCount())
            return createIndex(row, column, item);

        return QModelIndex();
    }

    return createIndex(row, column, rootItem.get());
}

QModelIndex ReflectedPropertyModel::parent(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    ReflectedPropertyItem* item = reinterpret_cast<ReflectedPropertyItem*>(index.internalPointer());
    if (item == nullptr || item == rootItem.get())
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
    Logger::Debug(" === ReflectedPropertyModel::Update : %d ===", static_cast<int32>(SystemTimer::GetMs() - start));
}

void ReflectedPropertyModel::Update(ReflectedPropertyItem* item)
{
    int32 propertyNodesCount = item->GetPropertyNodesCount();
    for (int32 i = 0; i < propertyNodesCount; ++i)
    {
        childCreator.UpdateSubTree(item->GetPropertyNode(i));
    }

    for (const std::unique_ptr<ReflectedPropertyItem>& child : item->children)
    {
        Update(child.get());
    }

    fastWrappersProcessor.Sync();
}

void ReflectedPropertyModel::UpdateFastImpl(ReflectedPropertyItem* item)
{
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
    Logger::Debug(" === ReflectedPropertyModel::UpdateFast : %d ===", static_cast<int32>(SystemTimer::GetMs() - start));
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
        beginRemoveRows(QModelIndex(), 0, childCount - 1);
        rootItem->RemoveChildren();
        endRemoveRows();

        childCreator.Clear();
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
    }
    else
    {
        std::unique_ptr<BaseComponentValue> valueComponent = GetExtensionChain<EditorComponentExtension>()->GetEditor(node);
        valueComponent->Init(this);

        QModelIndex parentIndex = MapItem(parentItem);

        beginInsertRows(parentIndex, childPosition, childPosition);
        childItem = parentItem->CreateChild(std::move(valueComponent), childPosition);
        childItem->AddPropertyNode(node);
        endInsertRows();
    }

    auto newNode = nodeToItem.emplace(node, childItem);
    DVASSERT(newNode.second);
}

void ReflectedPropertyModel::ChildRemoved(std::shared_ptr<PropertyNode> node)
{
    auto iter = nodeToItem.find(node);
    DVASSERT(iter != nodeToItem.end());

    ReflectedPropertyItem* item = iter->second;
    ReflectedPropertyItem* itemParent = item->parent;

    bool needRemoveRow = item->GetPropertyNodesCount() == 1;

    if (needRemoveRow)
    {
        QModelIndex parentIndex = MapItem(itemParent);
        beginRemoveRows(parentIndex, item->position, item->position);
    }

    item->RemovePropertyNode(node);
    nodeToItem.erase(node);

    if (needRemoveRow)
    {
        itemParent->RemoveChild(item->position);
        endRemoveRows();
    }
}

ReflectedPropertyItem* ReflectedPropertyModel::MapItem(const QModelIndex& item) const
{
    if (item.isValid())
    {
        ReflectedPropertyItem* p = reinterpret_cast<ReflectedPropertyItem*>(item.internalPointer());
        if (p == nullptr)
        {
            p = rootItem.get();
        }
        return p->GetChild(item.row());
    }

    return rootItem.get();
}

QModelIndex ReflectedPropertyModel::MapItem(ReflectedPropertyItem* item) const
{
    DVASSERT(item != nullptr);
    if (item == rootItem.get())
    {
        return QModelIndex();
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
    GetExpandedListImpl(result, rootItem.get());
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

void ReflectedPropertyModel::SaveExpanded(PropertiesItem& propertyRoot) const
{
    expandedItems.Save(propertyRoot);
}

void ReflectedPropertyModel::LoadExpanded(const PropertiesItem& propertyRoot)
{
    expandedItems.Load(propertyRoot);
}

void ReflectedPropertyModel::HideEditors()
{
    HideEditor(rootItem.get());
}

void ReflectedPropertyModel::GetExpandedListImpl(QModelIndexList& list, ReflectedPropertyItem* item) const
{
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

} // namespace TArc
} // namespace DAVA
