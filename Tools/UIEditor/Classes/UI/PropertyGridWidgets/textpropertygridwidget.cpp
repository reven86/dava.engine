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


#include "textpropertygridwidget.h"
#include "ui_textpropertygridwidget.h"

#include "fontmanagerdialog.h"
#include "CommandsController.h"
#include "ChangeFontPropertyCommand.h"
#include "UITextControlMetadata.h"
#include "ResourcesManageHelper.h"
#include "PropertyNames.h"
#include "PropertiesHelper.h"
#include "WidgetSignalsBlocker.h"
#include "BackgroundGridWidgetHelper.h"

#include "EditorFontManager.h"

#include "editfontdialog.h"

static const QString TEXTFIELD_PROPERTY_BLOCK_NAME = "Text";
static const String TEXT_MARGINS_PROPERTY_PREFIX = "Text";

TextPropertyGridWidget::TextPropertyGridWidget(QWidget *parent) :
BasePropertyGridWidget(parent),
ui(new Ui::TextPropertyGridWidget)
{
    ui->setupUi(this);
    ui->marginsWidget->SetPropertyPrefix(TEXT_MARGINS_PROPERTY_PREFIX);

	SetPropertyBlockName(TEXTFIELD_PROPERTY_BLOCK_NAME);
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

TextPropertyGridWidget::~TextPropertyGridWidget()
{
    delete ui;
}

void TextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    ui->marginsWidget->Initialize(activeMetadata);

	FillComboboxes();
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    FillFontPresetCombobox();
    
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontPresetComboBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontPresetEditButton, false, true);
    
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox, false, true);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::SHADOW_COLOR, ui->shadowColorWidget, false, true);
    
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);
    
    //bool enableTextAlignComboBox = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL||
	//								dynamic_cast<UITextFieldMetadata*>(activeMetadata)	!= NULL||
	//								dynamic_cast<UIButtonMetadata*>(activeMetadata)		!= NULL);
	//ui->alignComboBox->setEnabled(enableTextAlignComboBox);
    ui->alignComboBox->setEnabled(true); // in any case metadata is one of: UIStaticTextMetadata, UITextFieldMetadata, UIButtonMetadata
    
	RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_USE_RTL_ALIGN_PROPERTY_NAME, ui->useRtlAlignCheckBox, false, true);
	ui->useRtlAlignCheckBox->setEnabled(true);
    
    // register font and size to display selected font and size, disable editing
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontSelectButton, false, true);
    
    ui->fontPresetComboBox->setEnabled(true);
    ui->fontPresetEditButton->setEnabled(true);
    
    ui->fontSelectButton->setEnabled(false);
    ui->fontSizeSpinBox->setEnabled(false);
}

void TextPropertyGridWidget::Cleanup()
{
    ui->marginsWidget->Cleanup();

    UnregisterPushButtonWidget(ui->fontSelectButton);
    UnregisterSpinBoxWidget(ui->fontSizeSpinBox);
    
    UnregisterSpinBoxWidget(ui->shadowOffsetXSpinBox);
    UnregisterSpinBoxWidget(ui->shadowOffsetYSpinBox);
    UnregisterColorWidget(ui->shadowColorWidget);
    
    UnregisterComboBoxWidget(ui->alignComboBox);
    UnregisterCheckBoxWidget(ui->useRtlAlignCheckBox);
	
    UnregisterComboBoxWidget(ui->fontPresetComboBox);
    UnregisterPushButtonWidget(ui->fontPresetEditButton);
    
    BasePropertyGridWidget::Cleanup();
}

void TextPropertyGridWidget::FillFontPresetCombobox()
{
    // fill font presets combo box
    WidgetSignalsBlocker blocker(ui->fontPresetComboBox);
    ui->fontPresetComboBox->clear();
    
    //const Map<Font*, String> &fonts = FontManager::Instance()->GetRegisteredFonts();
    //Map<Font*, String> ::const_iterator it = fonts.begin();
    //Map<Font*, String> ::const_iterator endIt = fonts.end();
    
    // Get current value of Font property
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                         PropertyNames::FONT_PROPERTY_NAME,
                                                                         false);
    
    const Map<String, Font*> &fonts = EditorFontManager::Instance()->GetLocalizedFonts("default");
    
    Map<String, Font*> ::const_iterator it = fonts.begin();
    Map<String, Font*> ::const_iterator endIt = fonts.end();
    QString fontPresetName;
    for(; it != endIt; ++it)
    {
        fontPresetName = QString::fromStdString(it->first);
        ui->fontPresetComboBox->addItem(fontPresetName);
    }
    
    UITextControlMetadata* textMetaData = dynamic_cast<UITextControlMetadata*>(activeMetadata);
    if(textMetaData)
    {
        //Map<Font*, String> ::const_iterator findIt = fonts.find(fontPropertyValue);
        //if(findIt != endIt)
        String fontName = EditorFontManager::Instance()->GetLocalizedFontName(fontPropertyValue);

        // Setup combo box value
        int index = ui->fontPresetComboBox->findText(QString::fromStdString(fontName));
        if(index != -1)
        {
            ui->fontPresetComboBox->setCurrentIndex(index);
        }
        else
        {
            Logger::Warning("TextPropertyGridWidget::Initialize font not found in presets - failed to set font preset");
        }
    }
    else
    {
        Logger::Warning("TextPropertyGridWidget::Initialize not UITextControlMetadata - failed to set font preset");
    }
    
}

void TextPropertyGridWidget::UpdateFontPresetValues()
{
    //TODO: what is it for?
//    // Also update the "dirty" style for the "Value"
//    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(ui->fontSizeSpinBox);
//    if (iter != this->propertyGridWidgetsMap.end())
//    {
//        UpdateWidgetPalette(ui->fontSizeSpinBox, iter->second.getProperty().name());
//    }
    
    FillFontPresetCombobox();
    
    //int fontSize = BasePropertyGridWidget::GetPropertyIntValue(PropertyNames::FONT_SIZE_PROPERTY_NAME);
    // Get current value of Font property
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                         PropertyNames::FONT_PROPERTY_NAME,
                                                                         false);
    if(fontPropertyValue)
    {
        //TODO: remove this workaround
        Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(fontPropertyValue);
        
        if(localizedFont)
        {
            int fontSize = localizedFont->GetSize();
            ui->fontSizeSpinBox->setValue(fontSize);
        
            UpdatePushButtonWidgetWithFont(this->ui->fontSelectButton, localizedFont);
        }
        else
        {
            Logger::Warning("TextPropertyGridWidget::UpdateFontPresetValues failed to get localized font");
        }
    }
    else
    {
        Logger::Warning("TextPropertyGridWidget::UpdateFontPresetValues failed to get font preset");
    }
}

void TextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(ui->fontPresetComboBox, propertyName))
    {
        // font preset updated - update size
        UpdateFontPresetValues();
    }
}

void TextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);
    
    if (IsWidgetBoundToProperty(ui->fontPresetComboBox, propertyName))
    {
        // font preset updated - update size
        UpdateFontPresetValues();
    }
}

void TextPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty)
{
    BasePropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);
}

void TextPropertyGridWidget::ProcessPushButtonClicked(QPushButton *senderWidget)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged or not fontSelectButton
        return;
    }
    
    if(senderWidget == this->ui->fontSelectButton)
    {
        //TODO: remove this code or modify it to set preset instead of setting font
#if 0
        
        // Get current value of Font property
        Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                             PropertyNames::FONT_PROPERTY_NAME,
                                                                             false);
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
        
        PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
        if (iter == propertyGridWidgetsMap.end())
        {
            Logger::Error("OnPushButtonClicked - unable to find attached property in the propertyGridWidgetsMap!");
            return;
        }
        
        // Don't update the property if the text wasn't actually changed.
        Font* curValue = PropertiesHelper::GetAllPropertyValues<Font*>(this->activeMetadata, iter->second.getProperty().name());
        if (curValue && curValue->IsEqual(resultFont))
        {
            SafeRelease(resultFont);
            return;
        }
        
        BaseCommand* command = new ChangePropertyCommand<Font *>(activeMetadata, iter->second, resultFont);
        CommandsController::Instance()->ExecuteCommand(command);
        SafeRelease(command);
        // TODO - probable memory leak. Need to investigate how to fix it
        // SafeRelease(resultFont);
#endif
    }
    else if(senderWidget == this->ui->fontPresetEditButton)
    {
        // open font preset dialog, edit preset, apply changes
        
        // Get current value of Font property
        Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                             PropertyNames::FONT_PROPERTY_NAME,
                                                                             false);
        String fontPresetName = EditorFontManager::Instance()->GetLocalizedFontName(fontPropertyValue);
        
        Logger::FrameworkDebug("TextPropertyGridWidget::ProcessPushButtonClicked fontPropertyValue=%p fontPresetName=%s", fontPropertyValue, fontPresetName.c_str());
        // Get sprite path from graphics font
        //QString currentGFontPath = ResourcesManageHelper::GetGraphicsFontPath(fontPropertyValue);
        
        //Call font selection dialog - with ok button and preset of graphics font path
        //FontManagerDialog *fontDialog = new FontManagerDialog(true, currentGFontPath);
        EditFontDialog *editFontDialog = new EditFontDialog(fontPresetName);
        
        if ( editFontDialog->exec() == QDialog::Accepted )
        {
            PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
            if (iter == propertyGridWidgetsMap.end())
            {
                Logger::Error("OnPushButtonClicked - unable to find attached property in the propertyGridWidgetsMap!");
                SafeDelete(editFontDialog);
                return;
            }
            
            BaseCommand* command = new ChangeFontPropertyCommand(activeMetadata, iter->second, editFontDialog->GetResult());
            CommandsController::Instance()->ExecuteCommand(command);
            SafeRelease(command);
        }
        
        //Delete font select dialog reference
        SafeDelete(editFontDialog);
    }
}

void TextPropertyGridWidget::UpdatePushButtonWidgetWithFont(QPushButton *pushButtonWidget, Font* font)
{
    if (pushButtonWidget != this->ui->fontSelectButton)
    {
        return; //Not font select button
    }
    
    if (font)
    {
        //Set button text
        Font::eFontType fontType = font->GetFontType();
        QString buttonText;
        
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = static_cast<FTFont*>(font);
                //Set pushbutton widget text
				buttonText = QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath());
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = static_cast<GraphicsFont*>(font);
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
                DFFont *dfFont = static_cast<DFFont*>(font);
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
        
        pushButtonWidget->setText(buttonText);
    }
}

//void TextPropertyGridWidget::UpdateSpinBoxWidgetWithPropertyValue(QSpinBox* spinBoxWidget,
//                                                                  const QMetaProperty& curProperty)
//{
//    if (pushButtonWidget != this->ui->fontSelectButton)
//    {
//        return; //Not font select button
//    }
//    
//    // Get the current value.
//    bool isPropertyValueDiffers = false;
//    float propertyValue =
//    PropertiesHelper::GetAllPropertyValues<float>(this->activeMetadata,
//                                                  curProperty.name(),
//                                                  isPropertyValueDiffers);
//    
//    {
//        // For Spin Box set some value irregardless of isPropertyValueDiffers flag.
//        WidgetSignalsBlocker blocker(spinBoxWidget);
//        spinBoxWidget->setValue(propertyValue);
//    }
//    
//    
//    BasePropertyGridWidget::UpdateSpinBoxWidgetWithPropertyValue(spinBoxWidget, curProperty);
//}

void TextPropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty &curProperty)
{
    bool isPropertyValueDiffers = false;
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                         curProperty.name(), isPropertyValueDiffers);
    UpdatePushButtonWidgetWithFont(pushButtonWidget, fontPropertyValue);
}

void TextPropertyGridWidget::FillComboboxes()
{
    ui->alignComboBox->clear();
    int itemsCount = BackgroundGridWidgetHelper::GetAlignTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->alignComboBox->addItem(BackgroundGridWidgetHelper::GetAlignTypeDesc(i));
    }
}

void TextPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
{
    // Don't update the property if the text wasn't actually changed.
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}
    
	BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void TextPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                                                const QString& value)
{
	if (senderWidget == NULL)
    {
        Logger::Error("TextPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
    if (senderWidget == ui->alignComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAlignType(selectedIndex));
    }
	else if(senderWidget == ui->fontPresetComboBox)
    {
        Font* curFont = PropertiesHelper::GetAllPropertyValues<Font*>(this->activeMetadata, iter->second.getProperty().name());
        QString curFontPresetName = QString::fromStdString(EditorFontManager::Instance()->GetLocalizedFontName(curFont));
        QString newFontPresetName = senderWidget->currentText();
        Font* newFont = EditorFontManager::Instance()->GetLocalizedFont(newFontPresetName.toStdString(), LocalizationSystem::Instance()->GetCurrentLocale());
        
        Logger::FrameworkDebug("TextPropertyGridWidget::ProcessComboboxValueChanged curFont=%p (%s) newFont=%p (%s)", curFont, curFontPresetName.toStdString().c_str(), newFont, newFontPresetName.toStdString().c_str());
        
        if(!newFont)
        {
            Logger::Warning("TextPropertyGridWidget::ProcessComboboxValueChanged newFont==NULL");
            return;
        }
        
        if(curFont == newFont)
        {
            return;
        }
        
        BaseCommand* setFontCommand = new ChangePropertyCommand<Font *>(activeMetadata, iter->second, newFont);
        CommandsController::Instance()->ExecuteCommand(setFontCommand);
        SafeRelease(setFontCommand);
        return;
    }
    
    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void TextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
	if (!this->activeMetadata)
    {
        return;
    }
    
    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    
    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->alignComboBox)
    {
        int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);
        
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetAlignTypeDescByType(propertyValue));
    }
	else if(comboBoxWidget == ui->fontPresetComboBox)
    {
        Font* propertyValue = PropertiesHelper::GetPropertyValue<Font*>(this->activeMetadata, propertyName, isPropertyValueDiffers);
        
        QString fontPresetName = QString::fromStdString(EditorFontManager::Instance()->GetLocalizedFontName(propertyValue));
        
        UpdateWidgetPalette(comboBoxWidget, propertyName); // what is it for? - needed for controls like UIButton
        
//        int index = comboBoxWidget->findText(fontPresetName); //TODO: remove debug log
//        Logger::Debug("TextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue %s index=%d", fontPresetName.toStdString().c_str(), index);
        
        return SetComboboxSelectedItem(ui->fontPresetComboBox, fontPresetName );
    }
    
    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}


