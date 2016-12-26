#include "TArc/Controls/PropertyPanel/Private/ChildCreator.hpp"
#include "TArc/Controls/PropertyPanel/DefaultPropertyModelExtensions.hpp"

namespace DAVA
{
namespace TArc
{
ChildCreator::ChildCreator()
    : extensions(ChildCreatorExtension::CreateDummy())
    , allocator(CreateDefaultAllocator())
{
}

ChildCreator::~ChildCreator()
{
    DVASSERT(propertiesIndex.empty());
}

std::shared_ptr<const PropertyNode> ChildCreator::CreateRoot(const Reflection& reflectedRoot)
{
    auto result = propertiesIndex.emplace(MakeRootNode(allocator.get()), std::vector<std::shared_ptr<const PropertyNode>>());
    DVASSERT(result.second == true);
    return result.first->first;
}

void ChildCreator::UpdateSubTree(const std::shared_ptr<const PropertyNode>& parent)
{
    DVASSERT(parent != nullptr);
    std::vector<std::shared_ptr<const PropertyNode>> children;
    extensions->ExposeChildren(parent, children);

    auto iter = propertiesIndex.find(parent);
    if (iter == propertiesIndex.end())
    {
        // insert new parent into index and store it's children
        std::vector<std::shared_ptr<const PropertyNode>>& newItems = propertiesIndex[parent];
        newItems = std::move(children);
        for (size_t i = 0; i < newItems.size(); ++i)
        {
            nodeCreated.Emit(parent, newItems[i], i);
        }
    }
    else
    {
        bool childrenVectorsEqual = true;
        std::vector<std::shared_ptr<const PropertyNode>>& currentChildren = iter->second;
        if (children.size() != currentChildren.size())
        {
            // if size isn't equal, current parent is a collection, this collection was modified and we should rebuild whole subtree
            for (auto iter = currentChildren.rbegin(); iter != currentChildren.rend(); ++iter)
            {
                RemoveNode(*iter);
            }

            std::swap(children, currentChildren);
            for (size_t i = 0; i < currentChildren.size(); ++i)
            {
                nodeCreated.Emit(parent, currentChildren[i], i);
            }
        }
        else
        {
            for (size_t i = 0; i < children.size(); ++i)
            {
                if (children[i] != currentChildren[i])
                {
                    RemoveNode(currentChildren[i]);
                    std::swap(currentChildren[i], children[i]);
                    nodeCreated.Emit(parent, currentChildren[i], i);
                }
            }
        }
    }
}

void ChildCreator::RemoveNode(const std::shared_ptr<const PropertyNode>& parent)
{
    auto iter = propertiesIndex.find(parent);
    DVASSERT(iter != propertiesIndex.end());
    const std::vector<std::shared_ptr<const PropertyNode>>& children = iter->second;
    for (const std::shared_ptr<const PropertyNode>& node : children)
    {
        RemoveNode(node);
    }
    propertiesIndex.erase(iter);
    nodeRemoved.Emit(parent);
    DVASSERT(parent.unique());
}

void ChildCreator::Clear()
{
    propertiesIndex.clear();
}

void ChildCreator::RegisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension)
{
    extension->SetAllocator(allocator);
    extensions = std::static_pointer_cast<ChildCreatorExtension>(ChildCreatorExtension::AddExtension(extensions, extension));
}

void ChildCreator::UnregisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension)
{
    extension->SetAllocator(nullptr);
    extensions = std::static_pointer_cast<ChildCreatorExtension>(ChildCreatorExtension::RemoveExtension(extensions, extension));
}

} // namespace TArc
} // namespace DAVA
