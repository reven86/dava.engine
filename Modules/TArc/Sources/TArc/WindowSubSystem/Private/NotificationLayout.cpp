#include "Tarc/WindowSubSystem/Private/NotificationLayout.h"
#include "TArc/WindowSubSystem/Private/NotificationWidget.h"

namespace DAVA
{
namespace TArc
{
NotificationLayout::NotificationLayout() = default;

NotificationLayout::~NotificationLayout()
{
    for (QMap<QWidget*, NotificationList>::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        NotificationList& widgets = iter.value();
        for (NotificationList::Iterator notificationIter = widgets.begin(); notificationIter != widgets.end(); ++notificationIter)
        {
            NotificationWidget* widget = *notificationIter;
            delete widget;
        }
    }
}

void NotificationLayout::AddNotificationWidget(QWidget* parent, const NotificationWidgetParams& params)
{
    NotificationWidget* widget = new NotificationWidget(params, parent);
    connect(widget, &NotificationWidget::Remove, this, &NotificationLayout::RemoveWidget);
    widget->Show();
    notifications[parent].append(widget);
    LayoutWidgets();
}

void NotificationLayout::RemoveWidget()
{
    for (QMap<QWidget*, NotificationList>::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        NotificationList& widgets = *iter;
        QObject* senderWidget = sender();
        NotificationWidget* widget = qobject_cast<NotificationWidget*>(senderWidget);
        widgets.removeAll(widget);
    }
    LayoutWidgets();
}

void NotificationLayout::LayoutWidgets()
{
    for (QMap<QWidget*, NotificationList>::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        int y = 0;
        NotificationList& widgets = iter.value();
        for (NotificationList::Iterator notificationIter = widgets.begin(); notificationIter != widgets.end(); ++notificationIter)
        {
            NotificationWidget* widget = *notificationIter;
            QWidget* parent = iter.key();
            int x = parent->width() - widget->width();
            widget->SetPosition(QPoint(x, y));
            y += widget->size().height();
        }
    }
}
} //namespace DAVA
} //namespace TArc
