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


#include "Vector2DEdit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>

Vector2DEdit::Vector2DEdit(QWidget *parent)
    : QWidget(parent)
    , editX(NULL)
    , editY(NULL)
{
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    QLabel * label1 = new QLabel(this);
    label1->setText("[");

    horizontalLayout->addWidget(label1);

    editX = new QLineEdit(this);
    editX->setObjectName(QString::fromUtf8("lineEditX"));
    editX->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));
    horizontalLayout->addWidget(editX);

    QLabel * label2 = new QLabel(this);
    label2->setText(",");

    horizontalLayout->addWidget(label2);

    editY = new QLineEdit(this);
    editY->setObjectName(QString::fromUtf8("lineEditY"));
    editY->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));
    horizontalLayout->addWidget(editY);

    QLabel * label3 = new QLabel(this);
    label3->setText("]");

    horizontalLayout->addWidget(label3);

    horizontalLayout->setStretch(1, 1);
    horizontalLayout->setStretch(3, 1);

    setAutoFillBackground(true);
}

Vector2DEdit::~Vector2DEdit()
{
}

QVector2D Vector2DEdit::vector2D() const
{
    return QVector2D(editX->text().toDouble(), editY->text().toDouble() );
}

void Vector2DEdit::setVector2D(const QVector2D &newValue)
{
    if (signalsBlocked())
    {
        editX->blockSignals(true);
        editY->blockSignals(true);
    }

    editX->setText(QString("%1").arg(newValue.x()));
    editY->setText(QString("%1").arg(newValue.y()));

    if (signalsBlocked())
    {
        editX->blockSignals(false);
        editY->blockSignals(false);
    }
}
