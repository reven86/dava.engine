#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Controls/Noitifications/NotificationWidget.h"

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

    ~NotificationLayout();

    void AddNotificationWidget(QWidget* parent, const NotificationWidgetParams& params);
    void SetLayoutType(eLayoutType type);
    void SetDisplayTimeMs(DAVA::uint32 displayTimeMS);

private:
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

    DAVA::uint32 displayTimeMs = 6000;
    const DAVA::uint32 maximumDisplayCount = 5;
};
} //namespace TArc
} //namespace DAVA
