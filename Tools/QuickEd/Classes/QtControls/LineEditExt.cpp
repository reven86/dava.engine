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


#include "LineEditExt.h"
#include <QCommonStyle>
#include <QHBoxLayout>
#include <QToolButton>

LineEditExt::~LineEditExt()
{
    delete clearButton;
}

LineEditExt::LineEditExt(QWidget *parent)
    : QLineEdit(parent)
    , buttonEnabled(true)
{
    clearButton = new QToolButton(this);
    clearButton->setFixedSize(18, 18);
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setIcon(QIcon(":/Icons/editclear.png"));
    clearButton->setStyleSheet(buttonStyleSheetForCurrentState());
    
    // Some stylesheet and size corrections for the text box
    setStyleSheet(this->styleSheetForCurrentState());
    
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    QSize minSizeHint = minimumSizeHint();
    setMinimumSize(qMax(minSizeHint.width(), clearButton->sizeHint().width() + frameWidth * 2 + 2),
                   qMax(minSizeHint.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
    
    updateClearButton(text());
    
    // Update the clear button when the text changes
    QObject::connect(this, SIGNAL(textChanged(QString)), SLOT(updateClearButton(QString)));
}

bool LineEditExt::clearButtonEnabled() const
{
    return buttonEnabled;
}

void LineEditExt::setClearButtonEnabled(bool enable)
{
    if (buttonEnabled == enable)
        return;
    
    buttonEnabled = enable;
    updateClearButton(text());
}

void LineEditExt::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QSize size = clearButton->sizeHint();
    int frameWidth = this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - size.width() - 2, (rect().bottom() + 2 - size.height()) / 2);
}

void LineEditExt::updateClearButton(const QString &text)
{
    if (!text.isEmpty())
    {
        clearButton->show();
        connect(clearButton, SIGNAL(clicked()), SLOT(clear()));
        
    }
    else
    {
        disconnect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
        clearButton->hide();
        
    }
    
    setStyleSheet(styleSheetForCurrentState());
}

QString LineEditExt::styleSheetForCurrentState() const
{
    int frameWidth = this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    
    QString style;
    style += "QLineEdit {";
    if (this->text().isEmpty())
    {
        style += "font-family: 'MS Sans Serif';";
        style += "font-style: italic;";
    }
    
    style += "padding-left: 3px;";
    style += QString("padding-right: %1px;").arg(clearButton->sizeHint().width() + frameWidth + 1);
    style += "border-width: 3px;";
    style += "background-color: rgba(255, 255, 255, 204);";
    style += "}";
    style += "QLineEdit:hover, QLineEdit:focus {";
    style += "background-color: rgba(255, 255, 255, 255);";
    style += "}";
    return style;
}

QString LineEditExt::buttonStyleSheetForCurrentState() const
{
    QString style;
    
    style += "QToolButton {";
    style += "border: none; margin: 0; padding: 0;";
    style += "}";
    
    return style;
}