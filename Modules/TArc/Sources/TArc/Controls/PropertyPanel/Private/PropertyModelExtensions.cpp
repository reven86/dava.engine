#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.hpp"

namespace DAVA
{
namespace TArc
{
namespace PMEDetails
{
class DummyChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>&, std::vector<std::shared_ptr<const PropertyNode>>&) const override
    {
    }
};

class DummyMerger : public MergeValuesExtension
{
public:
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const std::vector<std::unique_ptr<ReflectedPropertyItem>>& items) const override
    {
        return nullptr;
    }
};
}

std::shared_ptr<const PropertyNode> MakeRootNode(IChildAllocator* allocator)
{
    return allocator->CreatePropertyNode(DAVA::Reflection(), PropertyNode::SelfRoot);
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

ChildCreatorExtension::ChildCreatorExtension()
    : ExtensionChain(Type::Instance<ChildCreatorExtension>())
{
}

void ChildCreatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, std::vector<std::shared_ptr<const PropertyNode>>& children) const
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

ReflectedPropertyItem* MergeValuesExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const std::vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    return GetNext<MergeValuesExtension>()->LookUpItem(node, items);
}

std::shared_ptr<MergeValuesExtension> MergeValuesExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyMerger>();
}

} // namespace TArc
} // namespace DAVA