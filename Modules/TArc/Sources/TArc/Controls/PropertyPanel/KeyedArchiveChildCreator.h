#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

namespace DAVA
{
namespace TArc
{
class KeyedArchiveChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const override;
};
} // namespace TArc
} // namespace DAVA