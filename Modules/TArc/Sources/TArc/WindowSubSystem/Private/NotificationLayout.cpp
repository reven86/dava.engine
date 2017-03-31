#include "Tarc/WindowSubSystem/Private/NotificationLayout.h"
#include "TArc/WindowSubSystem/Private/NotificationWidget.h"

#include <QEvent>

namespace DAVA
{
namespace TArc
{
void NotificationLayout::AddNotificationWidget(QWidget* parent, const NotificationWidgetParams& params)
{
    if (notifications.contains(parent) == false)
    {
        parent->installEventFilter(this);
    }

    NotificationWidget* widget = new NotificationWidget(params, parent);
    connect(widget, &NotificationWidget::Removed, this, &NotificationLayout::RemoveWidget);

    notifications[parent].append(widget);
    LayoutWidgets();
}

void NotificationLayout::RemoveWidget()
{
    for (NotificationListMap::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
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
    for (NotificationListMap::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        QWidget* parent = iter.key();
        const int maxDisplayCount = 5;
        int displayedCount = 0;
        int totalHeight = 0;

        NotificationList& widgets = iter.value();

        DAVA::Vector<NotificationWidget*> widgetsToDisplay;
        int size = std::min(widgets.size(), maxDisplayCount);
        widgetsToDisplay.resize(size);

        int position = (layoutType == BottomLeft || layoutType == BottomRight) ? 0 : (size - 1);
        int step = (layoutType == BottomLeft || layoutType == BottomRight) ? +1 : -1;
        NotificationList::Iterator end = widgets.begin() + size;
        for (NotificationList::Iterator notificationIter = widgets.begin(); notificationIter != end; ++notificationIter)
        {
            widgetsToDisplay[position] = *notificationIter;
            position += step;
        }

        for (NotificationWidget* widget : widgetsToDisplay)
        {
            if (widget->isVisible() == false)
            {
                widget->Add();
            }
            switch (layoutType)
            {
            case TopLeft:
                LayoutToTopLeft(totalHeight, widget, parent);
                break;
            case TopRight:
                LayoutToTopRight(totalHeight, widget, parent);
                break;
            case BottomLeft:
                LayoutToBottonLeft(totalHeight, widget, parent);
                break;
            case BottomRight:
                LayoutToBottomRight(totalHeight, widget, parent);
                break;
            }
            totalHeight += widget->size().height();
        }
    }
}

void NotificationLayout::LayoutToTopLeft(int totalHeight, NotificationWidget* widget, QWidget* parent)
{
    QPoint parentTopLeft = parent->mapToGlobal(QPoint(0, 0));
    int x = parentTopLeft.x();
    int y = parentTopLeft.y() + totalHeight;
    widget->SetPosition(QPoint(x, y));
}

void NotificationLayout::LayoutToTopRight(int totalHeight, NotificationWidget* widget, QWidget* parent)
{
    QPoint parentTopRight = parent->mapToGlobal(QPoint(parent->width(), 0));
    int x = parentTopRight.x() - widget->width();
    int y = parentTopRight.y() + totalHeight;
    widget->SetPosition(QPoint(x, y));
}

void NotificationLayout::LayoutToBottonLeft(int totalHeight, NotificationWidget* widget, QWidget* parent)
{
    QPoint parentBottomLeft = parent->mapToGlobal(QPoint(0, parent->height()));
    int x = parentBottomLeft.x();
    int y = parentBottomLeft.y() - widget->height() - totalHeight;
    widget->SetPosition(QPoint(x, y));
}

void NotificationLayout::LayoutToBottomRight(int totalHeight, NotificationWidget* widget, QWidget* parent)
{
    QPoint parentBottomRight = parent->mapToGlobal(QPoint(parent->width(), parent->height()));
    int x = parentBottomRight.x() - widget->width();
    int y = parentBottomRight.y() - widget->height() - totalHeight;
    widget->SetPosition(QPoint(x, y));
}

bool NotificationLayout::eventFilter(QObject* object, QEvent* event)
{
    QEvent::Type type = event->type();
    if (type == QEvent::Resize || type == QEvent::Move)
    {
        LayoutWidgets();
    }
    return QObject::eventFilter(object, event);
}

void NotificationLayout::SetLayoutTyle(eLayoutType type)
{
    layoutType = type;
    LayoutWidgets();
}

} //namespace DAVA
} //namespace TArc
