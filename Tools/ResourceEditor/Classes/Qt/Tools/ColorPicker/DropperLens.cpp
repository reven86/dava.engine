#include "DropperLens.h"

#include <QPainter>
#include <QCursor>


namespace
{
    const int maxZoom = 10;
    const int cLensRadius = 50;
    const QSize cLensSize(cLensRadius * 2 + 1, cLensRadius * 2 + 1);
}


DropperLens::DropperLens()
    : QWidget(NULL, Qt::Window | Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint | Qt::ToolTip*/)
    , zoomFactor(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::BlankCursor);
    setFixedSize(cLensSize);
}

DropperLens::~DropperLens()
{
}

int DropperLens::ZoomFactor() const
{
    return zoomFactor;
}

void DropperLens::changeZoom( int delta )
{
    const int oldZoom = zoomFactor;

    zoomFactor += delta > 0 ? 1 : -1;
    if (zoomFactor < 0)
    {
        zoomFactor = 0;
    }
    if (zoomFactor > maxZoom)
    {
        zoomFactor = maxZoom;
    }

    if (oldZoom != zoomFactor)
    {
        update();
    }
}

void DropperLens::updatePreview(const QPixmap& preview)
{
    pixmap = preview;
    update();
}

void DropperLens::moveTo(const QPoint& pos)
{
    QPoint pt(pos);
    pt.rx() -= width() / 2;
    pt.ry() -= height() / 2;

    move(pt);
}

void DropperLens::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.drawPixmap(QRectF(geometry()), pixmap, QRectF(pixmap.rect()));

    p.setPen(QPen(Qt::red, 3.0));
    p.drawRect(QRectF(geometry()));
}
