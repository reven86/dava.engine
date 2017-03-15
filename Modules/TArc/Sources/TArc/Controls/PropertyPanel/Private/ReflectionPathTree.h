#pragma once

#include <TArc/Controls/PropertyPanel/Private/ObjectsPool.h>

#include <Base/FastName.h>

namespace DAVA
{
namespace TArc
{
class PropertiesItem;
class ReflectionPathTree final
{
public:
    ReflectionPathTree(const FastName& rootName);
    ~ReflectionPathTree();

    void AddLeaf(List<FastName>&& leafPath);
    void RemoveLeaf(List<FastName>&& leafPath);

    bool PushRoot(const FastName& newRootName) const;
    bool HasChildInCurrentRoot(const FastName& childName) const;
    void PopRoot() const;

    void Load(const PropertiesItem& settingsNode);
    void Save(PropertiesItem& settingsNode) const;

private:
    enum Flags
    {
        HasChildren,
        Removed,

        FlagsCount
    };

    struct Node
    {
        Node() = default;

        FastName name;
        Bitset<FlagsCount> flags;

        Vector<std::shared_ptr<Node>> children;
    };

    void Load(const PropertiesItem& settingsNode, std::shared_ptr<Node> node);
    void Save(PropertiesItem& settingsNode, std::shared_ptr<Node> node) const;
    void PushRoot(std::shared_ptr<Node> newRoot) const;

    std::shared_ptr<Node> CreateNode(const FastName& name);

    mutable Stack<std::shared_ptr<Node>> root;
    ObjectsPool<Node, SingleThreadStrategy> objectsPool;
};
} // namespace TArc
} // namespace DAVA