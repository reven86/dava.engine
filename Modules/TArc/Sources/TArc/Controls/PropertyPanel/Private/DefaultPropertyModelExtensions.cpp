#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"

#include "TArc/Controls/PropertyPanel/Private/ObjectsPool.h"
#include "TArc/Controls/PropertyPanel/Private/TextComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/BoolComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/IntComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
void DefaultChildCheatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    DVASSERT(node->field.ref.IsValid());

    if (node->propertyType == PropertyNode::SelfRoot || node->propertyType == PropertyNode::RealProperty)
    {
        Vector<Reflection::Field> fields = node->field.ref.GetFields();
        for (Reflection::Field& field : fields)
        {
            children.push_back(allocator->CreatePropertyNode(std::move(field), PropertyNode::RealProperty));
        }
    }

    return ChildCreatorExtension::ExposeChildren(node, children);
}

class DefaultAllocator : public IChildAllocator
{
public:
    DefaultAllocator();
    ~DefaultAllocator();
    std::shared_ptr<PropertyNode> CreatePropertyNode(Reflection::Field&& reflection, int32_t type = PropertyNode::RealProperty) override;

private:
    ObjectsPool<PropertyNode, SingleThreadStrategy> pool;
};

DefaultAllocator::DefaultAllocator()
    : pool(10000, 10)
{
}

DefaultAllocator::~DefaultAllocator()
{
}

std::shared_ptr<PropertyNode> DefaultAllocator::CreatePropertyNode(Reflection::Field&& field, int32_t type)
{
    std::shared_ptr<PropertyNode> result = pool.RequestObject();
    result->propertyType = type;
    result->field = std::move(field);
    if (result->field.ref.IsValid())
    {
        result->cachedValue = result->field.ref.GetValue();
    }

    return result;
}

std::shared_ptr<IChildAllocator> CreateDefaultAllocator()
{
    return std::make_shared<DefaultAllocator>();
}

ReflectedPropertyItem* DefaultMergeValueExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    DVASSERT(node->field.ref.IsValid());

    ReflectedPropertyItem* result = nullptr;
    const ReflectedType* valueType = node->field.ref.GetValueObject().GetReflectedType();

    for (const std::unique_ptr<ReflectedPropertyItem>& item : items)
    {
        DVASSERT(item->GetPropertyNodesCount() > 0);
        std::shared_ptr<const PropertyNode> etalonNode = item->GetPropertyNode(0);
        const ReflectedType* etalonItemType = etalonNode->field.ref.GetValueObject().GetReflectedType();

        if (valueType == etalonItemType && etalonNode->field.key == node->field.key)
        {
            result = item.get();
            break;
        }
    }

    return result;
}

std::unique_ptr<BaseComponentValue> DefaultEditorComponentExtension::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    const Type* valueType = node->cachedValue.GetType();
    if (valueType == Type::Instance<String>() ||
        valueType == Type::Instance<FastName>() ||
        valueType == Type::Instance<const char*>())
    {
        return std::make_unique<TextComponentValue>();
    }
    else if (valueType == Type::Instance<bool>())
    {
        return std::make_unique<BoolComponentValue>();
    }
    /*else if (valueType == Type::Instance<int32>() ||
             valueType == Type::Instance<uint32>())
    {
        return std::make_unique<IntComponentValue>();
    }*/

    return EditorComponentExtension::GetEditor(node);
}
} // namespace TArc
} // namespace DAVA
