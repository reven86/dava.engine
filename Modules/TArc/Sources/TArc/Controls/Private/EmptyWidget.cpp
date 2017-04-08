#include "TArc/Controls/EmptyWidget.h"

#include <QtEvents>

namespace DAVA
{
namespace TArc
{
EmptyWidget::EmptyWidget(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
}

EmptyWidget::EmptyWidget(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), accessor, model, parent)
{
}

void EmptyWidget::UpdateControl(const ControlDescriptor& descriptor)
{
}

void EmptyWidget::mousePressEvent(QMouseEvent* e)
{
    e->accept();
}

void EmptyWidget::mouseReleaseEvent(QMouseEvent* e)
{
    e->accept();
}

void EmptyWidget::mouseMoveEvent(QMouseEvent* e)
{
    e->accept();
}

} // namespace TArc
} // namespace DAVA