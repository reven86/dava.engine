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


#include "ColorComponentSlider.h"

#include <QVBoxLayout>
#include <QPainter>

#include "../Widgets/ValueSlider.h"
#include "GradientSlider.h"


ColorComponentSlider::ColorComponentSlider(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin( 0 );
    l->setContentsMargins( 0, 0, 0, 0 );
    l->setSpacing( 0 );
    
    value = new ValueSlider();
    value->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    value->setMinimumSize( 0, 20 );
    l->addWidget( value );

    gradient = new GradientSlider();
    gradient->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );
    gradient->setMinimumSize( 0, 5 );
    l->addWidget( gradient );

    setLayout( l );
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum );
    updateGeometry();

    connect( value, SIGNAL( started( double ) ), SIGNAL( started( double ) ) );
    connect( value, SIGNAL( changing( double ) ), SIGNAL( changing( double ) ) );
    connect( value, SIGNAL( changed( double ) ), SIGNAL( changed( double ) ) );
    connect( value, SIGNAL( canceled() ), SIGNAL( canceled() ) );
}

ColorComponentSlider::~ColorComponentSlider()
{
}

void ColorComponentSlider::SetColorRange( QColor const& c1, QColor const& c2 )
{
    gradient->SetColors( c1, c2 );
}

void ColorComponentSlider::SetValue( double val )
{
    value->SetValue( val );
}

double ColorComponentSlider::GetValue() const
{
    return value->GetValue();
}