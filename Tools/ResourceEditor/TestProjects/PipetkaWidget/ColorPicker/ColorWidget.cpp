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


#include "ColorWidget.h"
#include "ui_ColorWidget.h"

#include "IColorEditor.h"
#include "HSVPaletteWidget.h"


ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorWidget())
{
    ui->setupUi(this);
    
    // TEST
    {
        ui->test_01->SetColorRange( QColor( 255, 0, 0 ), QColor( 0, 0, 0, 0 ) );
        ui->test_01->SetRenderDimensions( true, false );
    }
    {
        const QColor c1 = QColor( 0, 255, 0 );
        const QColor c2 = QColor( 0, 0, 255 );
        ui->test_02->SetColorRange( c1, c2 );
        ui->test_02->SetRenderDimensions( false, true );
        ui->test_02->SetBgPadding( 5, 0, 5, 0 );
        ui->test_02->setEditorDimensions( Qt::LeftEdge | Qt::RightEdge/* | Qt::TopEdge | Qt::BottomEdge*/ );
        ui->test_02->setPrefferableArrows();

        connect( ui->test_02, SIGNAL( changing( const QColor& ) ), SLOT( onSliderColor( const QColor& ) ) );
    }

    AddPalette( "HSV", new HSVPaletteWidget() );

    connect( ui->paletteCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( onPaletteType() ) );
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::AddPalette( QString const& name, IColorEditor* _pal )
{
    QWidget *pal = dynamic_cast<QWidget *>( _pal );
    Q_ASSERT( pal );

    ui->paletteCombo->addItem( name, name );
    ui->paletteStack->addWidget( pal );
    paletteMap[name] = pal;
}

void ColorWidget::onPaletteType()
{
    const int idx = ui->paletteCombo->currentIndex();
    //if ( idx < 0 )
    //    return;

    const QString& key = ui->paletteCombo->itemData( idx ).toString();
    ui->paletteStack->setCurrentWidget( paletteMap[key] );
}

void ColorWidget::onSliderColor( QColor const& c )
{
    const QSize picSize( ui->cmid->size() );
    QPixmap pix( picSize );
    pix.fill( c );
    ui->cmid->setPixmap( pix );
}
