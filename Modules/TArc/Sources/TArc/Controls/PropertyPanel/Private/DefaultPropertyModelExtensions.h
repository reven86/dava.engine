#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectionPathTree.h"
#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class UI;
class DefaultChildCheatorExtension : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const override;
};

class DefaultEditorComponentExtension : public EditorComponentExtension
{
public:
    DefaultEditorComponentExtension(UI* ui);
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override;

private:
    UI* ui = nullptr;
};

std::shared_ptr<IChildAllocator> CreateDefaultAllocator();
} // namespace TArc
} // namespace DAVA
