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


#include "editfontdialog.h"

#include "Helpers/ResourcesManageHelper.h"
#include "UI/fontmanagerdialog.h"
#include "Project/Project.h"
#include "Project/EditorFontManager.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "Model/ControlProperties/PropertiesRoot.h"
#include "Model/ControlProperties/PropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"

#include <QFileInfo>
#include <QFontComboBox>

using namespace DAVA;

EditFontDialog::EditFontDialog(QWidget *parent) 
    : QDialog(parent)
{
    setupUi(this);
    pushButton_resetLocaleFont->setIcon(QIcon(":/Icons/edit_undo.png"));
    pushButton_resetLocaleFont->setToolTip(tr("Reset Font for locale"));
    connect(comboBox_locale, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated), this, &EditFontDialog::UpdateLocaleFontWidgets);
    connect(pushButton_resetLocaleFont, &QPushButton::clicked, this, &EditFontDialog::ResetCurrentLocale);
    connect(pushButton_resetFontForAllLocales, &QPushButton::clicked, this, &EditFontDialog::ResetAllLocales);
    connect(pushButton_cancel, &QPushButton::clicked, this, &EditFontDialog::reject);
    connect(pushButton_applyToAll, &QPushButton::clicked, this, &EditFontDialog::OnApplyToAll);
    connect(pushButton_createNew, &QPushButton::clicked, this, &EditFontDialog::OnCreateNew);
}

void EditFontDialog::OnPropjectOpened()
{ 
    comboBox_locale->clear();
    for (auto &locale : LocalizationSystem::Instance()->GetAvailableLocales())
    {
        comboBox_locale->addItem(QString::fromStdString(locale));
    }
    QStringList fontsList = ResourcesManageHelper::GetFontsList();
    comboBox_defaultFont->clear();
    comboBox_defaultFont->addItems(fontsList);

    comboBox_localizedFont->clear();
    comboBox_localizedFont->addItems(fontsList);
}

void EditFontDialog::UpdateFontPreset(const QString &presetNameArg)
{
    OnPropjectOpened();
    presetName = presetNameArg;
    DVASSERT(!presetName.isEmpty());
    lineEdit_fontPresetName->setText(presetName);

    UpdateCurrentLocale();
    UpdateDefaultFontWidgets();
    UpdateLocaleFontWidgets();
}

String EditFontDialog::FindFontRecursive(const ControlNode *node)
{
    PropertiesRoot *propertiesRoot = node->GetPropertiesRoot();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        PropertiesSection *section = dynamic_cast<PropertiesSection*>(propertiesRoot->GetProperty(index));
        int sectionCount = section->GetCount();
        for (int prop = 0; prop < sectionCount; ++prop)
        {
            ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
            if (!strcmp(valueProperty->GetMember()->Name(), "font"))
            {
                return valueProperty->GetValue().AsString();
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        return FindFontRecursive(node->Get(index));
    }
    return String();
}

void EditFontDialog::SetFontPresetRecursively(ControlNode *node, const String &presetName)
{
    PropertiesRoot *propertiesRoot = node->GetPropertiesRoot();
    int propertiesCount = propertiesRoot->GetCount();
    for (int index = 0; index < propertiesCount; ++index)
    {
        PropertiesSection *section = dynamic_cast<PropertiesSection*>(propertiesRoot->GetProperty(index));
        int sectionCount = section->GetCount();
        for (int prop = 0; prop < sectionCount; ++prop)
        {
            ValueProperty *valueProperty = dynamic_cast<ValueProperty*>(section->GetProperty(prop));
            if (!strcmp(valueProperty->GetMember()->Name(), "font"))
            {
                valueProperty->SetValue(VariantType(presetName));
            }
        }
    }
    for (int index = 0; index < node->GetCount(); ++index)
    {
        SetFontPresetRecursively(node->Get(index), presetName);
    }
}

Font* EditFontDialog::GetLocaleFont(const DAVA::String &locale) const
{
    Font* tmpFont = Project::Instance()->GetEditorFontManager()->GetLocalizedFont(presetName.toStdString(), locale);
    DVASSERT(nullptr != tmpFont);
    return tmpFont->Clone();
}

void EditFontDialog::UpdateCurrentLocale()
{
    String currentLanguageID = LocalizationSystem::Instance()->GetCurrentLocale();
    comboBox_locale->setCurrentText(QString::fromStdString(currentLanguageID));
}

void EditFontDialog::UpdateDefaultFontWidgets()
{
    Font *defaultFont = GetLocaleFont("default");
    spinBox_defaultFontSize->setValue(defaultFont->GetSize());

    DVASSERT(defaultFont->GetFontType() == Font::TYPE_FT);
    FTFont *ftFont = static_cast<FTFont*>(defaultFont);
    QFileInfo fileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    comboBox_defaultFont->setCurrentText(fileInfo.fileName());
}

void EditFontDialog::UpdateLocaleFontWidgets()
{
    Font *defaultFont = GetLocaleFont(comboBox_locale->currentText().toStdString());
    spinBox_localizedFontSize->setValue(defaultFont->GetSize());

    DVASSERT(defaultFont->GetFontType() == Font::TYPE_FT);
    FTFont *ftFont = static_cast<FTFont*>(defaultFont);
    QFileInfo fileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    comboBox_localizedFont->setCurrentText(fileInfo.fileName());
}

void EditFontDialog::ResetFontForLocale(const QString &locale)
{
    QString currentDefaultFont = comboBox_defaultFont->currentText();
    QString fontPath = ResourcesManageHelper::GetFontRelativePath(currentDefaultFont);
    Font* font = FTFont::Create(fontPath.toStdString());
    font->SetSize(spinBox_defaultFontSize->value());
    Logger::FrameworkDebug("EditFontDialog::ResetFontForLocale SetLocalizedFont def");
    //Project::Instance()->GetEditorFontManager()->SetLocalizedFont(presetName.toStdString(), font, presetName.toStdString(), true, locale.toStdString());
    UpdateLocaleFontWidgets();
}

void EditFontDialog::ResetCurrentLocale()
{
    ResetFontForLocale(comboBox_locale->currentText());
}

void EditFontDialog::ResetAllLocales()
{
    for (auto i = comboBox_locale->count() - 1; i >= 0; --i)
    {
        ResetFontForLocale(comboBox_locale->itemText(i));
    }
}

void EditFontDialog::OnApplyToAll()
{
    QString currentDefaultFont = comboBox_defaultFont->currentText();
    QString fontPath = ResourcesManageHelper::GetFontRelativePath(currentDefaultFont);
    Font* font = FTFont::Create(fontPath.toStdString());
    font->SetSize(spinBox_defaultFontSize->value());
    Project::Instance()->GetEditorFontManager()->UseNewPreset(presetName.toStdString(), font->Clone(), lineEdit_fontPresetName->text().toStdString(), "default");

    for (auto &locale : LocalizationSystem::Instance()->GetAvailableLocales())
    {
        String newFontName = Project::Instance()->GetEditorFontManager()->UseNewPreset(presetName.toStdString()
            , Project::Instance()->GetEditorFontManager()->GetLocalizedFont(presetName.toStdString(), locale)->Clone()
            , lineEdit_fontPresetName->text().toStdString()
            , locale
            );
        lineEdit_fontPresetName->setText(QString::fromStdString(newFontName));
    }
    if (!Project::Instance()->GetEditorFontManager()->GetDefaultFontsPath().IsEmpty())
    {
        Project::Instance()->GetEditorFontManager()->SaveLocalizedFonts();
    }
    done(ApplyToAll);
}

void EditFontDialog::OnCreateNew()
{
    QString currentDefaultFont = comboBox_defaultFont->currentText();
    QString fontPath = ResourcesManageHelper::GetFontRelativePath(currentDefaultFont);
    Font* font = FTFont::Create(fontPath.toStdString());
    font->SetSize(spinBox_defaultFontSize->value());
    Project::Instance()->GetEditorFontManager()->CreateNewPreset(lineEdit_fontPresetName->text().toStdString(), font->Clone(), "default");
    
    for (auto &locale : LocalizationSystem::Instance()->GetAvailableLocales())
    {
        String newFontName = Project::Instance()->GetEditorFontManager()->CreateNewPreset(lineEdit_fontPresetName->text().toStdString()
            , Project::Instance()->GetEditorFontManager()->GetLocalizedFont(presetName.toStdString(), locale)->Clone()
            , locale
            );
        lineEdit_fontPresetName->setText(QString::fromStdString(newFontName));
    }
    if (!Project::Instance()->GetEditorFontManager()->GetDefaultFontsPath().IsEmpty())
    {
        Project::Instance()->GetEditorFontManager()->SaveLocalizedFonts();
    }
    done(CreateNew);
}

