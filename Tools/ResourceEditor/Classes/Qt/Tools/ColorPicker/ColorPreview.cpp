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


#include "ColorPreview.h"

#include <QPainter>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QCursor>
#include <QApplication>


#include "../Helpers/PaintingHelper.h"
#include "../Helpers/MouseHelper.h"


ColorPreview::ColorPreview(QWidget* parent)
    : QWidget(parent)
    , bgBrush(PaintingHelper::DrawGridBrush(QSize(7, 7)))
    , dragPreviewSize(21, 21)
    , mouse(new MouseHelper(this))
{
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);

    connect(mouse, SIGNAL( mousePress( const QPoint& ) ), SLOT( OnMousePress( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnMouseRelease( const QPoint& ) ));
}

ColorPreview::~ColorPreview()
{
}

void ColorPreview::SetDragPreviewSize(const QSize& _size)
{
    dragPreviewSize = _size;
}

void ColorPreview::SetColorOld(const QColor& c)
{
    cOld = c;
    repaint();
}

void ColorPreview::SetColorNew(const QColor& c)
{
    cNew = c;
    repaint();
}

void ColorPreview::OnMousePress(const QPoint& pos)
{
    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData();

    const QColor c = GetColorAt(pos);
    QPixmap pix(dragPreviewSize);

    pix.fill(c);
    mime->setColorData(c);
    drag->setMimeData(mime);

    drag->setPixmap(pix);
    drag->setHotSpot(QPoint(dragPreviewSize.width() / 2, dragPreviewSize.height() / 2));

    drag->exec(Qt::MoveAction);
}

void ColorPreview::OnMouseRelease(const QPoint& pos)
{
}

void ColorPreview::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);

    QColor cOldS(cOld);
    cOldS.setAlpha(255);
    QColor cNewS(cNew);
    cNewS.setAlpha(255);

    p.fillRect(0, 0, width() -1, height() - 1, bgBrush);
    p.fillRect(OldColorSRect(), cOldS);
    p.fillRect(OldColorRect(), cOld);
    p.fillRect(NewColorSRect(), cNewS);
    p.fillRect(NewColorRect(), cNew);

    p.setPen(Qt::black);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

QColor ColorPreview::GetColorAt(QPoint const& pos) const
{
    QColor cOldS(cOld);
    cOldS.setAlpha(255);
    QColor cNewS(cNew);
    cNewS.setAlpha(255);

    if (OldColorSRect().contains(pos))
    {
        return cOldS;
    }
    if (OldColorRect().contains(pos))
    {
        return cOld;
    }
    if (NewColorSRect().contains(pos))
    {
        return cNewS;
    }
    if (NewColorRect().contains(pos))
    {
        return cNew;
    }

    return QColor();
}

QRect ColorPreview::OldColorSRect() const
{
    return QRect(0, 0, width() / 2, height() / 2);
}

QRect ColorPreview::OldColorRect() const
{
    return QRect(width() / 2, 0, width() / 2, height() / 2);
}

QRect ColorPreview::NewColorSRect() const
{
    return QRect(0, height() / 2, width() / 2, height() / 2);
}

QRect ColorPreview::NewColorRect() const
{
    return QRect(width() / 2, height() / 2, width() / 2, height() / 2);
}
