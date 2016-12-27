#include "TArc/Controls/PropertyPanel/DefaultPropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/ReflectedPropertyItem.h"

#include "TArc/Controls/PropertyPanel/Private/ObjectsPool.h"
#include "TArc/Controls/PropertyPanel/Private/TextComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
void DefaultChildCheatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    DVASSERT(node->reflectedObject.IsValid());

    if (node->propertyType == PropertyNode::RealProperty)
    {
        Vector<Reflection::Field> fields = node->reflectedObject.GetFields();
        for (Reflection::Field& field : fields)
        {
            children.push_back(allocator->CreatePropertyNode(std::move(field.key), std::move(field.ref), PropertyNode::RealProperty));
        }
    }

    return ChildCreatorExtension::ExposeChildren(node, children);
}

class DefaultAllocator : public IChildAllocator
{
public:
    DefaultAllocator();
    ~DefaultAllocator();
    std::shared_ptr<PropertyNode> CreatePropertyNode(Any&& fieldName, Reflection&& reflection, int32_t type = PropertyNode::RealProperty) override;

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

std::shared_ptr<PropertyNode> DefaultAllocator::CreatePropertyNode(Any&& fieldName, Reflection&& reflection, int32_t type)
{
    std::shared_ptr<PropertyNode> result = pool.RequestObject();
    result->propertyType = type;
    result->fieldName = std::move(fieldName);
    result->reflectedObject = std::move(reflection);
    result->cachedValue = result->reflectedObject.GetValue();

    return result;
}

std::shared_ptr<IChildAllocator> CreateDefaultAllocator()
{
    return std::make_shared<DefaultAllocator>();
}

ReflectedPropertyItem* DefaultMergeValueExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    DVASSERT(node->reflectedObject.IsValid());

    ReflectedPropertyItem* result = nullptr;
    const ReflectedType* valueType = node->reflectedObject.GetValueObject().GetReflectedType();

    for (const std::unique_ptr<ReflectedPropertyItem>& item : items)
    {
        DVASSERT(item->GetPropertyNodesCount() > 0);
        std::shared_ptr<const PropertyNode> etalonNode = item->GetPropertyNode(0);
        const ReflectedType* etalonItemType = etalonNode->reflectedObject.GetValueObject().GetReflectedType();

        if (valueType == etalonItemType && etalonNode->fieldName == node->fieldName)
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
    if (valueType == Type::Instance<String>())
    {
        return std::make_unique<TextComponentValue>();
    }

    return EditorComponentExtension::GetEditor(node);
}
} // namespace TArc
} // namespace DAVA
