#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

namespace DAVA
{
namespace TArc
{
class SubPropertyValueChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const override;
};

class SubPropertyEditorCreator : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override;
};
} // namespace TArc
} // namespace DAVA