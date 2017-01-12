#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace TArc
{
class DefaultChildCheatorExtension : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const override;
};

class DefaultMergeValueExtension : public MergeValuesExtension
{
public:
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const override;
};

class DefaultEditorComponentExtension : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override;
};

std::shared_ptr<IChildAllocator> CreateDefaultAllocator();
} // namespace TArc
} // namespace DAVA
