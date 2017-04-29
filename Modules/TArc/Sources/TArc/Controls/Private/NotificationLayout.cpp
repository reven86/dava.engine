#include "TArc/Controls/Private/NotificationLayout.h"
#include "TArc/Controls/Private/NotificationWidget.h"

#include <QEvent>
#include <QPropertyAnimation>

namespace DAVA
{
namespace TArc
{
struct NotificationLayout::Notification
{
    Notification(NotificationWidget* widget_)
        : widget(widget_)
    {
        const int durationTimeMs = 100;
        positionAnimation = new QPropertyAnimation(widget, "position", widget);
        positionAnimation->setDuration(durationTimeMs);

        opacityAnimation = new QPropertyAnimation(widget, "opacity", widget);
        //opacity animation must be longer for better visual effects
        opacityAnimation->setDuration(durationTimeMs * 2);
    }

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
    NotificationWidget* widget = nullptr;
    std::function<void()> callback;
    uint32 remainTimeMs = 0;
    QPropertyAnimation* positionAnimation = nullptr;
    QPropertyAnimation* opacityAnimation = nullptr;
};

NotificationLayout::NotificationLayout()
    : QObject(nullptr)
{
    const int updateIntervalMs = 60;
    basicTimer.start(updateIntervalMs, this);
}

NotificationLayout::~NotificationLayout()
{
    Clear();
}

void NotificationLayout::ShowNotification(QWidget* parent, const NotificationParams& params)
{
    auto notificationsIter = allNotifications.find(parent);
    if (notificationsIter == allNotifications.end())
    {
        parent->installEventFilter(this);
        connect(parent, &QObject::destroyed, this, &NotificationLayout::OnParentWidgetDestroyed);
    }

    NotificationWidget* widget = new NotificationWidget(params, parent);
    connect(widget, &NotificationWidget::CloseButtonClicked, this, &NotificationLayout::OnCloseClicked);
    connect(widget, &NotificationWidget::DetailsButtonClicked, this, &NotificationLayout::OnDetailsClicked);
    connect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);

    Notification widgetParams(widget);
    widgetParams.callback = std::move(params.callback);
    widgetParams.remainTimeMs = displayTimeMs;

    allNotifications[parent].push_back(widgetParams);

    LayoutWidgets(parent);
}

void NotificationLayout::LayoutWidgets(QWidget* parent)
{
    int32 totalHeight = 0;
    auto notificationsIter = allNotifications.find(parent);
    DVASSERT(notificationsIter != allNotifications.end());
    List<Notification>& notifications = notificationsIter->second;

    uint32 size = std::min(static_cast<uint32>(notifications.size()), maximumDisplayCount);
    auto endIter = std::next(notifications.begin(), size);
    List<Notification> notificationsToDisplay(notifications.begin(), endIter);
    if (layoutType & ALIGN_TOP)
    {
        std::reverse(notificationsToDisplay.begin(), notificationsToDisplay.end());
    }

    for (const Notification& notification : notificationsToDisplay)
    {
        NotificationWidget* widget = notification.widget;
        bool justCreated = false;
        if (widget->isVisible() == false)
        {
            justCreated = true;

            widget->show();
            QPropertyAnimation* opacityAnimation = notification.opacityAnimation;
            opacityAnimation->setStartValue(0.0f);
            opacityAnimation->setEndValue(1.0f);
            opacityAnimation->start();
        }

        int32 x = (layoutType & ALIGN_LEFT) ? 0 : (parent->width() - widget->width());
        int32 y = (layoutType & ALIGN_TOP) ? totalHeight : (parent->height() - widget->height() - totalHeight);
        QPoint widgetPos(static_cast<int>(x), static_cast<int>(y));

        //noticationWidget marked as window inside Qt, in this case we need to use global coordinates
        //if not mark it as window - on OS X notification will be behind from RenderWidget
        widgetPos = parent->mapToGlobal(widgetPos);

        if (justCreated)
        {
            widget->move(widgetPos);
        }
        else
        {
            QPropertyAnimation* positionAnimation = notification.positionAnimation;
            positionAnimation->stop();
            positionAnimation->setStartValue(widget->pos());
            positionAnimation->setEndValue(widgetPos);
            positionAnimation->start();
        }

        totalHeight += widget->size().height();
    }
}

void NotificationLayout::Clear()
{
    for (auto& windowNotifications : allNotifications)
    {
        for (Notification& notification : windowNotifications.second)
        {
            QWidget* widget = notification.widget;
            disconnect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);
            delete widget;
        }
    }
    allNotifications.clear();
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

void NotificationLayout::timerEvent(QTimerEvent* /*event*/)
{
    int elapsedMs = elapsedTimer.restart();
    for (auto& windowNotifications : allNotifications)
    {
        QWidget* parent = windowNotifications.first;
        if (parent->isActiveWindow())
        {
            bool needLayout = false;
            List<Notification>& notifications = windowNotifications.second;
            for (List<Notification>::iterator iter = notifications.begin(); iter != notifications.end();)
            {
                NotificationWidget* widget = iter->widget;
                if (widget->isVisible() == false)
                {
                    ++iter;
                    continue;
                }
                iter->DecrementTime(elapsedMs);
                if (iter->remainTimeMs == 0)
                {
                    needLayout = true;
                    iter = notifications.erase(iter);
                    disconnect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);
                    delete widget;
                }
                else
                {
                    ++iter;
                }
            }
            if (needLayout)
            {
                LayoutWidgets(parent);
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
    if (displayTimeMs_ > displayTimeMs)
    {
        uint32 differenceMs = displayTimeMs_ - displayTimeMs;
        for (auto& windowNotifications : allNotifications)
        {
            List<Notification>& notifications = windowNotifications.second;
            for (Notification& notification : notifications)
            {
                notification.remainTimeMs += differenceMs;
            }
        }
    }
    displayTimeMs = displayTimeMs_;
}

void NotificationLayout::OnCloseClicked(NotificationWidget* notification)
{
    delete notification;
}

void NotificationLayout::OnDetailsClicked(NotificationWidget* widget)
{
    QWidget* parent = widget->parentWidget();
    auto iter = allNotifications.find(parent);
    DVASSERT(iter != allNotifications.end());
    List<Notification>& notifications = iter->second;
    List<Notification>::iterator notificationsIter = std::find_if(notifications.begin(), notifications.end(), [widget](const Notification& notification) {
        return notification.widget == widget;
    });
    DVASSERT(notificationsIter != notifications.end());
    notificationsIter->callback();

    delete widget;
}

void NotificationLayout::OnWidgetDestroyed()
{
    NotificationWidget* widget = static_cast<NotificationWidget*>(sender());
    for (auto& windowNotifications : allNotifications)
    {
        List<Notification>& notifications = windowNotifications.second;
        List<Notification>::iterator notificationsIter = std::find_if(notifications.begin(), notifications.end(), [widget](const Notification& notification) {
            return notification.widget == widget;
        });
        if (notificationsIter != notifications.end())
        {
            notifications.erase(notificationsIter);
            LayoutWidgets(windowNotifications.first);
            return;
        }
    }
}

void NotificationLayout::OnParentWidgetDestroyed()
{
    QWidget* senderWidget = static_cast<QWidget*>(sender());
    allNotifications.erase(senderWidget);
}

} //namespace DAVA
} //namespace TArc
