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


#include "dropper.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QCursor>
#include <QDateTime>
#include <QDebug>


#include "ColorPicker/ColorPicker.h"
#include "ColorPicker/EyeDropper.h"


Dropper::Dropper(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.test_2->SetColors( QColor( 0, 255, 0 ), QColor( 0, 0, 255 ) );
    ui.test_2->SetDimensions( Qt::LeftEdge | Qt::RightEdge );
    ui.test_2->SetOrientation( Qt::Vertical );
    ui.test_2->SetOffsets( 10, 10, 10, 10 );

    connect( ui.test_2, SIGNAL( started( const QPointF& ) ), SLOT( OnStarted( const QPointF& ) ) );
    connect( ui.test_2, SIGNAL( changing( const QPointF& ) ), SLOT( OnChanging( const QPointF& ) ) );
    connect( ui.test_2, SIGNAL( changed( const QPointF& ) ), SLOT( OnChanged( const QPointF& ) ) );

    connect( ui.btn, SIGNAL( clicked() ), SLOT( showCP() ) );
}

Dropper::~Dropper()
{
}

void Dropper::showCP()
{
    ColorPicker *cp = new ColorPicker( this );
    cp->setWindowFlags( Qt::Window );
    cp->setAttribute( Qt::WA_DeleteOnClose, true );

    cp->SetColor( QColor( 90, 150, 230, 120 ) );
    cp->exec();
}

void Dropper::OnStarted( const QPointF& posF )
{
    OnOn( posF );
}

void Dropper::OnChanging( const QPointF& posF )
{
    OnOn( posF );
}

void Dropper::OnChanged( const QPointF& posF )
{
    OnOn( posF );
}

void Dropper::OnOn( const QPointF& posF )
{
    const QString text = QString::number( posF.y() );
    ui.edit->setText( text );
}
