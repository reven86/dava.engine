#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QQmlComponent>

namespace DAVA
{
namespace TArc
{

DAVA_REFLECTION_IMPL(EmptyComponentValue)
{
    ReflectionRegistrator<EmptyComponentValue>::Begin()
    .End();
}

} // namespace TArc
} // namespace DAVA
