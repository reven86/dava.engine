#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QCursor>

#include "../Helpers/MouseHelper.h"


DropperShade::DropperShade( const QImage& src, const QRect& rect )
    : QWidget(NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , cache( src )
    , cursorSize(151, 151)
    , zoomFactor(3)
    , mouse(new MouseHelper(this))
    , drawCursor(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
    setFixedSize( rect.size() );
    move( rect.topLeft() );
    cursorPos = mapFromGlobal(QCursor::pos());

    connect(mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnClicked( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseWheel( int ) ), SLOT( OnMouseWheel( int ) ));
    connect(mouse, SIGNAL( mouseEntered() ), SLOT( OnMouseEnter() ));
    connect(mouse, SIGNAL( mouseLeaved() ), SLOT( OnMouseLeave() ));
}

DropperShade::~DropperShade()
{
}

void DropperShade::SetZoomFactor(int zoom)
{
    if ( (sender() != this) && (zoomFactor != zoom) )
    {
        zoomFactor = zoom;
        update();
    }
}

void DropperShade::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.drawImage(0, 0, cache);
    if (drawCursor)
    {
        DrawCursor(cursorPos, &p);
    }
}

void DropperShade::DrawCursor(const QPoint& pos, QPainter* p)
{
    const int sx = cursorSize.width() / 2 - 1;
    const int sy = cursorSize.height() / 2 - 1;
    const QColor c = GetPixel(pos);

    QRect rc(QPoint(pos.x() - sx, pos.y() - sy), QPoint(pos.x() + sx, pos.y() + sy));

    const int fc = zoomFactor;
    QRect rcZoom(QPoint(pos.x() - sx / fc, pos.y() - sy / fc), QPoint(pos.x() + sx / fc, pos.y() + sy / fc));
    const QImage& zoomed = cache.copy(rcZoom).scaled(rc.size(), Qt::KeepAspectRatio, Qt::FastTransformation);

    p->drawImage(rc, zoomed);
    p->setPen(QPen(Qt::black, 1.0));

    const int midX = (rc.left() + rc.right()) / 2;
    const int midY = (rc.bottom() + rc.top()) / 2;

    p->drawLine(rc.left(), midY, rc.right(), midY);
    p->drawLine(midX, rc.top(), midX, rc.bottom());
    p->fillRect(pos.x() - 1, pos.y() - 1, 3, 3, c);

    p->setPen(Qt::white);
    p->drawRect(rc);
    rc.adjust(-1, -1, 1, 1);
    p->setPen(Qt::black);
    p->drawRect(rc);
}

QColor DropperShade::GetPixel(const QPoint& pos) const
{
    const QColor c = cache.pixel(pos);
    return c;
}

void DropperShade::OnMouseMove(const QPoint& pos)
{
    const int sx = cursorSize.width() / 2;
    const int sy = cursorSize.height() / 2;
    QRect rcOld(QPoint(cursorPos.x() - sx, cursorPos.y() - sy), cursorSize);
    rcOld.adjust(-1, -1, 2, 2);
    QRect rcNew(QPoint(pos.x() - sx, pos.y() - sy), cursorSize);
    rcNew.adjust(-1, -1, 2, 2);

    cursorPos = pos;
    update(rcOld);
    update(rcNew);

    emit moved(GetPixel(pos));
}

void DropperShade::OnClicked(const QPoint& pos)
{
    emit picked(GetPixel(pos));
}

void DropperShade::OnMouseWheel(int delta)
{
    const int old = zoomFactor;

    const int maxDpi = 5;
    const int max = qMin(cursorSize.width() / maxDpi, cursorSize.height() / maxDpi);
    const int sign = delta > 0 ? 1 : -1;
    const double step = (zoomFactor - 1) / 2.0;

    zoomFactor += sign * qMax( int(step), 1 );
    if (zoomFactor < 1)
        zoomFactor = 1;
    if (zoomFactor > max )
        zoomFactor = max;

    if (old != zoomFactor)
    {
        update();
        emit zoonFactorChanged(zoomFactor);
    }
}

void DropperShade::OnMouseEnter()
{
    drawCursor = true;
    setFocus();
    update();
}

void DropperShade::OnMouseLeave()
{
    drawCursor = false;
    update();
}

void DropperShade::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        emit canceled();
    }

    QWidget::keyPressEvent(e);
}
