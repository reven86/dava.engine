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


#include "ColorPickerHSV.h"

#include <QHBoxLayout>

#include "PaletteHSV.h"
#include "GradientSlider.h"


ColorPickerHSV::ColorPickerHSV(QWidget *parent)
    : AbstractColorPicker(parent)
{
    QHBoxLayout *l = new QHBoxLayout();

    pal = new PaletteHSV( this );
    pal->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    l->addWidget( pal );

    val = new GradientSlider( this );
    val->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum );
    val->setMinimumSize( 30, 0 );
    val->SetDimensions( Qt::LeftEdge | Qt::RightEdge );
    val->SetOrientation( Qt::Vertical );
    val->SetOffsets( 5, 5, 5, 5 );
    l->addWidget( val );

    alpha = new GradientSlider( this );
    alpha->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum );
    alpha->setMinimumSize( 30, 0 );
    alpha->SetDimensions( Qt::LeftEdge | Qt::RightEdge );
    alpha->SetOrientation( Qt::Vertical );
    alpha->SetOffsets( 5, 5, 5, 5 );
    l->addWidget( alpha );

    setLayout( l );
    updateGeometry();

    QObject *editors[] = { pal, val, alpha };

    for ( size_t i = 0; i < sizeof( editors ) / sizeof( *editors ); i++ )
    {
        connect( editors[i], SIGNAL( started( const QPointF& ) ), SIGNAL( begin() ) );
        connect( editors[i], SIGNAL( started( const QPointF& ) ), SLOT( OnChanging() ) );
        connect( editors[i], SIGNAL( changing( const QPointF& ) ), SLOT( OnChanging() ) );
        connect( editors[i], SIGNAL( changed( const QPointF& ) ), SLOT( OnChanged() ) );
        connect( editors[i], SIGNAL( canceled() ), SIGNAL( canceled() ) );
    }

    connect( pal, SIGNAL( started( const QPointF& ) ), SLOT( OnHS() ) );
    connect( pal, SIGNAL( changing( const QPointF& ) ), SLOT( OnHS() ) );
    connect( val, SIGNAL( started( const QPointF& ) ), SLOT( OnVal() ) );
    connect( val, SIGNAL( changing( const QPointF& ) ), SLOT( OnVal() ) );
}

ColorPickerHSV::~ColorPickerHSV()
{
}

void ColorPickerHSV::SetColorInternal( QColor const& c )
{
    QColor min;
    QColor max;
    int h, s, v, a;
    c.getHsv( &h, &s, &v, &a );

    pal->SetColor( h, s );

    min.setHsv( h, s, 0, 255 );
    max.setHsv( h, s, 255, 255 );
    val->SetColors( max, min );
    const double dv = 1.0 - double( v ) / 255.0;
    val->SetValue( dv );

    min.setHsv( h, s, v, 0 );
    max.setHsv( h, s, v, 255 );
    alpha->SetColors( max, min );
    const double da = 1.0 - double( a ) / 255.0;
    alpha->SetValue( da );
}

void ColorPickerHSV::OnChanging()
{
    UpdateColor();
    emit changing( GetColor() );
}

void ColorPickerHSV::OnChanged()
{
    UpdateColor();
    emit changed( GetColor() );
}

void ColorPickerHSV::OnHS()
{
    int h, s, v, a;
    const QColor c = GetColor();
    QColor min;
    QColor max;

    c.getHsv( &h, &s, &v, &a );

    min.setHsv( h, s, 0, 255 );
    max.setHsv( h, s, 255, 255 );
    val->SetColors( max, min );

    min.setHsv( h, s, v, 0 );
    max.setHsv( h, s, v, 255 );
    alpha->SetColors( max, min );
}

void ColorPickerHSV::OnVal()
{
    int h, s, v, a;
    const QColor c = GetColor();
    QColor min;
    QColor max;

    c.getHsv( &h, &s, &v, &a );

    min.setHsv( h, s, v, 0 );
    max.setHsv( h, s, v, 255 );
    alpha->SetColors( max, min );
}

void ColorPickerHSV::OnAlpha()
{
}

void ColorPickerHSV::UpdateColor()
{
    const int h = pal->GetHue();
    const int s = pal->GetSat();
    const int v = int( 255 - val->GetValue() * 255 );      // HSV, max V = 255
    const int a = int( 255 - alpha->GetValue() * 255 );    // Transparency, max = 255

    QColor c;
    c.setHsv( h, s, v, a );
    color = c;
}
