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

    void ShowNotification(QWidget* parent, const NotificationParams& params);
    //supported values is ALIGN_TOP or ALIGN_BOTTOM and ALIGN_LEFT or ALIGN_RIGHT
    void SetLayoutType(uint64 alignment);
    void SetDisplayTimeMs(uint32 displayTimeMS);

private slots:
    void OnCloseClicked(NotificationWidget* notification);
    void OnDetailsClicked(NotificationWidget* notification);
    void OnWidgetDestroyed();
    void OnParentWidgetDestroyed();

private:
    void LayoutWidgets(QWidget* parent);
    void Clear();

    bool eventFilter(QObject* object, QEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

    struct Notification;
    Map<QWidget*, List<Notification>> allNotifications;

    uint64 layoutType = ALIGN_TOP | ALIGN_RIGHT;

    uint32 displayTimeMs = 10000;
    const uint32 maximumDisplayCount = 5;
    QElapsedTimer elapsedTimer;
    QBasicTimer basicTimer;
};
} //namespace TArc
} //namespace DAVA
