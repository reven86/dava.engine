#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>

#include "../Helpers/MouseHelper.h"


DropperShade::DropperShade()
    : QWidget( NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint )
{
    setAttribute( Qt::WA_DeleteOnClose );
    setFocusPolicy( Qt::WheelFocus );
    setMouseTracking( true );
    setCursor( Qt::BlankCursor );

    QDesktopWidget* desktop = QApplication::desktop();
    const QRect rc = desktop->geometry();

    m_screen = screenShot();
    
    setFixedSize( rc.size() );
    move( rc.topLeft() );
}

DropperShade::~DropperShade()
{
}

void DropperShade::OnMouseMove(const QPoint& pos)
{
}

void DropperShade::OnClicked(const QPoint& pos)
{
}

void DropperShade::OnMouseWheel(int delta)
{
}

void DropperShade::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p( this );

    p.drawPixmap( 0, 0, m_screen );
}

QPixmap DropperShade::screenShot()
{
    QDesktopWidget* desktop = QApplication::desktop();
    QScreen* mainScreen = qApp->primaryScreen();
    const QRect rc = desktop->geometry();
    const QPixmap pix = mainScreen->grabWindow( 0, rc.left(), rc.top(), rc.width(), rc.height() );
    return pix;
}
