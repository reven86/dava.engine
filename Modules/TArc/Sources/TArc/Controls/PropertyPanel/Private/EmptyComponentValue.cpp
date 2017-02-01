#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
QWidget* EmptyComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    return nullptr;
}

void EmptyComponentValue::ReleaseEditorWidget(QWidget* editor)
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(EmptyComponentValue)
{
    ReflectionRegistrator<EmptyComponentValue>::Begin()
    .End();
}

} // namespace TArc
} // namespace DAVA
