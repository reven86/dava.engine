#include "Tarc/Controls/Private/NotificationLayout.h"
#include "TArc/Controls/Private/NotificationWidget.h"

#include <QTimer>
#include <QEvent>

namespace DAVA
{
namespace TArc
{
struct NotificationLayout::NotificationWidgetParams
{
    std::function<void()> callback;
    uint32 remainTimeMs = 0;

    void DecrementTime(uint32 elapsedMs)
    {
        if (elapsedMs > remainTimeMs)
        {
            remainTimeMs = 0;
        }
        else
        {
            remainTimeMs -= elapsedMs;
        }
    }
};

NotificationLayout::NotificationLayout()
    : QObject(nullptr)
    , timer(new QTimer(this))
{
    elapsedTimer.start();
    timer->setInterval(60);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &NotificationLayout::OnTimeout);
}

NotificationLayout::~NotificationLayout()
{
    timer->stop();
    Clear();
}

void NotificationLayout::AddNotificationWidget(QWidget* parent, const NotificationParams& params)
{
    if (notifications.contains(parent) == false)
    {
        parent->installEventFilter(this);
    }

    NotificationWidget* widget = new NotificationWidget(params, parent);
    connect(widget, &NotificationWidget::CloseButtonClicked, this, &NotificationLayout::OnCloseClicked);
    connect(widget, &NotificationWidget::DetailsButtonClicked, this, &NotificationLayout::OnDetailsClicked);
    connect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);

    NotificationWidgetParams widgetParams;
    widgetParams.callback = std::move(params.callback);
    widgetParams.remainTimeMs = displayTimeMs;

    notifications[parent][widget] = widgetParams;

    LayoutWidgets(parent);
}

void NotificationLayout::LayoutWidgets(QWidget* parent)
{
    int32 totalHeight = 0;
    DVASSERT(notifications.contains(parent));
    WindowNotifications& widgets = notifications[parent];

    Vector<NotificationWidget*> widgetsToDisplay;
    uint32 size = std::min(static_cast<uint32>(widgets.size()), maximumDisplayCount);
    widgetsToDisplay.resize(size);

    uint32 position = (layoutType & ALIGN_BOTTOM) ? 0 : (size - 1);
    int32 step = (layoutType & ALIGN_BOTTOM) ? +1 : -1;

    for (WindowNotifications::Iterator notificationIter = widgets.begin(), end = widgets.begin() + size;
         notificationIter != end;
         ++notificationIter)
    {
        widgetsToDisplay[position] = notificationIter.key();
        position += step;
    }

    for (NotificationWidget* widget : widgetsToDisplay)
    {
        if (widget->isVisible() == false)
        {
            widget->show();
        }

        int32 x = (layoutType & ALIGN_LEFT) ? 0 : parent->width() - widget->width();
        int32 y = (layoutType & ALIGN_TOP) ? totalHeight : parent->height() - widget->height() - totalHeight;
        QPoint widegetPos(static_cast<int>(x), static_cast<int>(y));
        //noticationWidget marked as window inside Qt, in this case we need to use global coordinates
        widegetPos = parent->mapToGlobal(widegetPos);
        widget->SetPosition(widegetPos);

        totalHeight += widget->size().height();
    }
}

void NotificationLayout::Clear()
{
    for (WindowNotifications& widgets : notifications)
    {
        for (WindowNotifications::Iterator iter = widgets.begin(); iter != widgets.end(); ++iter)
        {
            QWidget* widget = iter.key();
            disconnect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);
            delete widget;
        }
    }
    notifications.clear();
}

bool NotificationLayout::eventFilter(QObject* object, QEvent* event)
{
    QEvent::Type type = event->type();
    QWidget* sender = qobject_cast<QWidget*>(object);
    if (type == QEvent::Resize || type == QEvent::Move)
    {
        LayoutWidgets(sender);
    }
    return QObject::eventFilter(object, event);
}

void NotificationLayout::OnTimeout()
{
    for (AllNotifications::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        QWidget* parent = iter.key();
        if (parent->isActiveWindow())
        {
            int elapsedMs = elapsedTimer.restart();
            WindowNotifications& widgets = iter.value();
            for (WindowNotifications::Iterator iter = widgets.begin(); iter != widgets.end();)
            {
                NotificationWidget* widget = iter.key();
                NotificationWidgetParams& params = iter.value();
                params.DecrementTime(elapsedMs);
                if (params.remainTimeMs == 0)
                {
                    iter = widgets.erase(iter);
                    delete widget;
                }
                else
                {
                    ++iter;
                }
            }
            return;
        }
    }
}

void NotificationLayout::SetLayoutType(uint64 type)
{
    if (layoutType == type)
    {
        return;
    }

    DVASSERT((type & ALIGN_LEFT || type & ALIGN_RIGHT) && (type & ALIGN_BOTTOM) || (type & ALIGN_TOP));

    layoutType = type;

    //now remove all notifications
    Clear();
}

void NotificationLayout::SetDisplayTimeMs(uint32 displayTimeMs_)
{
    uint32 differenceMs = displayTimeMs_ - displayTimeMs;
    if (differenceMs == 0)
    {
        return;
    }
    displayTimeMs = displayTimeMs_;

    if (differenceMs > 0)
    {
        for (AllNotifications::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
        {
            QWidget* parent = iter.key();
            WindowNotifications& widgets = iter.value();
            for (WindowNotifications::Iterator iter = widgets.begin(); iter != widgets.end(); ++iter)
            {
                NotificationWidgetParams& params = iter.value();
                params.remainTimeMs += differenceMs;
            }
        }
    }
}

void NotificationLayout::OnCloseClicked(NotificationWidget* notification)
{
    delete notification;
}

void NotificationLayout::OnDetailsClicked(NotificationWidget* notification)
{
    QWidget* parent = notification->parentWidget();
    DVASSERT(notifications.contains(parent));
    notifications[parent][notification].callback();

    delete notification;
}

void NotificationLayout::OnWidgetDestroyed()
{
    NotificationWidget* senderWidget = static_cast<NotificationWidget*>(sender());
    for (AllNotifications::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        WindowNotifications& widgets = *iter;
        if (widgets.contains(senderWidget))
        {
            widgets.remove(senderWidget);
        }
        LayoutWidgets(iter.key());
        return;
    }
}

} //namespace DAVA
} //namespace TArc
