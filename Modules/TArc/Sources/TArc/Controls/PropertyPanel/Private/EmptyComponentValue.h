#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class EmptyComponentValue : public BaseComponentValue
{
public:

private:
    DAVA_VIRTUAL_REFLECTION(EmptyComponentValue, BaseComponentValue);
};
}
} // namespace DAVA