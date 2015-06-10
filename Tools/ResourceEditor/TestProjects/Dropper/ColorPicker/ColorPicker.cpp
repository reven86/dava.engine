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


#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"
#include "ColorPickerRGBAM.h"
#include "ColorPreview.h"
#include "EyeDropper.h"


ColorPicker::ColorPicker(QWidget *parent)
    : AbstractColorPicker( parent )
    , ui( new Ui::ColorPicker() )
{
    ui->setupUi( this );

    setFocusPolicy( Qt::ClickFocus );

    // Pickers
    RegisterPicker( "HSV rectangle", new ColorPickerHSV() );

    // Editors
    RegisterColorSpace( "RGBA M", new ColorPickerRGBAM() );

    // Preview
    connect( this, SIGNAL( changing( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ) );
    connect( this, SIGNAL( changed( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ) );

    // Dropper
    connect( ui->dropper, SIGNAL( clicked() ), SLOT( OnDropper() ) );
}

ColorPicker::~ColorPicker()
{
}

void ColorPicker::exec()
{
    const Qt::WindowFlags f = windowFlags();
    const Qt::WindowModality m = windowModality();
    setWindowFlags( f | Qt::Dialog );
    setWindowModality( Qt::WindowModal );

    show();
    modalLoop.exec();

    setWindowFlags( f );
    setWindowModality(m);
}

void ColorPicker::RegisterPicker( QString const& key, AbstractColorPicker* picker )
{
    delete pickers[key];
    pickers[key] = picker;

    ui->pickerCombo->addItem( key, key );
    ui->pickerStack->addWidget( picker );
    ConnectPicker( picker );
}

void ColorPicker::RegisterColorSpace( QString const& key, AbstractColorPicker* picker )
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem( key, key );
    ui->colorSpaceStack->addWidget( picker );
    ConnectPicker( picker );
}

void ColorPicker::SetColorInternal( const QColor& c )
{
    UpdateControls( c );
    ui->preview->SetColorOld( c );
    ui->preview->SetColorNew( c );
}

void ColorPicker::OnChanging( const QColor& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changing( c );
}

void ColorPicker::OnChanged( const QColor& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changed( c );
}

void ColorPicker::OnDropperChanged( const QColor& c )
{
    QColor normalized( c );
    normalized.setAlphaF( GetColor().alphaF() );
    UpdateControls(normalized);
    ui->preview->SetColorNew(normalized);
}

void ColorPicker::OnDropper()
{
    dropper = new EyeDropper( this );
    connect( dropper, SIGNAL( picked( const QColor& ) ), SLOT( OnDropperChanged( const QColor& ) ) );
    dropper->Exec();
}

void ColorPicker::UpdateControls( const QColor& c, AbstractColorPicker* source )
{
    for ( auto it = pickers.begin(); it != pickers.end(); ++it )
    {
        AbstractColorPicker *recv = it.value();
        if ( recv && recv != source )
        {
            recv->SetColor( c );
        }
    }
    for ( auto it = colorSpaces.begin(); it != colorSpaces.end(); ++it )
    {
        AbstractColorPicker *recv = it.value();
        if ( recv && recv != source )
        {
            recv->SetColor( c );
        }
    }
}

void ColorPicker::ConnectPicker( AbstractColorPicker* picker )
{
    connect( picker, SIGNAL( begin() ), SIGNAL( begin() ) );
    connect( picker, SIGNAL( changing( const QColor& ) ), SLOT( OnChanging( const QColor& ) ) );
    connect( picker, SIGNAL( changed( const QColor& ) ), SLOT( OnChanged( const QColor& ) ) );
    connect( picker, SIGNAL( canceled() ), SIGNAL( canceled() ) );
}

void ColorPicker::closeEvent( QCloseEvent* e )
{
    if (modalLoop.isRunning())
    {
        modalLoop.quit();
    }

    QWidget::closeEvent(e);
}
