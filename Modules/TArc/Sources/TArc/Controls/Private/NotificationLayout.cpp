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
        const int durationTimeMs = 100;
        positionAnimation = new QPropertyAnimation(target, "position", target);
        positionAnimation->setDuration(durationTimeMs);

        opacityAnimation = new QPropertyAnimation(target, "opacity", target);
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
    NotificationWidget* notification = nullptr;
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

void NotificationLayout::AddNotificationWidget(QWidget* parent, const NotificationParams& params)
{
    auto notificationsIter = notifications.find(parent);
    if (notificationsIter == notifications.end())
    {
        parent->installEventFilter(this);
        connect(parent, &QObject::destroyed, this, &NotificationLayout::OnParentWidgetDestroyed);
    }

    NotificationWidget* notification = new NotificationWidget(params, parent);
    connect(notification, &NotificationWidget::CloseButtonClicked, this, &NotificationLayout::OnCloseClicked);
    connect(notification, &NotificationWidget::DetailsButtonClicked, this, &NotificationLayout::OnDetailsClicked);
    connect(notification, &QObject::destroyed, this, &NotificationLayout::OnWidgetDestroyed);

    NotificationWidgetParams widgetParams;
    widgetParams.notification = notification;
    widgetParams.InitAnimation(notification);

    widgetParams.callback = std::move(params.callback);
    widgetParams.remainTimeMs = displayTimeMs;

    notifications[parent].push_back(widgetParams);

    LayoutWidgets(parent);
}

void NotificationLayout::LayoutWidgets(QWidget* parent)
{
    int32 totalHeight = 0;
    auto notificationsIter = notifications.find(parent);
    DVASSERT(notificationsIter != notifications.end());
    ParameterList& parameterList = notificationsIter->second;

    uint32 size = std::min(static_cast<uint32>(parameterList.size()), maximumDisplayCount);

    auto endIter = parameterList.begin();
    std::advance(endIter, size);
    List<NotificationWidgetParams> paramsToDisplay(parameterList.begin(), endIter);
    if (layoutType & ALIGN_TOP)
    {
        std::reverse(paramsToDisplay.begin(), paramsToDisplay.end());
    }

    for (const NotificationWidgetParams& params : paramsToDisplay)
    {
        NotificationWidget* widget = params.notification;
        bool justCreated = false;
        if (widget->isVisible() == false)
        {
            justCreated = true;

            widget->show();
            QPropertyAnimation* opacityAnimation = params.opacityAnimation;
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
            QPropertyAnimation* positionAnimation = params.positionAnimation;
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
    for (auto& widgetParamsPair : notifications)
    {
        for (NotificationWidgetParams& params : widgetParamsPair.second)
        {
            QWidget* widget = params.notification;
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
    for (auto& widgetParamsPair : notifications)
    {
        QWidget* parent = widgetParamsPair.first;
        if (parent->isActiveWindow())
        {
            bool needLayout = false;
            ParameterList& parameterList = widgetParamsPair.second;
            for (ParameterList::iterator iter = parameterList.begin(); iter != parameterList.end();)
            {
                NotificationWidget* widget = iter->notification;
                if (widget->isVisible() == false)
                {
                    ++iter;
                    continue;
                }
                iter->DecrementTime(elapsedMs);
                if (iter->remainTimeMs == 0)
                {
                    needLayout = true;
                    iter = parameterList.erase(iter);
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
        for (auto& widgetParamsPair : notifications)
        {
            ParameterList& paramList = widgetParamsPair.second;
            for (NotificationWidgetParams& params : paramList)
            {
                params.remainTimeMs += differenceMs;
            }
        }
    }
    displayTimeMs = displayTimeMs_;
}

void NotificationLayout::OnCloseClicked(NotificationWidget* notification)
{
    delete notification;
}

void NotificationLayout::OnDetailsClicked(NotificationWidget* notification)
{
    QWidget* parent = notification->parentWidget();
    auto iter = notifications.find(parent);
    DVASSERT(iter != notifications.end());
    ParameterList& paramList = iter->second;
    ParameterList::iterator paramListIter = std::find_if(paramList.begin(), paramList.end(), [notification](const NotificationWidgetParams& params) {
        return params.notification == notification;
    });
    DVASSERT(paramListIter != paramList.end());
    paramListIter->callback();

    delete notification;
}

void NotificationLayout::OnWidgetDestroyed()
{
    NotificationWidget* notification = static_cast<NotificationWidget*>(sender());
    for (auto& widgetParamsPair : notifications)
    {
        ParameterList& paramList = widgetParamsPair.second;
        ParameterList::iterator paramListIter = std::find_if(paramList.begin(), paramList.end(), [notification](const NotificationWidgetParams& params) {
            return params.notification == notification;
        });
        if (paramListIter != paramList.end())
        {
            paramList.erase(paramListIter);
            LayoutWidgets(widgetParamsPair.first);
            return;
        }
    }
}

void NotificationLayout::OnParentWidgetDestroyed()
{
    QWidget* senderWidget = static_cast<QWidget*>(sender());
    notifications.erase(senderWidget);
}

} //namespace DAVA
} //namespace TArc
