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


#include "QColorButton.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"
#include <QFile>

using namespace DAVA;

QColorButton::QColorButton(QWidget *parent) :
    QPushButton(parent)
{
    //Set default background color as white
    SetBackgroundColor(QColor(255,255,255));
}

QColor QColorButton::GetBackgroundColor() const
{ 
    return currentBackgroundColor;
}

void QColorButton::SetBackgroundColor(const QColor& color)
{
    //Create style string for background color
    QString style = QString("* { border: 1px solid grey; background-color: %1;} ").arg(color.name());
    this->currentBackgroundColor = color;
    //Reset style
    this->setStyleSheet("");
    this->setStyleSheet(style);
    //Call signal each time control background color is changed
    emit backgroundColorChanged(color);
}

void QColorButton::SetBackgroundImage(const QString &imagePath)
{
    //Check if background image file realy exist  
    if(QFile::exists(imagePath))
    {
        //Get current background color
        QString currentBgColor = this->currentBackgroundColor.name();
        //Create style string for background image. Keep current background color
        QString style = QString("* {border: 1px solid grey; background-color: %1; background-image: url(%2);} ").arg(currentBgColor,imagePath);
        //Reset style
        this->setStyleSheet("");
        this->setStyleSheet(style);
    }    
}

void QColorButton::SetDisplayMultipleColors(const bool needSetbackgroundImage)
{
    if (needSetbackgroundImage)
    {
        SetBackgroundImage(ResourcesManageHelper::GetButtonBackgroundImagePath());
    }
}