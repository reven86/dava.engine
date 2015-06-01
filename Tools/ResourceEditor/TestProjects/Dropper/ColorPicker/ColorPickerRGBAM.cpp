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


#include "ColorPickerRGBAM.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "ColorComponentSlider.h"
#include "../Helpers/PaintingHelper.h"


ColorPickerRGBAM::ColorPickerRGBAM(QWidget *parent)
    : AbstractColorPicker(parent)
{
    r = new ColorComponentSlider();
    g = new ColorComponentSlider();
    b = new ColorComponentSlider();
    a = new ColorComponentSlider();
    m = new ColorComponentSlider();

    ColorComponentSlider *sliders[] = { r, g, b, a, m, };
    const QString labels[] = { "Red", "Green", "Blue", "Alpha", "Multiply" };
    
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin( 0 );
    l->setContentsMargins( 0, 0, 0, 0 );

    for ( int i = 0; i < sizeof( sliders ) / sizeof( *sliders ); i++ )
    {
        l->addLayout( CreateSlider( labels[i], sliders[i] ) );

        connect( sliders[i], SIGNAL( changing( double ) ), SLOT( OnChanging( double ) ) );
    }
    l->addStretch( 1 );

    setLayout( l );
}

ColorPickerRGBAM::~ColorPickerRGBAM()
{
}

void ColorPickerRGBAM::OnChanging( double val )
{
    Q_UNUSED( val );
    ColorComponentSlider *source = qobject_cast<ColorComponentSlider *>( sender() );
    UpdateColorInternal( source );
    emit changing( GetColor() );
}

void ColorPickerRGBAM::SetColorInternal( const QColor& c )
{
    color = c;
    r->SetValue( color.redF() );
    g->SetValue( color.greenF() );
    b->SetValue( color.blueF() );
    a->SetValue( color.alphaF() );
    m->SetColorRange( color, color );

    UpdateColorInternal();
}

void ColorPickerRGBAM::UpdateColorInternal( ColorComponentSlider* source )
{
    color.setRgbF( r->GetValue(), g->GetValue(), b->GetValue(), a->GetValue() );

    if ( source != r )
    {
        r->SetColorRange( PaintingHelper::MinColorComponent( color, 'r' ), PaintingHelper::MaxColorComponent( color, 'r' ) );
        r->SetValue( color.redF() );
    }
    if ( source != g )
    {
        g->SetColorRange( PaintingHelper::MinColorComponent( color, 'g' ), PaintingHelper::MaxColorComponent( color, 'g' ) );
        g->SetValue( color.greenF() );
    }
    if ( source != b )
    {
        b->SetColorRange( PaintingHelper::MinColorComponent( color, 'b' ), PaintingHelper::MaxColorComponent( color, 'b' ) );
        b->SetValue( color.blueF() );
    }
    if ( source != a )
    {
        a->SetColorRange( PaintingHelper::MinColorComponent( color, 'a' ), PaintingHelper::MaxColorComponent( color, 'a' ) );
        a->SetValue( color.alphaF() );
    }
    if ( source != m )
    {
        m->SetColorRange( color, color );
    }
}

QLayout* ColorPickerRGBAM::CreateSlider( const QString& text, ColorComponentSlider *w ) const
{
    QHBoxLayout *l = new QHBoxLayout();
    QLabel *txt = new QLabel( text );
    txt->setMinimumSize( 40, 0 );
    txt->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
    l->addWidget( txt );
    l->addWidget( w );

    return l;
}