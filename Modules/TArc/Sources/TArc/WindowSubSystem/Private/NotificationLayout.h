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
    enum eLayoutType
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    void AddNotificationWidget(QWidget* parent, const NotificationWidgetParams& params);

private:
    void SetLayoutTyle(eLayoutType type);
    void RemoveWidget();
    void LayoutWidgets();

    void LayoutToTopLeft(int totalHeight, NotificationWidget* widget, QWidget* parent);
    void LayoutToTopRight(int totalHeight, NotificationWidget* widget, QWidget* parent);
    void LayoutToBottonLeft(int totalHeight, NotificationWidget* widget, QWidget* parent);
    void LayoutToBottomRight(int totalHeight, NotificationWidget* widget, QWidget* parent);

    bool eventFilter(QObject* object, QEvent* event);

    using NotificationList = QList<NotificationWidget*>;
    using NotificationListMap = QMap<QWidget*, NotificationList>;
    NotificationListMap notifications;
    eLayoutType layoutType = TopLeft;
};
} //namespace TArc
} //namespace DAVA
