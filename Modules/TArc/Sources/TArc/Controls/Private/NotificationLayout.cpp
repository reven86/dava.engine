#include "Tarc/Controls/Private/NotificationLayout.h"
#include "TArc/Controls/Private/NotificationWidget.h"

#include <QEvent>
#include <QPropertyAnimation>

namespace DAVA
{
namespace TArc
{
struct NotificationLayout::NotificationWidgetParams
{
    void InitAnimation(NotificationWidget* target)
    {
        positionAnimation = new QPropertyAnimation(target, "position", target);
        positionAnimation->setEasingCurve(QEasingCurve::OutExpo);
        const int durationTimeMs = 150;
        positionAnimation->setDuration(durationTimeMs);
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

    std::function<void()> callback;
    uint32 remainTimeMs = 0;
    QPropertyAnimation* positionAnimation = nullptr;
};

NotificationLayout::NotificationLayout()
    : QObject(nullptr)
{
    basicTimer.start(60, this);
}

NotificationLayout::~NotificationLayout()
{
    Clear();
}

void NotificationLayout::AddNotificationWidget(QWidget* parent, const NotificationParams& params)
{
    if (notifications.contains(parent) == false)
    {
        parent->installEventFilter(this);
        connect(parent, &QObject::destroyed, this, &NotificationLayout::OnParentWidgetDestroyed);
    }

    NotificationWidget* widget = new NotificationWidget(params, parent);
    connect(widget, &NotificationWidget::CloseButtonClicked, this, &NotificationLayout::OnCloseClicked);
    connect(widget, &NotificationWidget::DetailsButtonClicked, this, &NotificationLayout::OnDetailsClicked);
    connect(widget, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);

    NotificationWidgetParams widgetParams;
    widgetParams.InitAnimation(widget);

    widgetParams.callback = std::move(params.callback);
    widgetParams.remainTimeMs = displayTimeMs;

    notifications[parent].emplace_back(widget, widgetParams);

    LayoutWidgets(parent);
}

void NotificationLayout::LayoutWidgets(QWidget* parent)
{
    int32 totalHeight = 0;
    DVASSERT(notifications.contains(parent));
    WindowNotifications& widgets = notifications[parent];

    uint32 size = std::min(static_cast<uint32>(widgets.size()), maximumDisplayCount);
    Vector<NotificationPair> widgetsToDisplay(widgets.begin(), widgets.begin() + size);
    if (layoutType & ALIGN_TOP)
    {
        std::reverse(widgetsToDisplay.begin(), widgetsToDisplay.end());
    }

    for (const NotificationPair& pair : widgetsToDisplay)
    {
        NotificationWidget* widget = pair.first;
        bool justCreated = false;
        if (widget->isVisible() == false)
        {
            justCreated = true;
            widget->show();
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
            QPropertyAnimation* positionAnimation = pair.second.positionAnimation;
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
    for (WindowNotifications& widgets : notifications)
    {
        for (WindowNotifications::iterator iter = widgets.begin(); iter != widgets.end(); ++iter)
        {
            QWidget* widget = iter->first;
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

void NotificationLayout::timerEvent(QTimerEvent* /*event*/)
{
    int elapsedMs = elapsedTimer.restart();
    for (AllNotifications::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        QWidget* parent = iter.key();
        if (parent->isActiveWindow())
        {
            bool needLayout = false;
            WindowNotifications& widgets = iter.value();
            for (WindowNotifications::iterator iter = widgets.begin(); iter != widgets.end();)
            {
                NotificationWidget* widget = iter->first;
                NotificationWidgetParams& params = iter->second;
                params.DecrementTime(elapsedMs);
                if (params.remainTimeMs == 0)
                {
                    needLayout = true;
                    iter = widgets.erase(iter);
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
            for (WindowNotifications::iterator iter = widgets.begin(); iter != widgets.end(); ++iter)
            {
                NotificationWidgetParams& params = iter->second;
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
    WindowNotifications& widgets = notifications[parent];
    WindowNotifications::iterator iter = std::find_if(widgets.begin(), widgets.end(), [notification](const NotificationPair& pair) {
        return pair.first == notification;
    });
    DVASSERT(iter != widgets.end());
    iter->second.callback();

    delete notification;
}

void NotificationLayout::OnWidgetDestroyed()
{
    NotificationWidget* notification = static_cast<NotificationWidget*>(sender());
    for (AllNotifications::Iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
    {
        WindowNotifications& widgets = *iter;
        WindowNotifications::iterator widgetsIter = std::find_if(widgets.begin(), widgets.end(), [notification](const NotificationPair& pair) {
            return pair.first == notification;
        });
        if (widgetsIter != widgets.end())
        {
            widgets.erase(widgetsIter);
            LayoutWidgets(iter.key());
            return;
        }
    }
}

void NotificationLayout::OnParentWidgetDestroyed()
{
    QWidget* senderWidget = static_cast<QWidget*>(sender());
    notifications.remove(senderWidget);
}

} //namespace DAVA
} //namespace TArc
