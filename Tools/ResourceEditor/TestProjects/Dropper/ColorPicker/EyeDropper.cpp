/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

#include "../Helpers/MouseHelper.h"


EyeDropper::EyeDropper(QWidget *parent)
    : QWidget( parent, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint )
    , mouse(new MouseHelper(this))
    , cursorSize(69, 69)
{
    setAttribute( Qt::WA_DeleteOnClose );
    setFocusPolicy( Qt::StrongFocus );
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);

    connect( mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnClicked( const QPoint& ) ) );
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::Exec()
{
    CreateShade();
    show();
    setFocus();
    update();
}

void EyeDropper::OnMouseMove( const QPoint& pos )
{
    const int sx = cursorSize.width() / 2;
    const int sy = cursorSize.height() / 2;
    QRect rcOld( QPoint( cursorPos.x() - sx, cursorPos.y() - sy ), cursorSize );
    rcOld.adjust( -1, -1, 2, 2 );
    QRect rcNew( QPoint( pos.x() - sx, pos.y() - sy ), cursorSize );
    rcNew.adjust( -1, -1, 2, 2 );
    const QRect rc = rcOld.united( rcNew );

    cursorPos = pos;
    repaint(rc);

    emit moved(GetPixel(pos));
}

void EyeDropper::OnClicked( const QPoint& pos )
{
    emit picked(GetPixel(pos));
    close();
}

void EyeDropper::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, false );
    p.drawImage( 0, 0, cache );
    DrawCursor( cursorPos, &p );
}

void EyeDropper::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        close();
    }

    QWidget::keyPressEvent(e);
}

void EyeDropper::DrawCursor(const QPoint& pos, QPainter *p)
{
    const int sx = cursorSize.width() / 2 - 1;
    const int sy = cursorSize.height() / 2 - 1;
    const QColor c = GetPixel(pos);

    QRect rc( QPoint( pos.x() - sx, pos.y() - sy ), QPoint( pos.x() + sx, pos.y() + sy ) );

    const int fc = 4;
    QRect rcZoom( QPoint( pos.x() - sx / fc, pos.y() - sy / fc ), QPoint( pos.x() + sx / fc, pos.y() + sy / fc ) );
    const QImage& zoomed = cache.copy( rcZoom ).scaled( rc.size(), Qt::KeepAspectRatio, Qt::FastTransformation );

    p->drawImage( rc, zoomed );
    p->setPen( QPen( Qt::black, 1.0 ) );

    const int midX = ( rc.left() + rc.right() ) / 2;
    const int midY = ( rc.bottom() + rc.top() ) / 2;

    p->drawLine( rc.left(), midY, rc.right(), midY );
    p->drawLine( midX, rc.top(), midX, rc.bottom() );
    p->fillRect( pos.x() - 1, pos.y() - 1, 3, 3, c );

    p->setPen( Qt::white );
    p->drawRect( rc );
    rc.adjust( -1, -1, 1, 1 );
    p->setPen( Qt::black );
    p->drawRect( rc );
}

void EyeDropper::CreateShade()
{
    QDesktopWidget *desktop = QApplication::desktop();
    const int n = desktop->screenCount();
    QRect rc;

    for (int i = 0; i < n; i++)
    {
        const QRect screenRect = desktop->screenGeometry(i);
        rc = rc.united( screenRect );
    }

    cache = QPixmap::grabWindow( QApplication::desktop()->winId(), rc.left(), rc.top(), rc.width(), rc.height() ).toImage();
    resize(rc.size());
    move(rc.topLeft());
    cursorPos = mapFromGlobal( QCursor::pos() );
}

QColor EyeDropper::GetPixel( const QPoint& pos ) const
{
    const QColor c = cache.pixel( pos );
    return c;
}
