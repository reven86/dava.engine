#include "TArc/Controls/Widget.h"

#include <QLayout>

namespace DAVA
{
namespace TArc
{
Widget::Widget(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
}

void Widget::SetLayout(QLayout* layout)
{
    setLayout(layout);
}

void Widget::AddControl(ControlProxy* control, Qt::Alignment alignment)
{
    controls.push_back(control);
    QLayout* l = layout();
    DVASSERT(l != nullptr);
    QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(l);
    if (boxLayout != nullptr)
    {
        boxLayout->addWidget(control->ToWidgetCast(), 0, alignment);
    }
    else
    {
        l->addWidget(control->ToWidgetCast());
    }
}

void Widget::ForceUpdate()
{
    std::for_each(controls.begin(), controls.end(), [](ControlProxy* c)
                  {
                      c->ForceUpdate();
                  });
}

void Widget::TearDown()
{
    std::for_each(controls.begin(), controls.end(), [](ControlProxy* c)
                  {
                      c->TearDown();
                  });
}

QWidget* Widget::ToWidgetCast()
{
    return this;
}
} // namespace TArc
} // namespace DAVA
