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


#include "QtColorLineEdit.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QStyle>
#include <QColorDialog>
#include <QRegExpValidator>
#include <QFocusEvent>
#include <QApplication>

QtColorLineEdit::QtColorLineEdit(QWidget * parent)
	: QLineEdit(parent),
    needEmitEditingFinished(true)
{
	QRegExpValidator *validator = new QRegExpValidator();
	validator->setRegExp(QRegExp("#{0,1}[A-F0-9]{8}", Qt::CaseInsensitive));

	setValidator(validator);

	QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(EditFinished()));
    installEventFilter(this);
}

void QtColorLineEdit::SetColor(const QColor &color)
{
	QString s;
	curColor = color;
	setText(s.sprintf("#%02x%02x%02x%02x", curColor.red(), curColor.green(), curColor.blue(), curColor.alpha()));
}

QColor QtColorLineEdit::GetColor() const
{
	return curColor;
}

void QtColorLineEdit::EditFinished()
{
	int r = 0, g = 0, b = 0, a = 255;
	QString colorString = text().startsWith("#") ? text() : "#" + text();
	if(4 == sscanf(colorString.toStdString().c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a))
	{
		curColor = QColor(r, g, b, a);
	}

    if (needEmitEditingFinished)
    {
        emit colorEditFinished();
    }
    else
    {
        // The signal was blocked because of validator, but have to be emitted next time.
        needEmitEditingFinished = true;
    }
}

bool QtColorLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj && event->type() == QEvent::FocusOut)
    {
        // Verify whether color is correct, if not - reset the value to existing.
        int pos = 0;
        QString curText = text();
        if (validator() && validator()->validate(curText, pos) != QValidator::Acceptable)
        {
            SetColor(curColor);
            needEmitEditingFinished = false;
        }
    }

    return QLineEdit::eventFilter( obj, event );
}
