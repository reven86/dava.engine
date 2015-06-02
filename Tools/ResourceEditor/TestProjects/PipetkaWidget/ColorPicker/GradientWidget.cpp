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


#include "GradientWidget.h"

#include <QPainter>
#include "PaintingHelper.h"


GradientWidget::GradientWidget( QWidget *parent )
    : QWidget( parent )
    , hor( true )
    , ver( false )
    , fillBg( true )
    , gridSize( 6, 6 )
{
}

GradientWidget::~GradientWidget()
{
}

void GradientWidget::SetColorRange( QColor const& start, QColor const& stop )
{
    startColor = start;
    stopColor = stop;
}

void GradientWidget::SetRenderDimensions( bool _hor, bool _ver )
{
    if ( hor != _hor || ver != _ver )
    {
        hor = _hor;
        ver = _ver;

        cacheBg = QPixmap();
        update();
    }
}

void GradientWidget::SetBgPadding( int left, int top, int right, int bottom )
{
    paddingOfs.left = left;
    paddingOfs.top = top;
    paddingOfs.right = right;
    paddingOfs.bottom = bottom;
}

void GradientWidget::SetGrid( bool enabled, const QSize& _size )
{
    if ( _size == QSize() )
    {
        enabled = false;
    }

    const bool reset = ( enabled != fillBg || _size != gridSize );
    fillBg = enabled;
    gridSize = _size;
    if ( reset )
    {
        cacheBg = QPixmap();
        update();
    }
}

QColor GradientWidget::GetColorAt( QPoint const& pos ) const
{
    drawBackground();   // Refresh cache;
    QRgb data = cacheBgImage.pixel( pos );

    return QColor(data);
}

QPixmap GradientWidget::drawBackground() const
{
    if ( cacheBg.isNull() )
    {
        const int horPadding = paddingOfs.left + paddingOfs.right;
        const int verPadding = paddingOfs.top + paddingOfs.bottom;
        QSize actualSize( size().width() - horPadding, size().height() - verPadding );
        
        const QImage& bg = PaintingHelper::BuildGradient( actualSize, startColor, stopColor, hor, ver );
        QImage fullBg( actualSize, QImage::Format_ARGB32 );
        fullBg.fill( Qt::transparent );

        const QRect rc( QPoint( 0, 0 ), actualSize );
        QPainter p( &fullBg );
        if ( fillBg )
        {
            const QBrush& bgBrush = PaintingHelper::BuildGridBrush( QSize( 5, 5 ) );
            p.fillRect( rc, bgBrush );
        }
        p.drawImage( QPoint( 0, 0 ), bg );

        cacheBgImage = fullBg;
        cacheBg = QPixmap::fromImage( fullBg );
    }

    return cacheBg;
}

QPixmap GradientWidget::drawContent() const
{
    return QPixmap();
}

GradientWidget::Offset const& GradientWidget::padding() const
{
    return paddingOfs;
}

void GradientWidget::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter p( this );
    const QPixmap& bg = drawBackground();
    const QPixmap& fg = drawContent();
    p.drawPixmap( paddingOfs.left, paddingOfs.top, bg );
    p.drawPixmap( 0, 0, fg );
}

void GradientWidget::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    cacheBg = QPixmap();
}
