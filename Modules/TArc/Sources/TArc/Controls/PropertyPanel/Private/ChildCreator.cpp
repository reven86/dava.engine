#include "TArc/Controls/PropertyPanel/Private/ChildCreator.h"
#include "TArc/Controls/PropertyPanel/Private/DefaultPropertyModelExtensions.h"

#include <Logger/Logger.h>

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

std::shared_ptr<PropertyNode> ChildCreator::CreateRoot(Reflection::Field&& reflectedRoot)
{
    std::shared_ptr<PropertyNode> rootNode = MakeRootNode(allocator.get(), std::move(reflectedRoot));
    auto result = propertiesIndex.emplace(rootNode, Vector<std::shared_ptr<PropertyNode>>());
    DVASSERT(result.second == true);
    return rootNode;
}

void ChildCreator::UpdateSubTree(const std::shared_ptr<PropertyNode>& parent)
{
    DVASSERT(parent != nullptr);
    Vector<std::shared_ptr<PropertyNode>> children;
    extensions->ExposeChildren(parent, children);

    auto iter = propertiesIndex.find(parent);
    if (iter == propertiesIndex.end())
    {
        // insert new parent into index and store it's children
        Vector<std::shared_ptr<PropertyNode>>& newItems = propertiesIndex[parent];
        newItems = std::move(children);
        for (size_t i = 0; i < newItems.size(); ++i)
        {
            nodeCreated.Emit(parent, newItems[i]);
        }
    }
    else
    {
        bool childrenVectorsEqual = true;
        Vector<std::shared_ptr<PropertyNode>>& currentChildren = iter->second;
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
                nodeCreated.Emit(parent, currentChildren[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < children.size(); ++i)
            {
                bool isEqual = false;
                try
                {
                    isEqual = *children[i] == *currentChildren[i];
                }
                catch (const Exception& e)
                {
                    Logger::Debug(e.what());
                }
                if (isEqual == false)
                {
                    RemoveNode(currentChildren[i]);
                    std::swap(currentChildren[i], children[i]);
                    nodeCreated.Emit(parent, currentChildren[i]);
                }
            }
        }
    }
}

void ChildCreator::RemoveNode(const std::shared_ptr<PropertyNode>& parent)
{
    auto iter = propertiesIndex.find(parent);
    DVASSERT(iter != propertiesIndex.end());
    const Vector<std::shared_ptr<PropertyNode>>& children = iter->second;
    for (const std::shared_ptr<PropertyNode>& node : children)
    {
        RemoveNode(node);
    }
    nodeRemoved.Emit(parent);
    propertiesIndex.erase(iter);
}

void ChildCreator::Clear()
{
    propertiesIndex.clear();
}

void ChildCreator::SetDevMode(bool isDevMode)
{
    DVASSERT(extensions != nullptr);
    extensions->SetDevelopertMode(isDevMode);
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
