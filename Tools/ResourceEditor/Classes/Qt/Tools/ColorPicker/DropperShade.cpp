#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QPaintEvent>
#include <QDebug>

#include "../Helpers/MouseHelper.h"

#include "DropperLens.h"


namespace
{
    const int cMaxZoom = 10;
}



DropperShade::DropperShade()
    : QWidget( NULL, Qt::Window | Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint*/ )
    , mouse(new MouseHelper(this))
{
    setAttribute( Qt::WA_DeleteOnClose );
    setFocusPolicy( Qt::WheelFocus );
    setMouseTracking( true );
    setCursor(Qt::BlankCursor);

    QDesktopWidget* desktop = QApplication::desktop();
    const QRect rc = desktop->geometry();

    screen = screenShot();
    screenData = screen.toImage();
    
    setFixedSize(rc.size());
    move(rc.topLeft());

    lens = new DropperLens(this);
    lens->SetZoomFactor(1);

    connect( mouse, &MouseHelper::mouseMove, this, &DropperShade::OnMouseMove );
    connect( mouse, &MouseHelper::mouseWheel, this, &DropperShade::OnMouseWheel );
}

DropperShade::~DropperShade()
{
}

const QImage& DropperShade::screenImage() const
{
    return screenData;
}

QPoint DropperShade::CursorPos() const
{
    return mouse->Pos();
}

void DropperShade::OnMouseMove(const QPoint& pos)
{
    QPoint pt( pos );
    pt.rx() -= lens->width() / 2;
    pt.ry() -= lens->height() / 2;
    lens->move(pt);
}

void DropperShade::OnClicked(const QPoint& pos)
{
}

void DropperShade::OnMouseWheel(int delta)
{
    int val = lens->ZoomFactor();
    val += (delta > 0 ? 1 : -1);
    if (val < 0)
        val = 0;
    if (val > cMaxZoom)
        val = cMaxZoom;

    lens->SetZoomFactor(val);
}

void DropperShade::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p( this );

    const QRect& updateRect = e->rect();
    p.drawPixmap( updateRect, screen, updateRect );
}

QPixmap DropperShade::screenShot()
{
    QDesktopWidget* desktop = QApplication::desktop();
    QScreen* mainScreen = qApp->primaryScreen();
    const QRect rc = desktop->geometry();
    const QPixmap pix = mainScreen->grabWindow( 0, rc.left(), rc.top(), rc.width(), rc.height() );
    return pix;
}
