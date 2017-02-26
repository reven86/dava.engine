#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"

#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

namespace DAVA
{
namespace TArc
{
namespace ReflectedPropertyModelDetail
{
std::tuple<const Type*, const ReflectedType*, const ReflectedStructure*> UnpackReflectionTypeInfo(const Reflection& r)
{
    const Type* t = r.GetValueType();
    if (t->IsPointer())
    {
        t = t->Deref();
    }

    const ReflectedType* reflectedType = ReflectedTypeDB::GetByType(t);
    const ReflectedStructure* structure = nullptr;
    if (reflectedType != nullptr)
    {
        structure = reflectedType->GetStrucutre();
    }

    return std::make_tuple(t, reflectedType, structure);
}

void InjectExpandedMeta(std::unique_ptr<ReflectedMeta>& meta, bool isExpanded)
{
    if (meta == nullptr)
    {
        meta.reset(new ReflectedMeta());
    }

    const M::FieldExpanded* expanded = meta->GetMeta<M::FieldExpanded>();
    if (expanded == nullptr)
    {
        meta->Emplace(M::FieldExpanded());
        expanded = meta->GetMeta<M::FieldExpanded>();
    }

    const_cast<M::FieldExpanded*>(expanded)->isExpanded = isExpanded;
};

const String ExpandedCount = "expandedCount";
const String ExpandedItem = "expandedItem";
const String TypeName = "typeName";
const String FieldName = "fieldName";
}

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
    using namespace ReflectedPropertyModelDetail;

    ReflectedPropertyItem* item = MapItem(index);
    DVASSERT(item);
    DVASSERT(item->GetPropertyNodesCount() > 0);
    std::shared_ptr<const PropertyNode> node = item->GetPropertyNode(0);
    ExpandedFieldDescriptor descr;

    QModelIndex parentIndex = index.parent();
    if (!parentIndex.isValid())
    {
        const Type* type = nullptr;
        const ReflectedType* refType = nullptr;
        const ReflectedStructure* structure = nullptr;
        std::tie(type, refType, structure) = UnpackReflectionTypeInfo(node->field.ref);
        InjectExpandedMeta(const_cast<ReflectedStructure*>(structure)->meta, expanded);

        descr.typePermanentName = refType->GetPermanentName();
    }
    else
    {
        ReflectedPropertyItem* parentItem = MapItem(parentIndex);
        DVASSERT(parentItem->GetPropertyNodesCount() > 0);
        std::shared_ptr<const PropertyNode> parentNode = parentItem->GetPropertyNode(0);

        const Type* type = nullptr;
        const ReflectedType* refType = nullptr;
        const ReflectedStructure* structure = nullptr;
        std::tie(type, refType, structure) = UnpackReflectionTypeInfo(parentNode->field.ref);
        DVASSERT(structure != nullptr);

        ReflectedStructure::Field* field = nullptr;
        for (const std::unique_ptr<ReflectedStructure::Field>& f : structure->fields)
        {
            if (f->name == node->field.key.Cast<String>(String()))
            {
                field = const_cast<ReflectedStructure::Field*>(f.get());
                break;
            }
        }
        DVASSERT(field != nullptr);

        InjectExpandedMeta(field->meta, expanded);

        descr.typePermanentName = refType->GetPermanentName();
        descr.fieldName = field->name;
    }

    DVASSERT(!descr.typePermanentName.empty());
    auto iter = std::find(expandedFields.begin(), expandedFields.end(), descr);
    if (expanded == true)
    {
        if (iter == expandedFields.end())
        {
            expandedFields.push_back(descr);
        }
    }
    else
    {
        DVASSERT(iter != expandedFields.end());
        RemoveExchangingWithLast(expandedFields, std::distance(expandedFields.begin(), iter));
    }
}

QModelIndexList ReflectedPropertyModel::GetExpandedList() const
{
    QModelIndexList result;
    GetExpandedListImpl(result, rootItem.get());
    return result;
}

void ReflectedPropertyModel::SaveExpanded(PropertiesItem& propertyRoot)
{
    using namespace ReflectedPropertyModelDetail;
    propertyRoot.Set(ExpandedCount, static_cast<int32>(expandedFields.size()));
    int32 counter = 0;
    for (const ExpandedFieldDescriptor& descr : expandedFields)
    {
        String key = Format("%s_%d", ExpandedItem.c_str(), counter++);
        PropertiesItem expandedItem = propertyRoot.CreateSubHolder(key);
        expandedItem.Set(TypeName, QString::fromStdString(descr.typePermanentName));
        expandedItem.Set(FieldName, QString::fromStdString(descr.fieldName));
    }
}

void ReflectedPropertyModel::LoadExpanded(const PropertiesItem& propertyRoot)
{
    using namespace ReflectedPropertyModelDetail;

    int32 count = propertyRoot.Get(ExpandedCount, 0);
    expandedFields.reserve(count);
    for (int32 i = 0; i < count; ++i)
    {
        String key = Format("%s_%d", ExpandedItem.c_str(), i);
        PropertiesItem expandedItem = propertyRoot.CreateSubHolder(key);
        ExpandedFieldDescriptor descr;
        descr.typePermanentName = expandedItem.Get(TypeName, QString()).toStdString();
        DVASSERT(!descr.typePermanentName.empty());
        descr.fieldName = expandedItem.Get(FieldName, QString()).toStdString();

        expandedFields.push_back(descr);
    }

    for (const ExpandedFieldDescriptor& desc : expandedFields)
    {
        const ReflectedType* type = ReflectedTypeDB::GetByPermanentName(desc.typePermanentName);
        DVASSERT(type != nullptr);
        ReflectedStructure* structure = const_cast<ReflectedStructure*>(type->GetStrucutre());
        DVASSERT(structure != nullptr);
        if (desc.fieldName.empty())
        {
            InjectExpandedMeta(structure->meta, true);
        }
        else
        {
            for (std::unique_ptr<ReflectedStructure::Field>& f : structure->fields)
            {
                if (f->name == desc.fieldName)
                {
                    InjectExpandedMeta(f->meta, true);
                }
            }
        }
    }
}

void ReflectedPropertyModel::GetExpandedListImpl(QModelIndexList& list, ReflectedPropertyItem* item) const
{
    using namespace ReflectedPropertyModelDetail;
    QModelIndex index = MapItem(item);
    if (index.isValid())
    {
        DVASSERT(item->GetPropertyNodesCount() > 0);
        std::shared_ptr<const PropertyNode> node = item->GetPropertyNode(0);
        const M::FieldExpanded* fieldExpand = node->field.ref.GetMeta<M::FieldExpanded>();
        if (fieldExpand != nullptr)
        {
            if (fieldExpand->isExpanded == true)
            {
                list << index;
            }
        }
        else
        {
            const Type* type = nullptr;
            const ReflectedType* refType = nullptr;
            const ReflectedStructure* structure = nullptr;
            std::tie(type, refType, structure) = UnpackReflectionTypeInfo(node->field.ref);
            if (structure != nullptr && structure->meta != nullptr)
            {
                const M::FieldExpanded* expandedMeta = structure->meta->GetMeta<M::FieldExpanded>();
                if (expandedMeta != nullptr && expandedMeta->isExpanded == true)
                {
                    list << index;
                }
            }
        }
    }

    int32 childCount = item->GetChildCount();
    for (int32 i = 0; i < childCount; ++i)
    {
        GetExpandedListImpl(list, item->GetChild(i));
    }
}

} // namespace TArc
} // namespace DAVA
