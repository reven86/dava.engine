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


#include "QColorWidget.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"

#include "WidgetSignalsBlocker.h"

#include <QHBoxLayout>
#include <QFile>
#include <QSpacerItem>
#include <QColorDialog>
#include <QPainter>

using namespace DAVA;

QColorButton::QColorButton(QWidget *parent) :
    QToolButton(parent)
{
}

void QColorButton::SetColor(const QColor& color)
{
    buttonColor = color;
}

void QColorButton::paintEvent(QPaintEvent* pEvent)
{
    QToolButton::paintEvent(pEvent);

    QPainter painter(this);
    QPen pen(Qt::black, 1, Qt::SolidLine);
    painter.setPen(pen);

    QRect border = rect();
    
    static const int controlOffset = 2;
    border.adjust(controlOffset, controlOffset, -(controlOffset + 1), -(controlOffset + 1));
    painter.drawRect(border);
    
    // Draw the chequered background.
    QBrush blackBrush(Qt::black);
    QBrush whiteBrush(Qt::white);

    QPoint gridSize(border.width() / 2, border.height() / 2);
    QPoint gridCenter = border.center();
    painter.fillRect(border.left() + 1, border.top() + 1, gridSize.x(), gridSize.y(), blackBrush);
    painter.fillRect(gridCenter.x() + 1, gridCenter.y() + 1, gridSize.x(), gridSize.y(), blackBrush);
    painter.fillRect(border.left() + 1, gridCenter.y() + 1, gridSize.x(), gridSize.y(), whiteBrush);
    painter.fillRect(gridCenter.x() + 1, border.top() + 1, gridSize.x(), gridSize.y(), whiteBrush);

    QBrush colorBrush(buttonColor);
    border.adjust(1, 1, 0, 0);
    painter.fillRect(border, buttonColor);
}

QColorWidget::QColorWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout* layoyt = new QHBoxLayout(this);
    layoyt->setContentsMargins(0, 0, 0, 0);
    layoyt->setSpacing(0);
    setLayout(layoyt);
    
    selectColorButton = new QColorButton(this);
    layoyt->addWidget(selectColorButton);

    QSpacerItem* horzSpacer = new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    layoyt->addItem(horzSpacer);

    colorLineEdit = new QtColorLineEdit(this);
    layoyt->addWidget(colorLineEdit);

    connect(selectColorButton, SIGNAL(clicked()), this, SLOT(onChangeColorButtonClicked()));
    connect(colorLineEdit, SIGNAL(colorEditFinished()), this, SLOT(onColorEditFinished()));

    //Set default background color as white
    SetBackgroundColor(QColor(255,255,255));
}

QColor QColorWidget::GetBackgroundColor() const
{ 
    return selectedColor;
}

void QColorWidget::SetBackgroundColor(const QColor& color)
{
    selectedColor = color;
    WidgetSignalsBlocker blocker(colorLineEdit);
    colorLineEdit->SetColor(selectedColor);

    SetButtonColor(color);
    emit colorChanged(selectedColor);
}

void QColorWidget::SetDisplayMultipleColors(const bool /*needSetbackgroundImage */)
{
    // TODO! implement "multiple colors" mode.
}

void QColorWidget::onChangeColorButtonClicked()
{
    QColor color = QColorDialog::getColor(selectedColor, NULL, "Select color", QColorDialog::ShowAlphaChannel);
	if (color.isValid() == false)
    {
        return;
    }

    WidgetSignalsBlocker blocker(colorLineEdit);
    colorLineEdit->SetColor(selectedColor);

    selectedColor = color;
    SetButtonColor(color);
    emit colorChanged(selectedColor);
}

void QColorWidget::onColorEditFinished()
{
    QColor color = colorLineEdit->GetColor();
    if (color.isValid() == false)
    {
        return;
    }

    selectedColor = color;
    SetButtonColor(color);
    emit colorChanged(selectedColor);
}

void QColorWidget::SetButtonColor(const QColor& color)
{
    selectColorButton->SetColor(color);
    selectColorButton->repaint();
}
