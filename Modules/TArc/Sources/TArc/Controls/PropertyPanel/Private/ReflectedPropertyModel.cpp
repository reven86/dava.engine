#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"

#include <Scene3D/Entity.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
ReflectedPropertyModel::ReflectedPropertyModel()
{
    rootItem.reset(new ReflectedPropertyItem(this, std::make_unique<EmptyComponentValue>()));

    RegisterExtension(ChildCreatorExtension::CreateDummy());
    RegisterExtension(MergeValuesExtension::CreateDummy());
    RegisterExtension(EditorComponentExtension::CreateDummy());
    RegisterExtension(ModifyExtension::CreateDummy());

    RegisterExtension(std::make_shared<DefaultChildCheatorExtension>());
    RegisterExtension(std::make_shared<DefaultMergeValueExtension>());
    RegisterExtension(std::make_shared<DefaultEditorComponentExtension>());

    using namespace std::placeholders;
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
    if (!parent.isValid())
    {
        return createIndex(row, column, nullptr);
    }

    return createIndex(row, column, MapItem(parent));
}

QModelIndex ReflectedPropertyModel::parent(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    ReflectedPropertyItem* item = reinterpret_cast<ReflectedPropertyItem*>(index.internalPointer());
    if (item == nullptr)
    {
        return QModelIndex();
    }
    return createIndex(0, 0, item->parent);
}

//////////////////////////////////////
//       QAbstractItemModel         //
//////////////////////////////////////

void ReflectedPropertyModel::Update()
{
    //std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    Update(rootItem.get());
    //double duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start).count();
    //NGT_TRACE_MSG("update duration : %f seconds\n", duration);
    wrappersProcessor.Sync();
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
        Reflection::Field field;
        field.ref = std::move(obj);
        field.key = Any(String("SelfRoot"));
        std::shared_ptr<PropertyNode> rootNode = childCreator.CreateRoot(std::move(field));
        nodeToItem.emplace(rootNode, rootItem.get());
        rootItem->AddPropertyNode(rootNode);
    }

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
    assert(iter != nodeToItem.end());

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

} // namespace TArc
} // namespace DAVA
