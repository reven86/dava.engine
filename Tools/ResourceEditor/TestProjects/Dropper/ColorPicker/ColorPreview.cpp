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

#include "../Helpers/PaintingHelper.h"



ColorPreview::ColorPreview(QWidget *parent)
    : QWidget(parent)
    , bgBrush( PaintingHelper::DrawGridBrush( QSize( 7, 7 ) ) )
{
}

ColorPreview::~ColorPreview()
{
}

void ColorPreview::SetColorOld( const QColor& c )
{
    cOld = c;
    repaint();
}

void ColorPreview::SetColorNew( const QColor& c )
{
    cNew = c;
    repaint();
}

void ColorPreview::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter p( this );

    QColor cOldS( cOld );
    cOldS.setAlpha( 255 );
    QColor cNewS( cNew );
    cNewS.setAlpha( 255 );

    const int x1 = 0;
    const int y1 = 0;
    const int x2 = width() / 2;
    const int y2 = height() / 2;
    const int w = x2;
    const int h = y2;

    p.fillRect( x2, y1, w, height() - 1, bgBrush );
    p.fillRect( x1, y1, w, h, cOldS );
    p.fillRect( x2, y1, w, h, cOld );
    p.fillRect( x1, y2, w, h, cNewS );
    p.fillRect( x2, y2, w, h, cNew );

    p.setPen( Qt::black );
    p.drawRect( x1, y1, width() - 1, height() - 1 );
}
