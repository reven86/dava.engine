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


#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QWidget>
#include <QMap>

#include "../Widgets/AbstractSlider.h"


class GradientSlider
    : public AbstractSlider
{
    Q_OBJECT

public:
    explicit GradientSlider(QWidget *parent = NULL);
    ~GradientSlider();

    void SetColors( const QColor& c1, const QColor& c2 );
    void SetDimensions( const Qt::Edges& flags );
    void SetOrientation( Qt::Orientation orientation );
    void SetOffsets( int l, int t, int r, int b );

    double GetValue() const;
    void SetValue( double val );

protected:
    void DrawBackground( QPainter *p ) const override;
    void DrawForeground( QPainter *p ) const override;
    QRect PosArea() const override;

    void resizeEvent( QResizeEvent* e ) override;

private:
    void drawArrow( Qt::Edge arrow, QPainter *p ) const;

    QColor c1;
    QColor c2;
    QSize arrowSize;
    Qt::Edges arrows;
    Qt::Orientation orientation;
    int ofsL;
    int ofsR;
    int ofsT;
    int ofsB;
    const QBrush bgBrush;
    mutable QPixmap bgCache;
    mutable QMap< Qt::Edge, QPixmap > arrowCache;
};


#endif // GRADIENTSLIDER_H
