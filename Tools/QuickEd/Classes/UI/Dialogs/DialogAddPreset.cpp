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


#include "DialogAddPreset.h"
#include "EditorCore.h"

#include <QCompleter>

using namespace DAVA;

DialogAddPreset::DialogAddPreset(const QString& originalPresetNameArg, QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    comboBox_baseFontPresetName->addItem("");
    comboBox_baseFontPresetName->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
    comboBox_baseFontPresetName->setCurrentText(originalPresetNameArg);
    lineEdit_newFontPresetName->setText(originalPresetNameArg);

    lineEdit_newFontPresetName->setCompleter(new QCompleter(GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit_newFontPresetName));

    connect(lineEdit_newFontPresetName, &QLineEdit::textChanged, this, &DialogAddPreset::OnNewPresetNameChanged);
    connect(comboBox_baseFontPresetName, &QComboBox::currentTextChanged, this, &DialogAddPreset::OnNewPresetNameChanged);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DialogAddPreset::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DialogAddPreset::OnAccept);

    OnNewPresetNameChanged();
}

void DialogAddPreset::OnNewPresetNameChanged()
{
    QString baseName = comboBox_baseFontPresetName->currentText();
    QString newName = lineEdit_newFontPresetName->text();
    QString resultText;
    bool enabled = false;
    if (newName.isEmpty())
    {
        enabled = false;
        resultText = tr("enter new preset name");
    }
    else if (baseName == newName)
    {
        enabled = false;
        resultText = tr("names match!");
    }
    else if (GetEditorFontSystem()->GetDefaultPresetNames().contains(newName))
    {
        enabled = false;
        resultText = tr("This preset name already exists in the system");
    }
    else
    {
        enabled = true;
        resultText = tr("New font preset will be created");
    }
    label_info->setText(resultText);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void DialogAddPreset::OnAccept()
{
    auto editorFontSystem = GetEditorFontSystem();
    if (!editorFontSystem->GetDefaultPresetNames().contains(lineEdit_newFontPresetName->text()))
    {
        editorFontSystem->CreateNewPreset(comboBox_baseFontPresetName->currentText().toStdString(), lineEdit_newFontPresetName->text().toStdString());
        editorFontSystem->SaveLocalizedFonts();
    }
    accept();
}