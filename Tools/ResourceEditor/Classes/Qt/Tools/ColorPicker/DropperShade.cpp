#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QCursor>
#include <QLabel>
#include <QPaintEvent>

#include "../Helpers/MouseHelper.h"


namespace
{
    const int cCursorRadius = 70;
}


DropperShade::DropperShade( const QImage& src, const QRect& rect )
: QWidget(NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip)
    , cache(src)
    , cursorSize(cCursorRadius * 2 + 1, cCursorRadius * 2 + 1)
    , zoomFactor(0)
    , mouse(new MouseHelper(this))
    , drawCursor(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
    setFixedSize(rect.size());
    move(rect.topLeft());
    cursorPos = mapFromGlobal(QCursor::pos());

    connect(mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnClicked( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseWheel( int ) ), SLOT( OnMouseWheel( int ) ));
    connect(mouse, SIGNAL( mouseEntered() ), SLOT( OnMouseEnter() ));
    connect(mouse, SIGNAL( mouseLeaved() ), SLOT( OnMouseLeave() ));

    label = new QLabel(this);
    label->setStyleSheet("background: white;");
    label->resize(300, 45);
    label->move(0, 0);
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
    QPainter p(this);
    p.drawImage(e->rect(), cache, e->rect());
    if (drawCursor)
    {
        DrawCursor(cursorPos, &p);
    }
}

void DropperShade::DrawCursor(const QPoint& _pos, QPainter* p)
{
    const int scale = static_cast<int>(cache.devicePixelRatio());
    const QPoint scaledPos(_pos.x() * scale, _pos.y() * scale);
    const QColor c = GetPixel(scaledPos);

    const int zf = zoomFactor * 2 + 1;
    const QRect rcVirtual(
        _pos.x() - cursorSize.width() / 2,
        _pos.y() - cursorSize.height() / 2,
        cursorSize.width(),
        cursorSize.height());
    const QRect rcReal(
        (_pos.x() - cursorSize.width() / zf / 2) * scale,
        (_pos.y() - cursorSize.height() / zf / 2) * scale,
        cursorSize.width() / (zf * scale),
        cursorSize.height() / (zf * scale));

    const QImage& crop = cache.copy(rcReal);
    const QImage& scaled = crop.scaled(cursorSize.width(), cursorSize.height(), Qt::KeepAspectRatio, Qt::FastTransformation);

    p->drawImage(rcVirtual, scaled);

    p->setPen( QPen( Qt::red, 3.0 ) );

    const QString text = QString(
        "Virtual: %1x%2 : %3x%4\n"
        "Scale :%5\n"
        "Real: %6x%7 : %8x%9"
        )
        .arg( rcVirtual.x() ).arg( rcVirtual.y() ).arg( rcVirtual.width() ).arg( rcVirtual.height() )
        .arg( scale )
        .arg( rcReal.x() ).arg( rcReal.y() ).arg( rcReal.width() ).arg( rcReal.height() );

    label->setText(text);
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
    const int max = 10;

    zoomFactor += delta > 0 ? 1 : -1;
    if (zoomFactor < 0)
        zoomFactor = 0;
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
