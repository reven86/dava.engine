#pragma once

#include "PropertyModelExtensions.h"

#include <Base/BaseTypes.h>
#include <Base/Any.h>

namespace DAVA
{
namespace TArc
{
class ValueCompositor
{
public:
    virtual Any Compose(const Vector<std::shared_ptr<PropertyNode>>& nodes) const = 0;
    virtual bool IsValidValue(const Any& value) const = 0;
};
} // namespace TArc
} // namespace DAVA