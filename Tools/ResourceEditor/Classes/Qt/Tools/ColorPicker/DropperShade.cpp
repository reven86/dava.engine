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


#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QCursor>
#include <QLabel>
#include <QPaintEvent>
#include <QDebug>

#include "../Helpers/MouseHelper.h"


namespace
{
    const int cCursorRadius = 151;  // Should be odd
}


DropperShade::DropperShade( const QImage& src, const QRect& rect )
: QWidget(NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip)
    , cache(src)
    , cursorSize(cCursorRadius, cCursorRadius)
    , zoomFactor(1)
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
    p.drawImage(0, 0, cache);
    if (drawCursor)
    {
        DrawCursor(cursorPos, &p);
    }
}

void DropperShade::DrawCursor(const QPoint& _pos, QPainter* p)
{
    const int scale = static_cast<int>(cache.devicePixelRatio());
    const QColor c = GetPixel(_pos);

    const int zf = (zoomFactor * 2 + 1);
    const QRect rcVirtual(
        _pos.x() - cursorSize.width() / 2,
        _pos.y() - cursorSize.height() / 2,
        cursorSize.width(),
        cursorSize.height());
    const QRect rcReal(
        (_pos.x() - cursorSize.width() / 2 / zf) * scale,
        (_pos.y() - cursorSize.height() / 2 / zf) * scale,
        (rcVirtual.width() / zf) * scale + 1,
        (rcVirtual.height() / zf) * scale + 1);

    const QImage& crop = cache.copy(rcReal);
    const QImage& scaled = crop.scaled(cursorSize.width() * scale, cursorSize.height() * scale, Qt::KeepAspectRatio, Qt::FastTransformation);

    p->drawImage(rcVirtual.topLeft(), scaled);

    int xl = rcVirtual.left();
    int xm = rcVirtual.center().x();
    int xr = rcVirtual.right();
    int yt = rcVirtual.top();
    int ym = rcVirtual.center().y();
    int yb = rcVirtual.bottom();

    p->setPen( QPen( Qt::black, 1.0 ) );
    p->drawLine(xl, yt, xr, yt);
    p->drawLine(xl, yb, xr, yb);
    p->drawLine(xl, yt, xl, yb);
    p->drawLine(xr, yt, xr, yb);
    
    p->drawLine(xl, ym, xr, ym);
    p->drawLine(xm, yt, xm, yb);

    xl++;
    xr--;
    yt++;
    yb--;

    p->setPen( QPen( Qt::white, 1.0 ) );
    p->drawLine(xl, yt, xr, yt);
    p->drawLine(xl, yb, xr, yb);
    p->drawLine(xl, yt, xl, yb);
    p->drawLine(xr, yt, xr, yb);

    const int size = 2;
    p->fillRect(xm - size, ym - size, size * 2, size * 2, c);
}

QColor DropperShade::GetPixel(const QPoint& pos) const
{
    const int scale = static_cast<int>(cache.devicePixelRatio());
    const QPoint pt(pos.x() * scale, pos.y() * scale);
    const QColor c = cache.pixel(pt);
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
