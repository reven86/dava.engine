#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

namespace DAVA
{
namespace TArc
{
namespace PMEDetails
{
class DummyChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>&, Vector<std::shared_ptr<PropertyNode>>&) const override
    {
    }
};

class DummyMerger : public MergeValuesExtension
{
public:
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const override
    {
        return nullptr;
    }
};

class DummyEditorComponent : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override
    {
        return std::make_unique<EmptyComponentValue>();
    }
};
}

std::shared_ptr<PropertyNode> MakeRootNode(IChildAllocator* allocator)
{
    return allocator->CreatePropertyNode(Any("SelfRoot"), DAVA::Reflection(), PropertyNode::SelfRoot);
}

bool PropertyNode::operator==(const PropertyNode& other) const
{
    return propertyType == other.propertyType &&
    reflectedObject == other.reflectedObject &&
    cachedValue == other.cachedValue;
}

bool PropertyNode::operator!=(const PropertyNode& other) const
{
    return propertyType != other.propertyType ||
    reflectedObject != other.reflectedObject ||
    cachedValue != other.cachedValue;
}

void PropertyNode::SetValue(const Any& value)
{
    if (reflectedObject.SetValue(cachedValue))
    {
        cachedValue = value;
    }
}

void PropertyNode::SetValue(Any&& value)
{
    if (reflectedObject.SetValue(value))
    {
        cachedValue = std::move(value);
    }
}

ChildCreatorExtension::ChildCreatorExtension()
    : ExtensionChain(Type::Instance<ChildCreatorExtension>())
{
}

void ChildCreatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    GetNext<ChildCreatorExtension>()->ExposeChildren(node, children);
}

std::shared_ptr<ChildCreatorExtension> ChildCreatorExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyChildCreator>();
}

void ChildCreatorExtension::SetAllocator(std::shared_ptr<IChildAllocator> allocator_)
{
    allocator = allocator_;
}

MergeValuesExtension::MergeValuesExtension()
    : ExtensionChain(Type::Instance<MergeValuesExtension>())
{
}

ReflectedPropertyItem* MergeValuesExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    return GetNext<MergeValuesExtension>()->LookUpItem(node, items);
}

std::shared_ptr<MergeValuesExtension> MergeValuesExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyMerger>();
}

EditorComponentExtension::EditorComponentExtension()
    : ExtensionChain(Type::Instance<EditorComponentExtension>())
{
}

std::unique_ptr<BaseComponentValue> EditorComponentExtension::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    return GetNext<EditorComponentExtension>()->GetEditor(node);
}

std::shared_ptr<EditorComponentExtension> EditorComponentExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyEditorComponent>();
}

} // namespace TArc
} // namespace DAVA