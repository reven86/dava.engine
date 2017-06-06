#include "TArc/Controls/PropertyPanel/Private/WidgetRenderHelper.h"

#include <Debug/DVAssert.h>

#include <QtWidgets/private/qwidget_p.h>

namespace DAVA
{
namespace TArc
{
class WidgetRenderHelperPrivate : public QWidgetPrivate
{
};

static void davaSendResizeEvents(QWidget* target)
{
    QResizeEvent e(target->size(), QSize());
    QApplication::sendEvent(target, &e);

    const QObjectList children = target->children();
    for (int i = 0; i < children.size(); ++i)
    {
        QWidget* child = static_cast<QWidget*>(children.at(i));
        if (child->isWidgetType() && !child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent))
            davaSendResizeEvents(child);
    }
}

QPixmap WidgetRenderHelper::davaGrab(qreal dpr, const QRect& rectangle)
{
    Q_D(WidgetRenderHelper);
    if (testAttribute(Qt::WA_PendingResizeEvent) || !testAttribute(Qt::WA_WState_Created))
        davaSendResizeEvents(this);

    const QWidget::RenderFlags renderFlags = QWidget::DrawWindowBackground | QWidget::DrawChildren | QWidget::IgnoreMask;

    const bool oldDirtyOpaqueChildren = d->dirtyOpaqueChildren;
    QRect r(rectangle);
    DVASSERT(r.width() >= 0);
    DVASSERT(r.height() >= 0);

    if (!r.intersects(rect()))
        return QPixmap();

    QPixmap res(r.size() * dpr);
    res.setDevicePixelRatio(dpr);
    if (!d->isOpaque)
        res.fill(Qt::transparent);
    d->render(&res, QPoint(), QRegion(r), renderFlags);

    d->dirtyOpaqueChildren = oldDirtyOpaqueChildren;
    return res;
}
}
}
