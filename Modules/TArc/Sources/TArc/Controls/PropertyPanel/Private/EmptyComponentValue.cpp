#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QQmlComponent>

namespace DAVA
{
namespace TArc
{
QQmlComponent* DAVA::TArc::EmptyComponentValue::GetComponent(QQmlEngine* engine) const
{
    static QQmlComponent component;
    return &component;
}

DAVA_REFLECTION_IMPL(EmptyComponentValue)
{
    ReflectionRegistrator<EmptyComponentValue>::Begin()
    .End();
}

} // namespace TArc
} // namespace DAVA
