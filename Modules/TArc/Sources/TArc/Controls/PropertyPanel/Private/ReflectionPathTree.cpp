#include "TArc/Controls/PropertyPanel/Private/ReflectionPathTree.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
namespace TArc
{
ReflectionPathTree::ReflectionPathTree(const FastName& rootName)
    : objectsPool(256 * sizeof(Node), 1)
{
    root.push(CreateNode(rootName));
}

ReflectionPathTree::~ReflectionPathTree()
{
    DVASSERT(root.size() == 1);
    root.top()->TearDown();
    root.pop();
}

void ReflectionPathTree::AddLeaf(List<FastName>&& leafPath)
{
    std::shared_ptr<Node> rootNode = root.top();
    FastName name = leafPath.front();
    leafPath.pop_front();
    auto iter = std::find_if(rootNode->children.begin(), rootNode->children.end(), [&name](const std::shared_ptr<Node>& node)
                             {
                                 return node->name == name;
                             });

    if (iter == rootNode->children.end())
    {
        std::shared_ptr<Node> newChild = CreateNode(name);
        rootNode->children.push_back(newChild);
        rootNode->flags[HasChildren] = true;
        PushRoot(newChild);
    }
    else
    {
        (*iter)->flags[Removed] = false;
        PushRoot(*iter);
    }

    if (leafPath.empty() == false)
    {
        AddLeaf(std::move(leafPath));
    }
    PopRoot();
}

void ReflectionPathTree::RemoveLeaf(List<FastName>&& leafPath)
{
    std::shared_ptr<Node> rootNode = root.top();
    FastName name = leafPath.front();
    leafPath.pop_front();

    auto iter = std::find_if(rootNode->children.begin(), rootNode->children.end(), [&name](const std::shared_ptr<Node>& node)
                             {
                                 return node->name == name;
                             });

    if (iter == rootNode->children.end())
    {
        return;
    }

    std::shared_ptr<Node> nodeToRemoveTest = *iter;
    PushRoot(nodeToRemoveTest);
    if (leafPath.empty() == false)
    {
        RemoveLeaf(std::move(leafPath));
    }
    else
    {
        nodeToRemoveTest->flags[Removed] = true;
    }

    bool hasChildren = false;
    for (const std::shared_ptr<Node>& child : rootNode->children)
    {
        if (child->flags[Removed] == false || child->flags[HasChildren] == true)
        {
            hasChildren = true;
            break;
        }
    }

    rootNode->flags[HasChildren] = hasChildren;
    if (hasChildren == false)
    {
        rootNode->TearDown();
    }

    PopRoot();
}

bool ReflectionPathTree::PushRoot(const FastName& newRootName) const
{
    std::shared_ptr<Node> rootNode = root.top();
    auto iter = std::find_if(rootNode->children.begin(), rootNode->children.end(), [&newRootName](const std::shared_ptr<Node>& node)
                             {
                                 return node->name == newRootName;
                             });

    if (iter == rootNode->children.end() || (*iter)->flags[Removed] == true)
    {
        return false;
    }

    PushRoot(*iter);
    return true;
}

void ReflectionPathTree::PushRoot(std::shared_ptr<Node> newRoot) const
{
    root.push(newRoot);
}

bool ReflectionPathTree::HasChildInCurrentRoot(const FastName& childName) const
{
    std::shared_ptr<Node> rootNode = root.top();
    auto iter = std::find_if(rootNode->children.begin(), rootNode->children.end(), [&childName](const std::shared_ptr<Node>& node)
                             {
                                 return node->name == childName;
                             });

    return iter != rootNode->children.end() && (*iter)->flags[Removed] == false;
}

void ReflectionPathTree::PopRoot() const
{
    root.pop();
    DVASSERT(root.empty() == false);
}

std::shared_ptr<ReflectionPathTree::Node> ReflectionPathTree::CreateNode(const FastName& name)
{
    std::shared_ptr<Node> node = objectsPool.RequestObject();
    node->name = name;
    node->flags.reset();
    DVASSERT(node->children.empty());
    return node;
}

void ReflectionPathTree::Load(const PropertiesItem& settingsNode)
{
    DVASSERT(root.size() == 1);
    Load(settingsNode, root.top());
}

void ReflectionPathTree::Save(PropertiesItem& settingsNode) const
{
    DVASSERT(root.size() == 1);
    Save(settingsNode, root.top());
}

void ReflectionPathTree::Load(const PropertiesItem& settingsNode, std::shared_ptr<ReflectionPathTree::Node> node)
{
    node->name = FastName(settingsNode.Get("nodeName", String(node->name.c_str())));
    node->flags = Bitset<FlagsCount>(static_cast<uint32>(settingsNode.Get<int32>("flags", 0)));

    int32 childCount = settingsNode.Get<int32>("childCount", 0);
    node->children.reserve(childCount);
    for (int32 i = 0; i < childCount; ++i)
    {
        std::shared_ptr<Node> childNode = CreateNode(FastName(""));
        node->children.push_back(childNode);

        PropertiesItem childSettingsNode = settingsNode.CreateSubHolder(Format("child_%d", static_cast<int32>(i)));
        Load(childSettingsNode, childNode);
    }
}

void ReflectionPathTree::Save(PropertiesItem& settingsNode, std::shared_ptr<ReflectionPathTree::Node> node) const
{
    settingsNode.Set("nodeName", String(node->name.c_str()));
    settingsNode.Set("flags", static_cast<int32>(node->flags.to_ulong()));
    settingsNode.Set("childCount", static_cast<int32>(node->children.size()));

    for (size_t i = 0; i < node->children.size(); ++i)
    {
        PropertiesItem childSettingsNode = settingsNode.CreateSubHolder(Format("child_%d", static_cast<int32>(i)));
        Save(childSettingsNode, node->children[i]);
    }
}

void ReflectionPathTree::Node::TearDown()
{
    std::for_each(children.begin(), children.end(), [](std::shared_ptr<Node> node) {
        node->TearDown();
    });

    children.clear();
}

} // namespace TArc
} // namespace DAVA