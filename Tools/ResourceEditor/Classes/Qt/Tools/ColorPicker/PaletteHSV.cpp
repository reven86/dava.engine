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


#include "PaletteHSV.h"

#include <QPainter>
#include <QDebug>

#include "../Helpers/PaintingHelper.h"


PaletteHSV::PaletteHSV(QWidget* parent)
    : AbstractSlider(parent)
      , cursorSize(8, 8)
{
}

PaletteHSV::~PaletteHSV()
{
}

int PaletteHSV::GetHue() const
{
    const int h = PaintingHelper::HueRC(Pos(), size());
    return h;
}

int PaletteHSV::GetSat() const
{
    const int s = PaintingHelper::SatRC(Pos(), size());
    return s;
}

void PaletteHSV::SetColor(int hue, int sat)
{
    QColor c;

    c.setHsv(hue, sat, PaintingHelper::ValRC(Pos(), size()));
    const QPoint pt = PaintingHelper::GetHSVColorPoint(c, size());
    SetPos(pt);
}

void PaletteHSV::SetColor(const QColor& c)
{
    SetColor(c.hue(), c.saturation());
}

void PaletteHSV::DrawBackground(QPainter* p) const
{
    if (bgCache.isNull())
    {
        const QImage& pal = PaintingHelper::DrawHSVImage(size());
        bgCache = QPixmap::fromImage(pal);
    }

    p->drawPixmap(0, 0, bgCache);
}

void PaletteHSV::DrawForeground(QPainter* p) const
{
    DrawCursor(p);
}

QRect PaletteHSV::PosArea() const
{
    return rect().adjusted(0, 0, -1, -1);
}

void PaletteHSV::resizeEvent(QResizeEvent* e)
{
    bgCache = QPixmap();
    AbstractSlider::resizeEvent(e);
}

void PaletteHSV::DrawCursor(QPainter* p) const
{
    const int sx = cursorSize.width() / 2;
    const int sy = cursorSize.height() / 2;
    QRect rc(QPoint(Pos().x() - sx, Pos().y() - sy), cursorSize);

    const int h = GetHue();
    const int s = GetSat();
    const int v = PaintingHelper::ValRC(Pos(), size());
    QColor c;
    c.setHsv(h, s, v);
    p->fillRect(rc, c);

    p->setPen(Qt::white);
    p->drawRect(rc);
    rc.adjust(-1, -1, 1, 1);
    p->setPen(Qt::black);
    p->drawRect(rc);
}