#include "DropperLens.h"

#include <QPainter>
#include <QCursor>
#include <QDebug>
#include <QLabel>


namespace
{
    const int maxZoom = 10;
    const int cLensRadius = 50;
    const QSize cLensSize(cLensRadius * 2 + 1, cLensRadius * 2 + 1);
}


DropperLens::DropperLens()
    : QWidget(NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip)
    , zoomFactor(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::BlankCursor);
    setFixedSize(cLensSize);

    label = new QLabel(this);
    label->setFixedSize(400, 40);
    label->move(0, 0);

    pixmap = QPixmap(width(), height());
    pixmap.fill(Qt::blue);
}

DropperLens::~DropperLens()
{
}

int DropperLens::ZoomFactor() const
{
    return zoomFactor;
}

QPixmap& DropperLens::preview()
{
    return pixmap;
}

void DropperLens::changeZoom( int delta )
{
    zoomFactor += delta > 0 ? 1 : -1;
    if (zoomFactor < 0)
    {
        zoomFactor = 0;
    }
    if (zoomFactor > maxZoom)
    {
        zoomFactor = maxZoom;
    }
}

void DropperLens::moveTo(const QPoint& pos)
{
    QPoint pt(pos);
    pt.rx() -= width() / 2;
    pt.ry() -= height() / 2;

    move(pt);
    update();
}

void DropperLens::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);

    p.setPen(QPen(Qt::red, 3.0));
    p.drawRect(0, 0, width() - 1, height() - 1);
}
