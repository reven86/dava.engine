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


#include "Debug/DVAssert.h"
#include "ValidatedTextInputDialog.h"
#include <QPushButton>
#include <QDialogButtonBox>

ValidatedTextInputDialog::ValidatedTextInputDialog(QWidget* parent)
    : QInputDialog(parent)
{
    setTextEchoMode(QLineEdit::Normal);
    ExtractInternalWidgets();

    connect(this, &QInputDialog::textValueChanged, this, &ValidatedTextInputDialog::OnTextChanged);
}

void ValidatedTextInputDialog::SetValidator(std::function<bool(const QString&)> validator)
{
    validateFunction = validator;
}

void ValidatedTextInputDialog::SetWarningMessage(const QString& message)
{
    warningMessage = message;
}

void ValidatedTextInputDialog::setLabelText(const QString& arg)
{
    labelText = arg;
    QInputDialog::setLabelText(arg);
}

void ValidatedTextInputDialog::OnTextChanged(const QString& text)
{
    QPalette palette(lineEdit->palette());
    bool enabled = true;
    if (text.isEmpty() || !validateFunction(text))
    {
        QInputDialog::setLabelText(warningMessage);
        palette.setColor(QPalette::Text, Qt::red);
        enabled = false;
    }
    else
    {
        QInputDialog::setLabelText(labelText);
        palette.setColor(QPalette::Text, Qt::black);
        enabled = true;
    }
    lineEdit->setPalette(palette);
    buttonOk->setEnabled(enabled);
}

void ValidatedTextInputDialog::ExtractInternalWidgets()
{
    okButtonText(); //force ensure layout of dialog

    const QObjectList& children = this->children();
    auto iter = std::find_if(children.begin(), children.end(), [](const QObject* obj) {
        return qobject_cast<const QLineEdit*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        DVASSERT(false && "ValidatedTextInputDialog: can not find line edit");
        return;
    }
    lineEdit = qobject_cast<QLineEdit*>(*iter);
    iter = std::find_if(children.begin(), children.end(), [](const QObject* obj) {
        return qobject_cast<const QDialogButtonBox*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        DVASSERT(false && "ValidatedTextInputDialog: can not find button box");
        return;
    }
    QDialogButtonBox* buttonBox = qobject_cast<QDialogButtonBox*>(*iter);
    buttonOk = buttonBox->button(QDialogButtonBox::Ok);
}
