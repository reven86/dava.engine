#pragma once

#include "TArc/WindowSubSystem/UI.h"

#include <Base/BaseTypes.h>

#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include <QBasicTimer>

namespace DAVA
{
namespace TArc
{
class NotificationWidget;

class NotificationLayout : public QObject
{
    Q_OBJECT

public:
    NotificationLayout();
    ~NotificationLayout() override;

    void AddNotificationWidget(QWidget* parent, const NotificationParams& params);
    void SetLayoutType(uint64 align);
    void SetDisplayTimeMs(DAVA::uint32 displayTimeMS);

private slots:
    void OnCloseClicked(NotificationWidget* notification);
    void OnDetailsClicked(NotificationWidget* notification);
    void OnWidgetDestroyed();

private:
    void LayoutWidgets(QWidget* parent);
    void Clear();

    bool eventFilter(QObject* object, QEvent* event);
    void timerEvent(QTimerEvent* event);

    struct NotificationWidgetParams;
    using WindowNotifications = QMap<NotificationWidget*, NotificationWidgetParams>;
    using AllNotifications = QMap<QWidget*, WindowNotifications>;
    AllNotifications notifications;

    uint64 layoutType = ALIGN_TOP | ALIGN_RIGHT;

    uint32 displayTimeMs = 10000;
    const uint32 maximumDisplayCount = 5;
    QElapsedTimer elapsedTimer;
    QBasicTimer basicTimer;
};
} //namespace TArc
} //namespace DAVA
