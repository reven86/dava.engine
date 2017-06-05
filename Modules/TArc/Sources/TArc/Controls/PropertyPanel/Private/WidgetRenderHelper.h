#pragma once

#include <QWidget>

namespace DAVA
{
namespace TArc
{
class WidgetRenderHelperPrivate;

class WidgetRenderHelper : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WidgetRenderHelper)
    Q_DISABLE_COPY(WidgetRenderHelper)
public:
    WidgetRenderHelper() = delete;

    QPixmap davaGrab(qreal dpr, const QRect& rectangle = QRect());
};
}
}
