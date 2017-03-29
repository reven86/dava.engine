#pragma once

#include "TArc/Controls/ControlProxy.h"

#include <QVBoxLayout>

namespace DAVA
{
namespace TArc
{
class QtVBoxLayout : public QVBoxLayout
{
public:
    QtVBoxLayout() = default;
    QtVBoxLayout(QWidget* parent);

    template <typename T>
    void AddControl(ControlProxyImpl<T>* control, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
};

template <typename T>
void QtVBoxLayout::AddControl(ControlProxyImpl<T>* control, int stretch, Qt::Alignment alignment)
{
    addWidget(control->ToWidgetCast(), stretch, alignment);
}

class QtHBoxLayout : public QHBoxLayout
{
public:
    QtHBoxLayout() = default;
    QtHBoxLayout(QWidget* parent);

    template <typename T>
    void AddControl(ControlProxyImpl<T>* control, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
};

template <typename T>
void QtHBoxLayout::AddControl(ControlProxyImpl<T>* control, int stretch, Qt::Alignment alignment)
{
    addWidget(control->ToWidgetCast(), stretch, alignment);
}

} // namespace TArc
} // namespace DAVA