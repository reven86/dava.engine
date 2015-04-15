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
#include "EditorFontManager.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "Model/ControlProperties/PropertiesRoot.h"
#include "Model/ControlProperties/PropertiesSection.h"
#include "Model/ControlProperties/ValueProperty.h"

#include <QtGui>
#include <QtWidgets>

using namespace DAVA;

EditFontDialog::EditFontDialog(QWidget *parent) 
	: QDialog(parent)
{
    setupUi(this);
    pushButton_resetLocaleFont->setIcon(QIcon(":/Icons/edit_undo.png"));
    pushButton_resetLocaleFont->setToolTip(tr("Reset Font for locale"));
    connect(comboBox_locale, &QComboBox::currentTextChanged, this, &EditFontDialog::UpdateLocaleFontWidgets);
    connect(pushButton_resetLocaleFont, &QPushButton::clicked, this, &EditFontDialog::ResetCurrentLocale);
    connect(pushButton_resetFontForAllLocales, &QPushButton::clicked, this, &EditFontDialog::ResetAllLocales);
    connect(pushButton_cancel, &QPushButton::clicked, this, &EditFontDialog::reject);
    connect(pushButton_applyToAll, &QPushButton::clicked, this, &EditFontDialog::OnApplyToAll);
    connect(pushButton_createNew, &QPushButton::clicked, this, &EditFontDialog::OnCreateNew);
}

void EditFontDialog::OnPropjectOpened()
{
    comboBox_locale->clear();
    const Vector<String> &locales = EditorFontManager::Instance()->GetLocales();
    for (auto &locale : EditorFontManager::Instance()->GetLocales())
    {
        comboBox_locale->addItem(QString::fromStdString(locale));
    }
    QStringList fontsList = ResourcesManageHelper::GetFontsList();
    comboBox_defaultFont->clear();
    comboBox_defaultFont->addItems(fontsList);

    comboBox_localizedFont->clear();
    comboBox_localizedFont->addItems(fontsList);
}

void EditFontDialog::UpdateFontPreset(ControlNode *selectedControlNode)
{
    presetName = findFont(selectedControlNode);
    DVASSERT(!presetName.empty());
    lineEdit_fontPresetName->setText(QString::fromStdString(presetName));   

    UpdateCurrentLocale();
    UpdateDefaultFontWidgets();
    UpdateLocaleFontWidgets();
}

String EditFontDialog::findFont(ControlNode *node)
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
        return findFont(node->Get(index));
    }
    return String();
}

Font* EditFontDialog::GetLocaleFont(const DAVA::String &locale) const
{
    Font* tmpFont = EditorFontManager::Instance()->GetLocalizedFont(presetName, locale);
    Font *defaultFont = nullptr != tmpFont ? tmpFont->Clone() : EditorFontManager::Instance()->GetDefaultFont()->Clone();
    DVASSERT(nullptr != defaultFont);
    return defaultFont;
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
    EditorFontManager::Instance()->SetLocalizedFont(presetName, font, presetName, true, locale.toStdString());
    UpdateLocaleFontWidgets();
}

void EditFontDialog::ResetCurrentLocale()
{
    ResetFontForLocale(comboBox_locale->currentText());
}

void EditFontDialog::ResetAllLocales()
{
    for (auto i = comboBox_locale->count() - 1; i >= 0; ++i)
    {
        ResetFontForLocale(comboBox_locale->itemText(i));
    }
}

void EditFontDialog::OnApplyToAll()
{

}

void EditFontDialog::OnCreateNew()
{

}

/*
void EditFontDialog::ProcessPushButtonClicked(QPushButton *senderWidget)
{
    if(senderWidget == ui->fontSelectButton || senderWidget == ui->localizedFontSelectButton)
    {
        
        // Get current value of Font property
        Font *fontPropertyValue = NULL;//(senderWidget == ui->fontSelectButton ? dialogResult.font : dialogResult.GetLocalizedFont(currentLocale));

        // Get sprite path from graphics font
        QString currentGFontPath = ResourcesManageHelper::GetGraphicsFontPath(fontPropertyValue);
        
        //Call font selection dialog - with ok button and preset of graphics font path
        FontManagerDialog *fontDialog = new FontManagerDialog(true, currentGFontPath);
        Font *resultFont = NULL;
        
        if ( fontDialog->exec() == QDialog::Accepted )
        {
            resultFont = fontDialog->ResultFont();
        }
        
        //Delete font select dialog reference
        SafeDelete(fontDialog);
        
        if (!resultFont)
        {
            return;
        }
        
        //        PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
        //        if (iter == propertyGridWidgetsMap.end())
        //        {
        //            Logger::Error("OnPushButtonClicked - unable to find attached property in the propertyGridWidgetsMap!");
        //            return;
        //        }
        
        // Don't update the property if the text wasn't actually changed.
        //Font* curValue = PropertiesHelper::GetAllPropertyValues<Font*>(this->activeMetadata, iter->second.getProperty().name());
        Font* curValue = fontPropertyValue;
        if (curValue && curValue->IsEqual(resultFont))
        {
            SafeRelease(resultFont);
            return;
        }
        
        if(senderWidget == ui->fontSelectButton)
        {
//             SafeRelease(dialogResult.font);
//             dialogResult.font = SafeRetain(resultFont);
            
            UpdateDefaultFontParams();
        }
        else
        {
            dialogResult.SetLocalizedFont(resultFont, currentLocale);
            
            UpdateLocalizedFontParams();
        }
        SafeRelease(resultFont);
    }
    else if(senderWidget == ui->resetFontForLocalePushButton)
    {
//         Font *defaultFontClone = dialogResult.font->Clone();
//         dialogResult.SetLocalizedFont(defaultFontClone, currentLocale);
//         SafeRelease(defaultFontClone);
        
        UpdateLocalizedFontParams();
    }
}

void EditFontDialog::UpdateDefaultFontParams()
{
    UpdateSpinBoxWidgetWithPropertyValue(ui->fontSizeSpinBox);
    UpdatePushButtonWidgetWithPropertyValue(ui->fontSelectButton);
}

void EditFontDialog::UpdateLocalizedFontParams()
{
    UpdateSpinBoxWidgetWithPropertyValue(ui->localizedFontSizeSpinBox);
    UpdatePushButtonWidgetWithPropertyValue(ui->localizedFontSelectButton);
}

void EditFontDialog::UpdateLineEditWidgetWithPropertyValue(QLineEdit *lineEditWidget)
{
    if(lineEditWidget != ui->fontPresetNameLlineEdit)
    {
        return; //Not font preset line edit
    }
    
    Font *fontPropertyValue = NULL;//dialogResult.font;
    
    if(fontPropertyValue)
    {
        //lineEditWidget->setText(QString::fromStdString(dialogResult.fontPresetName));
    }
}

void EditFontDialog::UpdateSpinBoxWidgetWithPropertyValue(QSpinBox *spinBoxWidget)
{
    Font *fontPropertyValue = NULL;
    if(spinBoxWidget == ui->fontSizeSpinBox)
    {
        //fontPropertyValue = dialogResult.font;
    }
    else if(spinBoxWidget == ui->localizedFontSizeSpinBox)
    {
        //fontPropertyValue = dialogResult.GetLocalizedFont(currentLocale);
    }
    
    if(fontPropertyValue)
    {
        spinBoxWidget->setValue(fontPropertyValue->GetSize());
    }
}

void EditFontDialog::UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget)//, const QMetaProperty &curProperty)
{
    Font *fontPropertyValue = NULL;
    if(pushButtonWidget == ui->fontSelectButton)
    {
        //fontPropertyValue = dialogResult.font;
    }
    else if(pushButtonWidget == ui->localizedFontSelectButton)
    {
        //fontPropertyValue = dialogResult.GetLocalizedFont(currentLocale);
    }
    
    //Logger::FrameworkDebug("EditFontDialog::UpdatePushButtonWidgetWithPropertyValue fontPropertyValue=%p fontPresetName=%s", fontPropertyValue, dialogResult.fontPresetName.c_str());
    
    if(fontPropertyValue)
    {
        //Set button text
        Font::eFontType fontType = fontPropertyValue->GetFontType();
        QString buttonText;
        
        switch(fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = static_cast<FTFont*>(fontPropertyValue);
                //Set pushbutton widget text
				buttonText = QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath());
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = static_cast<GraphicsFont*>(fontPropertyValue);
                //Put into result string font definition and font sprite path
                Sprite *fontSprite = gFont->GetFontSprite();
                if (!fontSprite) //If no sprite available - quit
                {
                    pushButtonWidget->setText("Graphical font is not available");
                    return;
                }
                //Get font definition and sprite relative path
                QString fontDefinitionName = QString::fromStdString(gFont->GetFontDefinitionName().GetFrameworkPath());
                QString fontSpriteName =QString::fromStdString(fontSprite->GetRelativePathname().GetFrameworkPath());
                //Set push button widget text - for grapics font it contains font definition and sprite names
                buttonText = QString("%1\n%2").arg(fontDefinitionName, fontSpriteName);
                break;
            }
            case Font::TYPE_DISTANCE:
            {
                DFFont *dfFont = static_cast<DFFont*>(fontPropertyValue);
                //Set pushbutton widget text
				buttonText = QString::fromStdString(dfFont->GetFontPath().GetFrameworkPath());
                break;
            }
            default:
            {
                //Do nothing if we can't determine font type
                return;
            }
        }
        
        Logger::FrameworkDebug("EditFontDialog::UpdatePushButtonWidgetWithPropertyValue %s", buttonText.toStdString().c_str());
        
        pushButtonWidget->setText(buttonText);
    }
}

void EditFontDialog::UpdateComboBoxWidgetWithPropertyValue(QComboBox *comboBoxWidget)
{
    if(comboBoxWidget != ui->selectLocaleComboBox)
    {
        return; //Not select locale combobox
    }
    
    QString locale = QString::fromStdString(currentLocale);
    int32 index = comboBoxWidget->findText(locale);
    if(index > -1)
    {
        comboBoxWidget->setCurrentIndex(index);
    }
    //TODO: log error if locale is not found?
}

void EditFontDialog::OnOkButtonClicked()
{
    //dialogResult.fontPresetName = ui->fontPresetNameLlineEdit->text().toStdString();
    
    Logger::FrameworkDebug("EditFontDialog::OnOkButtonClicked");
}
*/