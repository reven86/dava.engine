#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"
#include "TArc/Controls/EmptyWidget.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
Any EmptyComponentValue::GetMultipleValue() const
{
    return Any();
}

bool EmptyComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    return false;
}

ControlProxy* EmptyComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    ControlDescriptorBuilder<EmptyWidget::Fields> descr;
    return new EmptyWidget(descr, wrappersProcessor, model, parent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EmptyComponentValue)
{
    ReflectionRegistrator<EmptyComponentValue>::Begin()
    .End();
}

} // namespace TArc
} // namespace DAVA
