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


#include "UI/Dialogs/DialogConfigurePreset.h"
#include "Helpers/ResourcesManageHelper.h"
#include "FileSystem/LocalizationSystem.h"
#include "EditorCore.h"

using namespace DAVA;

DialogConfigurePreset::DialogConfigurePreset(const QString& originalPresetNameArg, QWidget* parent)
    : QDialog(parent)
    , originalPresetName(originalPresetNameArg)
{
    setupUi(this);
    pushButton_resetLocale->setIcon(QIcon(":/Icons/edit_undo.png"));
    pushButton_resetLocale->setToolTip(tr("Reset font for locale"));

    lineEdit_currentFontPresetName->setText(originalPresetName);
    QStringList fontsList = DAVA::ResourcesManageHelper::GetFontsList();
    comboBox_defaultFont->addItems(fontsList);
    comboBox_localizedFont->addItems(fontsList);

    comboBox_locale->addItems(GetEditorFontSystem()->GetAvailableFontLocales());

    comboBox_locale->setCurrentText(QString::fromStdString(LocalizationSystem::Instance()->GetCurrentLocale()));

    initPreset();

    connect(comboBox_defaultFont, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnDefaultFontChanged);
    connect(spinBox_defaultFontSize, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DialogConfigurePreset::OnDefaultFontSizeChanged);
    connect(comboBox_localizedFont, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnLocalizedFontChanged);
    connect(spinBox_localizedFontSize, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DialogConfigurePreset::OnLocalizedFontSizeChanged);
    connect(comboBox_locale, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnCurrentLocaleChanged);
    connect(pushButton_resetLocale, &QPushButton::clicked, this, &DialogConfigurePreset::OnResetLocale);
    connect(pushButton_applyDefaultToAllLocales, &QPushButton::clicked, this, &DialogConfigurePreset::OnApplyToAllLocales);
    connect(pushButton_ok, &QPushButton::clicked, this, &DialogConfigurePreset::OnOk);
    connect(pushButton_cancel, &QPushButton::clicked, this, &DialogConfigurePreset::OnCancel);
}

void DialogConfigurePreset::initPreset()
{
    UpdateDefaultFontWidgets();
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnDefaultFontChanged(const QString& arg)
{
    SetFont(arg, spinBox_defaultFontSize->value(), GetEditorFontSystem()->GetDefaultFontLocale());
}

void DialogConfigurePreset::OnDefaultFontSizeChanged(int size)
{
    SetFont(comboBox_defaultFont->currentText(), size, GetEditorFontSystem()->GetDefaultFontLocale());
}

void DialogConfigurePreset::OnLocalizedFontChanged(const QString& arg)
{
    SetFont(arg, spinBox_localizedFontSize->value(), comboBox_locale->currentText());
}

void DialogConfigurePreset::OnLocalizedFontSizeChanged(int size)
{
    SetFont(comboBox_localizedFont->currentText(), size, comboBox_locale->currentText());
}

void DialogConfigurePreset::OnCurrentLocaleChanged(const QString& arg)
{
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnResetLocale()
{
    SetFont(comboBox_defaultFont->currentText(), spinBox_defaultFontSize->value(), comboBox_locale->currentText());
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnApplyToAllLocales()
{
    for (const auto &locale : GetEditorFontSystem()->GetAvailableFontLocales())
    {
        SetFont(comboBox_defaultFont->currentText(), spinBox_defaultFontSize->value(), locale);
        UpdateLocalizedFontWidgets();
    }
}

void DialogConfigurePreset::OnOk()
{
    GetEditorFontSystem()->SaveLocalizedFonts();
    accept();
}

void DialogConfigurePreset::OnCancel()
{
    GetEditorFontSystem()->LoadLocalizedFonts();
    reject();
}

void DialogConfigurePreset::UpdateDefaultFontWidgets()
{
    spinBox_defaultFontSize->blockSignals(true);
    comboBox_defaultFont->blockSignals(true);
    Font *font = GetEditorFontSystem()->GetFont(lineEdit_currentFontPresetName->text().toStdString(), GetEditorFontSystem()->GetDefaultFontLocale());
    spinBox_defaultFontSize->setValue(font->GetSize());

    DVASSERT(font->GetFontType() == Font::TYPE_FT);
    FTFont *ftFont = static_cast<FTFont*>(font);
    QFileInfo fileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    comboBox_defaultFont->setCurrentText(fileInfo.fileName());
    spinBox_defaultFontSize->blockSignals(false);
    comboBox_defaultFont->blockSignals(false);
}

void DialogConfigurePreset::UpdateLocalizedFontWidgets()
{
    spinBox_localizedFontSize->blockSignals(true);
    comboBox_localizedFont->blockSignals(true);
    Font *font = GetEditorFontSystem()->GetFont(lineEdit_currentFontPresetName->text().toStdString(), comboBox_locale->currentText().toStdString());
    spinBox_localizedFontSize->setValue(font->GetSize());

    DVASSERT(font->GetFontType() == Font::TYPE_FT);
    FTFont *ftFont = static_cast<FTFont*>(font);
    QFileInfo fileInfo = QFileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    comboBox_localizedFont->setCurrentText(fileInfo.fileName());
    spinBox_localizedFontSize->blockSignals(false);
    comboBox_localizedFont->blockSignals(false);
}

void DialogConfigurePreset::SetFont(const QString& fontType, const int fontSize, const QString& locale)
{
    QString fontPath = ResourcesManageHelper::GetFontRelativePath(fontType);
    Font* font = FTFont::Create(fontPath.toStdString());
	if (nullptr == font)
	{
		QMessageBox::warning(this, tr("Font creation error"), tr("Can not create font from %1").arg(fontPath));
		return;
	}
    font->SetSize(fontSize);
    GetEditorFontSystem()->SetFont(lineEdit_currentFontPresetName->text().toStdString(), locale.toStdString(), font);
}
