#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.hpp"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
namespace RPMDetails
{
const int ValueColumn = 1;
} // namespace RPMDetails

ReflectedPropertyItem::ReflectedPropertyItem(ReflectedPropertyModel& model_)
    : model(model_)
{
}

void ReflectedPropertyItem::InjectData(size_t roleId, const Any& value)
{
    injectedData[roleId] = value;
}

Any ReflectedPropertyItem::GetInjectedData(size_t roleId)
{
    auto iter = injectedData.find(roleId);
    if (iter != injectedData.end())
    {
        return iter->second;
    }

    return Variant();
}

Any ReflectedPropertyItem::GetData() const
{
    if (column == RPMDetails::ValueColumn)
    {
        auto iter = injectedData.find(roleId);
        if (iter != injectedData.end())
        {
            return iter->second;
        }
    }

    return model.GetDataImpl(this, column, roleId);
}

bool ReflectedPropertyItem::SetData(int row, int column, size_t roleId, const Any& data)
{
    return false;
    // TODO uncomment this after back write on multiselection will be fixed
    //return model.setData(this, column, roleId, data);
}

Variant ReflectedPropertyItem::getValue() const
{
    return itemValue;
}

Variant ReflectedPropertyItem::evalValue(IDefinitionManager& definitionManager) const
{
    Variant result;
    if (!nodes.empty())
    {
        auto iter = nodes.begin();

        std::shared_ptr<const PropertyNode> node = *iter;
        result = node->propertyInstance->get(node->object, definitionManager);
        ++iter;
        for (iter; iter != nodes.end(); ++iter)
        {
            std::shared_ptr<const PropertyNode> node = *iter;
            Variant r = node->propertyInstance->get(node->object, definitionManager);
            if (r != result)
            {
                result = Variant();
                break;
            }
        }
    }

    return result;
}

void ReflectedPropertyItem::setValue(Variant&& newValue)
{
    itemValue = std::move(newValue);
}

ReflectedPropertyItem* ReflectedPropertyItem::getNonConstParent() const
{
    return parent;
}

size_t ReflectedPropertyItem::getPosition() const
{
    return position;
}

ReflectedPropertyItem* ReflectedPropertyItem::createChild()
{
    size_t newItemindex = children.size();
    children.push_back(std::unique_ptr<RefPropertyItem>(new RefPropertyItem(this, newItemindex)));
    return children.back().get();
}

size_t ReflectedPropertyItem::getChildCount() const
{
    return children.size();
}

ReflectedPropertyItem* ReflectedPropertyItem::getChild(size_t index) const
{
    assert(index < getChildCount());
    return children[index].get();
}

void ReflectedPropertyItem::removeChild(size_t index)
{
    assert(index < getChildCount());
    auto childIter = children.begin() + index;
    for (auto iter = childIter + 1; iter != children.end(); ++iter)
        (*iter)->position = (*iter)->position - 1;

    children.erase(childIter);
}

void ReflectedPropertyItem::removeChildren()
{
    children.clear();
}

void ReflectedPropertyItem::addObject(const std::shared_ptr<const PropertyNode>& node)
{
#ifdef _DEBUG
    if (!nodes.empty())
    {
        assert(nodes.front()->propertyInstance == node->propertyInstance);
    }
#endif

    nodes.push_back(node);
}

void ReflectedPropertyItem::removeObject(const std::shared_ptr<const PropertyNode>& object)
{
    auto iter = std::find(nodes.begin(), nodes.end(), object);
    if (iter == nodes.end())
        return;

    nodes.erase(iter);
}

void ReflectedPropertyItem::removeObjects()
{
    nodes.clear();
}

bool ReflectedPropertyItem::hasObjects() const
{
    return !nodes.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////

ReflectedPropertyModel::ReflectedPropertyModel(IComponentContext& context)
    : interfacesHolder(context)
    , childCreator(context)
{
    rootItem.reset(new ReflectedPropertyItem(*this));

    registerExtension(SetterGetterExtension::CreateDummy());
    registerExtension(MergeValuesExtension::CreateDummy());
    registerExtension(InjectDataExtension::CreateDummy());

    using namespace std::placeholders;
    childCreator.nodeCreated.connect(std::bind(&ReflectedPropertyModel::childAdded, this, _1, _2, _3));
    childCreator.nodeRemoved.connect(std::bind(&ReflectedPropertyModel::childRemoved, this, _1));

    registerExtension(std::make_shared<DefaultSetterGetterExtension>(context));
    registerExtension(std::make_shared<UrlGetterExtension>());
    registerExtension(std::make_shared<DefaultChildCheatorExtension>());
    registerExtension(std::make_shared<DefaultMergeValueExtension>());
}

ReflectedPropertyModel::~ReflectedPropertyModel()
{
    rootItem.reset();
}

void ReflectedPropertyModel::update()
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    update(rootItem.get());
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start).count();
    NGT_TRACE_MSG("update duration : %f seconds\n", duration);
}

void ReflectedPropertyModel::update(ReflectedPropertyItem* item)
{
    INTERFACE_REQUEST(IDefinitionManager, defManager, interfacesHolder, void());

    for (const std::shared_ptr<const PropertyNode>& node : item->nodes)
    {
        childCreator.UpdateSubTree(node);
    }

    for (const std::unique_ptr<ReflectedPropertyItem>& child : item->children)
    {
        update(child.get());
    }

    AbstractItemModel::ItemIndex itemIndex = item->getItemIndex();
    Variant value = item->evalValue(defManager);
    if (value != item->itemValue)
    {
        preDataChanged(itemIndex, ValueRole::roleId_, value);
        item->setValue(std::move(value));
        postDataChanged(itemIndex, ValueRole::roleId_, item->itemValue);
    }
}

void ReflectedPropertyModel::setObjects(const std::vector<ObjectHandle>& objects)
{
    int childCount = static_cast<int>(rootItem->getChildCount());
    if (childCount != 0)
    {
        nodeToItem.clear();
        preRowRemoved(ItemIndex(), 0, childCount);
        rootItem->removeChildren();
        postRowRemoved(ItemIndex(), 0, childCount);
        rootItem->removeObjects();
        childCreator.Clear();
    }

    for (const ObjectHandle& handle : objects)
    {
        std::shared_ptr<const PropertyNode> rootNode = childCreator.CreateRoot(handle);
        nodeToItem.emplace(rootNode, rootItem.get());
        rootItem->addObject(rootNode);
    }

    update();
}

AbstractItem* ReflectedPropertyModel::item(const ItemIndex& index) const
{
    const ReflectedPropertyItem* parent = getEffectiveParent(index.parent_);
    if (parent == nullptr)
        return nullptr;

    if (index.row_ < parent->getChildCount())
        return parent->getChild(index.row_);

    return nullptr;
}

void ReflectedPropertyModel::index(const AbstractItem* item, ItemIndex& o_Index) const
{
    assert(item != nullptr);
    o_Index = getModelParent(static_cast<const ReflectedPropertyItem*>(item));
}

int ReflectedPropertyModel::rowCount(const AbstractItem* item) const
{
    return getEffectiveParent(item)->getChildCount();
}

int ReflectedPropertyModel::columnCount(const AbstractItem* item) const
{
    return 2;
}

void ReflectedPropertyModel::childAdded(const std::shared_ptr<const PropertyNode>& parent, const std::shared_ptr<const PropertyNode>& node, size_t childPosition)
{
    INTERFACE_REQUEST(IDefinitionManager, defManager, interfacesHolder, void());

    auto iter = nodeToItem.find(parent);
    assert(iter != nodeToItem.end());

    ReflectedPropertyItem* parentItem = iter->second;

    std::shared_ptr<InjectDataExtension> injectExtension = getExtensionChain<InjectDataExtension>();
    ReflectedPropertyItem* childItem = getExtensionChain<MergeValuesExtension>()->LookUpItem(node, parentItem->children, defManager);
    if (childItem != nullptr)
    {
        childItem->addObject(node);
        injectExtension->UpdateInjection(childItem);
    }
    else
    {
        size_t childCount = parentItem->getChildCount();
        AbstractItemModel::ItemIndex parentIndex = getModelParent(parentItem);

        preRowInserted(parentIndex, childCount, 1);
        childItem = parentItem->createChild();
        childItem->addObject(node);
        postRowInserted(parentIndex, childCount, 1);
        injectExtension->Inject(childItem);
    }

    auto newNode = nodeToItem.emplace(node, childItem);
    assert(newNode.second);
}

void ReflectedPropertyModel::childRemoved(const std::shared_ptr<const PropertyNode>& node)
{
    auto iter = nodeToItem.find(node);
    assert(iter != nodeToItem.end());

    ReflectedPropertyItem* item = iter->second;
    item->removeObject(node);
    nodeToItem.erase(node);

    if (!item->hasObjects())
    {
        AbstractItemModel::ItemIndex index = getModelParent(item->getParent());
        preRowRemoved(index, index.row_, 1);
        item->getNonConstParent()->removeChild(index.row_);
        postRowRemoved(index, index.row_, 1);
    }
    else
    {
        getExtensionChain<InjectDataExtension>()->UpdateInjection(item);
    }
}

const ReflectedPropertyItem* ReflectedPropertyModel::getEffectiveParent(const AbstractItem* modelParent) const
{
    return getEffectiveParent(const_cast<AbstractItem*>(modelParent));
}

ReflectedPropertyItem* ReflectedPropertyModel::getEffectiveParent(AbstractItem* modelParent) const
{
    return modelParent == nullptr ? rootItem.get() : static_cast<ReflectedPropertyItem*>(modelParent);
}

AbstractListModel::ItemIndex ReflectedPropertyModel::getModelParent(const ReflectedPropertyItem* effectiveParent) const
{
    ItemIndex idx;

    if (effectiveParent != rootItem.get())
    {
        idx = effectiveParent->getItemIndex();

        if (idx.parent_ == rootItem.get())
        {
            idx.parent_ = nullptr;
        }
    }

    return idx;
}

Variant ReflectedPropertyModel::getDataImpl(const ReflectedPropertyItem* item, int column, size_t roleId) const
{
    INTERFACE_REQUEST(IDefinitionManager, defManager, interfacesHolder, Variant());
    return getExtensionChain<SetterGetterExtension>()->GetValue(item, column, roleId, defManager);
}

bool ReflectedPropertyModel::setDataImpl(ReflectedPropertyItem* item, int column, size_t roleId, const Variant& data)
{
    INTERFACE_REQUEST(IDefinitionManager, defManager, interfacesHolder, false);
    INTERFACE_REQUEST(ICommandManager, commandManager, interfacesHolder, false);
    return getExtensionChain<SetterGetterExtension>()->SetValue(item, column, roleId, data, defManager, commandManager);
}

void ReflectedPropertyModel::registerExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    const TypeId& extType = extension->GetType();
    if (extType == TypeId::getType<ChildCreatorExtension>())
    {
        childCreator.RegisterExtension(polymorphCast<ChildCreatorExtension>(extension));
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
    const TypeId& extType = extension->GetType();
    if (extType == TypeId::getType<ChildCreatorExtension>())
    {
        childCreator.UnregisterExtension(polymorphCast<ChildCreatorExtension>(extension));
        return;
    }

    auto iter = extensions.find(extType);
    if (iter == extensions.end())
    {
        /// you do something wrong
        assert(false);
        return;
    }

    iter->second = ExtensionChain::RemoveExtension(iter->second, extension);
}
} // namespace TArc
} // namespace wgt
