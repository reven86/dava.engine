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

#include "DialogEditPresetName.h"
#include "EditorCore.h"

#include <QCompleter>

using namespace DAVA;

DialogEditPresetName::DialogEditPresetName(const QString& originalPresetNameArg, QWidget* parent)
    : QDialog(parent)
    , originalPresetName(originalPresetNameArg)
{
    setupUi(this);
    lineEdit_currentFontPresetName->setText(originalPresetName);
    lineEdit_newFontPresetName->setText(originalPresetNameArg);

    lineEdit_newFontPresetName->setCompleter(new QCompleter(GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit_newFontPresetName));

    connect(lineEdit_newFontPresetName, &QLineEdit::textChanged, this, &DialogEditPresetName::OnNewPresetNameChanged);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DialogEditPresetName::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DialogEditPresetName::OnAccept);

    OnNewPresetNameChanged(lineEdit_newFontPresetName->text());
}

void DialogEditPresetName::OnNewPresetNameChanged(const QString& arg)
{
    QString text;
    bool enabled = false;
    if (arg.isEmpty())
    {
        enabled = false;
        text = tr("enter new preset name");
    }
    else if (arg == originalPresetName)
    {
        enabled = false;
        text = tr("names match!");
    }
    else if (GetEditorFontSystem()->GetDefaultPresetNames().contains(arg))
    {
        enabled = true;
        text = tr("This preset name already exists in the system");
    }
    else
    {
        enabled = true;
        text = tr("New font preset will be created");
    }
    label_info->setText(text);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void DialogEditPresetName::OnAccept()
{
    auto editorFontSystem = GetEditorFontSystem();
    if (!editorFontSystem->GetDefaultPresetNames().contains(lineEdit_newFontPresetName->text()))
    {
        editorFontSystem->CreateNewPreset(originalPresetName.toStdString(), lineEdit_newFontPresetName->text().toStdString());
        editorFontSystem->SaveLocalizedFonts();
    }
    accept();
}