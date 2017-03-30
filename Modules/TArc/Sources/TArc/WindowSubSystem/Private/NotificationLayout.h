#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/Private/NotificationWidget.h"

#include <QObject>
#include <QMap>

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

    void AddNotificationWidget(QWidget* parent, const NotificationWidgetParams& params);

    void RemoveWidget();

    void LayoutWidgets();

private:
    using NotificationList = QList<NotificationWidget*>;
    QMap<QWidget*, NotificationList> notifications;
};
} //namespace TArc
} //namespace DAVA
