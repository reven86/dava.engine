#pragma once

#include "TArc/Controls/ControlProxy.h"

#include <QVBoxLayout>

namespace DAVA
{
namespace TArc
{
class QtVBoxLayout : QVBoxLayout
{
public:
    QtVBoxLayout() = default;
    QtVBoxLayout(QWidget* parent);

    template <typename T>
    void AddWidget(ControlProxy<T>* control, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
};

template <typename T>
void QtVBoxLayout::AddWidget(ControlProxy<T>* control, int stretch, Qt::Alignment alignment)
{
    addWidget(control->ToWidgetCast(), stretch, alignment);
}

class QtHBoxLayout : QHBoxLayout
{
public:
    QtHBoxLayout() = default;
    QtHBoxLayout(QWidget* parent);

    template <typename T>
    void AddWidget(ControlProxy<T>* control, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
};

template <typename T>
void QtHBoxLayout::AddWidget(ControlProxy<T>* control, int stretch, Qt::Alignment alignment)
{
    addWidget(control->ToWidgetCast(), stretch, alignment);
}

} // namespace TArc
} // namespace DAVA