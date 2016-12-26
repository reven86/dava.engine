#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.hpp"

namespace DAVA
{
namespace TArc
{
class DefaultChildCheatorExtension: public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>& node, std::vector<std::shared_ptr<const PropertyNode>> & children) const override;
};

class DefaultMergeValueExtension: public MergeValuesExtension
{
public:
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const std::vector<std::unique_ptr<ReflectedPropertyItem>>& items) const override;
};

std::shared_ptr<IChildAllocator> CreateDefaultAllocator();
} // namespace TArc
} // namespace DAVA

