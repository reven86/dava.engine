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
#include "ui_editfontdialog.h"

//#include "ResourcesManageHelper.h"
//#include "EditorFontManager.h"
//#include "EditorSettings.h"
//#include "TexturePacker/ResourcePacker2D.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndexList>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableWidgetItem>

#include "Helpers/ResourcesManageHelper.h"
#include "UI/fontmanagerdialog.h"
#include "Helpers/WidgetSignalsBlocker.h"
#include "EditorFontManager.h"

using namespace DAVA;

//static const QString FONT_TABLE_NAME_COLUMN = "Font Name";
//static const QString FONT_TABLE_TYPE_COLUMN = "Font Type";
//static const QString FONT_TYPE_BASIC = "Basic";
//static const QString FONT_TYPE_GRAPHIC = "Graphics";
//static const QString LOAD_FONT_ERROR_MESSAGE = "Can't load font %1! Try again or select another one.";
//static const QString LOAD_FONT_ERROR_INFO_TEXT = "An error occured while loading font...";


EditFontDialog::EditFontDialog(const String & editFontPresetName, QDialog *parent) :
    QDialog(parent),
    ui(new Ui::EditFontDialog)
{    
    ui->setupUi(this);
    
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(false);
    
    //dialogResult.fontPresetOriginalName = editFontPresetName;
    //dialogResult.fontPresetName = editFontPresetName;
    Font* font = EditorFontManager::Instance()->GetLocalizedFont(editFontPresetName, "default");
    
    //dialogResult.font = font ? font->Clone() : EditorFontManager::Instance()->GetDefaultFont()->Clone();
    
    const Vector<String> &locales = EditorFontManager::Instance()->GetLocales();
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(editFontPresetName, locales[i]);
        //dialogResult.localizedFonts[locales[i]] = localizedFont ? localizedFont->Clone() : dialogResult.font->Clone();
        
        //Logger::FrameworkDebug("EditFontDialog::EditFontDialog dialogResult.localizedFonts[%s] = %p", locales[i].c_str(), dialogResult.localizedFonts[locales[i]]);
        
        ui->selectLocaleComboBox->addItem(QString::fromStdString(locales[i]));
    }
    if(0 < localesCount)
    {
        currentLocale = locales[0];
    }
    
    // Initialize dialog
    ConnectToSignals();
}

EditFontDialog::~EditFontDialog()
{
    DisconnectFromSignals();
    SafeDelete(ui);
}

void EditFontDialog::ConnectToSignals()
{
    //Connect signal and slots
    connect(ui->createNewRadioButton, SIGNAL(clicked()), this, SLOT(OnRadioButtonClicked()));
    connect(ui->applyToAllRadioButton, SIGNAL(clicked()), this, SLOT(OnRadioButtonClicked()));
    
    UpdateLineEditWidgetWithPropertyValue(ui->fontPresetNameLlineEdit);
    
    UpdateSpinBoxWidgetWithPropertyValue(ui->fontSizeSpinBox);
    UpdatePushButtonWidgetWithPropertyValue(ui->fontSelectButton);
    
    UpdateSpinBoxWidgetWithPropertyValue(ui->localizedFontSizeSpinBox);
    UpdatePushButtonWidgetWithPropertyValue(ui->localizedFontSelectButton);
    
    UpdateComboBoxWidgetWithPropertyValue(ui->selectLocaleComboBox);
    
    connect(ui->fontSelectButton, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
    connect(ui->fontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
    
    connect(ui->selectLocaleComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
    
    connect(ui->resetFontForLocalePushButton, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
    
    connect(ui->localizedFontSelectButton, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
    connect(ui->localizedFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
    
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(OnOkButtonClicked()));
}

void EditFontDialog::DisconnectFromSignals()
{
    disconnect(ui->createNewRadioButton, SIGNAL(clicked()), this, SLOT(OnRadioButtonClicked()));
    disconnect(ui->applyToAllRadioButton, SIGNAL(clicked()), this, SLOT(OnRadioButtonClicked()));
    
    disconnect(ui->fontSelectButton, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
    disconnect(ui->fontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
    
    disconnect(ui->selectLocaleComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
    
    disconnect(ui->localizedFontSelectButton, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
    disconnect(ui->localizedFontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
    
    disconnect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(OnButtonBoxButtonClicked(QAbstractButton*)));
}

void EditFontDialog::OnRadioButtonClicked()
{
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(true);
    QRadioButton* senderWidget = dynamic_cast<QRadioButton*>(QObject::sender());
    if(senderWidget == ui->applyToAllRadioButton)
    {
        //dialogResult.isApplyToAll = true;
    }
    else if(senderWidget == ui->createNewRadioButton)
    {
        //dialogResult.isApplyToAll = false;
    }
}
void EditFontDialog::OnPushButtonClicked()
{
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnPushButtonClicked - sender is NULL!");
        return;
    }
    
    ProcessPushButtonClicked(senderWidget);
}

void EditFontDialog::OnSpinBoxValueChanged(int newValue)
{
    QWidget* senderWidget = dynamic_cast<QWidget*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnSpinBoxValueChanged - sender is NULL!");
        return;
    }
    
    if(senderWidget == ui->fontSizeSpinBox || senderWidget == ui->localizedFontSizeSpinBox)
    {
        Font *font = NULL;
        if(senderWidget == ui->fontSizeSpinBox)
        {
            //font = dialogResult.font;
        }
        else if(senderWidget == ui->localizedFontSizeSpinBox)
        {
            //font = dialogResult.GetLocalizedFont(currentLocale);
        }
        
        if(font == NULL)
        {
            Logger::Error("OnSpinBoxValueChanged - font is NULL!");
            return;
        }
        
        if(font)
        {
            font->SetSize(newValue);
        }
    }
}

void EditFontDialog::OnComboBoxValueChanged(QString value)
{
    QComboBox* senderWidget = dynamic_cast<QComboBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnComboBoxValueChanged - sender is NULL!");
        return;
    }
    
    ProcessComboBoxValueChanged(senderWidget, value);
}

void EditFontDialog::ProcessComboBoxValueChanged(QComboBox *senderWidget, const QString& value)
{
    if(senderWidget == ui->selectLocaleComboBox)
    {
        currentLocale = value.toStdString();
        
        Logger::FrameworkDebug("EditFontDialog::ProcessComboBoxValueChanged currentLocale=%s", currentLocale.c_str());
        
        UpdatePushButtonWidgetWithPropertyValue(ui->localizedFontSelectButton);
        UpdateSpinBoxWidgetWithPropertyValue(ui->localizedFontSizeSpinBox);
    }
}

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
/*            dialogResult.SetLocalizedFont(resultFont, currentLocale);*/
            
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
