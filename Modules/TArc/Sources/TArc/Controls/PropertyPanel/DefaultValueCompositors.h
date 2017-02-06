#pragma once

#include "TArc/Controls/PropertyPanel/ValueCompositor.h"

#include <Base/Any.h>

namespace DAVA
{
namespace TArc
{
class EmptyValueCompositor : public ValueCompositor
{
public:
    ~EmptyValueCompositor() override = default;
    Any Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const override;
    bool IsValidValue(const Any& newValue, const Any& currentValue) const override;
};

class TextValueCompositor : public ValueCompositor
{
public:
    ~TextValueCompositor() override = default;
    Any Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const override;
    bool IsValidValue(const Any& newValue, const Any& currentValue) const override;
};

class BoolValueCompositor : public ValueCompositor
{
public:
    Any Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const override;
    bool IsValidValue(const Any& newValue, const Any& currentValue) const override;
};

} // namespace TArc
} // namespace DAVA
