#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include "Functional/Signal.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class ChildCreator
{
public:
    ChildCreator();
    ~ChildCreator();

    std::shared_ptr<PropertyNode> CreateRoot(Reflection::Field&& reflectedRoot);
    void UpdateSubTree(const std::shared_ptr<const PropertyNode>& parent);
    void RemoveNode(const std::shared_ptr<PropertyNode>& parent);
    void Clear();

    void RegisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension);
    void UnregisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension);

    Signal<std::shared_ptr<const PropertyNode> /*parent*/, std::shared_ptr<PropertyNode> /*child*/, int32 /*childPosition*/> nodeCreated;
    Signal<std::shared_ptr<PropertyNode> /*child*/> nodeRemoved;

private:
    std::shared_ptr<ChildCreatorExtension> extensions;
    UnorderedMap<std::shared_ptr<const PropertyNode>, Vector<std::shared_ptr<PropertyNode>>> propertiesIndex;
    std::shared_ptr<IChildAllocator> allocator;
};

} // namespace TArc
} // namespace DAVA