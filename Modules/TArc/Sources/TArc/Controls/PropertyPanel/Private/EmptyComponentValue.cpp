#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

#include "Reflection/ReflectionRegistrator.h"

#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
QWidget* EmptyComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return nullptr;
}

void EmptyComponentValue::ReleaseEditorWidget(QWidget* editor, const QModelIndex& index)
{
}

void EmptyComponentValue::StaticEditorPaint(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options)
{
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);
}

DAVA_REFLECTION_IMPL(EmptyComponentValue)
{
    ReflectionRegistrator<EmptyComponentValue>::Begin()
    .End();
}

} // namespace TArc
} // namespace DAVA
