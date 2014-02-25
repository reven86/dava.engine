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


#include "uitextfieldpropertygridwidget.h"
#include "ui_uitextfieldpropertygridwidget.h"
#include "fontmanagerdialog.h"
#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "ResourcesManageHelper.h"
#include "PropertyNames.h"
#include "PropertiesHelper.h"
#include "WidgetSignalsBlocker.h"
#include "BackgroundGridWidgetHelper.h"

static const QString TEXTFIELD_PROPERTY_BLOCK_NAME = "Text";

UITextFieldPropertyGridWidget::UITextFieldPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::UITextFieldPropertyGridWidget)
{
    ui->setupUi(this);
	SetPropertyBlockName(TEXTFIELD_PROPERTY_BLOCK_NAME);
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

UITextFieldPropertyGridWidget::~UITextFieldPropertyGridWidget()
{
    delete ui;
}

void UITextFieldPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterPushButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontSelectButton);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox);
    
    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_NAME, ui->textLineEdit);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::TEXT_COLOR_PROPERTY_NAME, ui->textColorWidget);

    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::SHADOW_COLOR, ui->shadowColorWidget);

	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);

	RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::IS_PASSWORD_PROPERTY_NAME, ui->isPasswordCheckbox);
	
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::AUTO_CAPITALIZATION_TYPE_PROPERTY_NAME, ui->autoCapitalizationTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::AUTO_CORRECTION_TYPE_PROPERTY_NAME, ui->autoCorrectionTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::SPELL_CHECKING_TYPE_PROPERTY_NAME, ui->spellCheckingTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::KEYBOARD_APPEARANCE_TYPE_PROPERTY_NAME, ui->keyboardAppearanceTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::KEYBOARD_TYPE_PROPERTY_NAME, ui->keyboardTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::RETURN_KEY_TYPE_PROPERTY_NAME, ui->returnKeyTypeComboBox);
	RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::IS_RETURN_KEY_PROPERTY_NAME, ui->isReturnKeyAutomatically);
    
    // Fitting is not needed for UITextField.
    ui->fittingTypeComboBox->setVisible(false);
    ui->fittingLabel->setVisible(false);
}

void UITextFieldPropertyGridWidget::Cleanup()
{
    UnregisterPushButtonWidget(ui->fontSelectButton);
    UnregisterSpinBoxWidget(ui->fontSizeSpinBox);
    UnregisterColorWidget(ui->textColorWidget);

    UnregisterSpinBoxWidget(ui->shadowOffsetXSpinBox);
    UnregisterSpinBoxWidget(ui->shadowOffsetYSpinBox);
    UnregisterColorWidget(ui->shadowColorWidget);

	UnregisterComboBoxWidget(ui->alignComboBox);

	UnregisterCheckBoxWidget(ui->isPasswordCheckbox);
	
	UnregisterComboBoxWidget(ui->autoCapitalizationTypeComboBox);
	UnregisterComboBoxWidget(ui->autoCorrectionTypeComboBox);
	UnregisterComboBoxWidget(ui->spellCheckingTypeComboBox);
	UnregisterComboBoxWidget(ui->keyboardAppearanceTypeComboBox);
	UnregisterComboBoxWidget(ui->keyboardTypeComboBox);
	UnregisterComboBoxWidget(ui->returnKeyTypeComboBox);
	UnregisterCheckBoxWidget(ui->isReturnKeyAutomatically);

    BasePropertyGridWidget::Cleanup();
}

void UITextFieldPropertyGridWidget::ProcessPushButtonClicked(QPushButton *senderWidget)
{
	    if ((activeMetadata == NULL) || (senderWidget != this->ui->fontSelectButton))
    {
        // No control already assinged or not fontSelectButton
        return;
    }
    
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
}

void UITextFieldPropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty &curProperty)
{
    
    if (pushButtonWidget != this->ui->fontSelectButton)
    {
        return; //Not font select button
    }
    
    bool isPropertyValueDiffers = false;
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                         curProperty.name(), isPropertyValueDiffers);
    if (fontPropertyValue)
    {
        //Set button text
        WidgetSignalsBlocker blocker(pushButtonWidget);
        Font::eFontType fontType = fontPropertyValue->GetFontType();
        QString buttonText;
        
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(fontPropertyValue);
                //Set pushbutton widget text
				buttonText = QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath());
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(fontPropertyValue);
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
            default:
            {
                //Do nothing if we can't determine font type
                return;
            }
        }
        
        pushButtonWidget->setText(buttonText);
    }
}


void UITextFieldPropertyGridWidget::FillComboboxes()
{
    ui->alignComboBox->clear();
    int itemsCount = BackgroundGridWidgetHelper::GetAlignTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->alignComboBox->addItem(BackgroundGridWidgetHelper::GetAlignTypeDesc(i));
    }
	
	ui->autoCapitalizationTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetAutoCapitalizationTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->autoCapitalizationTypeComboBox->addItem(BackgroundGridWidgetHelper::GetAutoCapitalizationTypeDesc(i));
	}
	
	ui->autoCorrectionTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetAutoCorrectionTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->autoCorrectionTypeComboBox->addItem(BackgroundGridWidgetHelper::GetAutoCorrectionTypeDesc(i));
	}
	
	ui->spellCheckingTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetSpellCheckingTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->spellCheckingTypeComboBox->addItem(BackgroundGridWidgetHelper::GetSpellCheckingTypeDesc(i));
	}
	
	ui->keyboardAppearanceTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetKeyboardAppearanceTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->keyboardAppearanceTypeComboBox->addItem(BackgroundGridWidgetHelper::GetKeyboardAppearanceTypeDesc(i));
	}
	
	ui->keyboardTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetKeyboardTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->keyboardTypeComboBox->addItem(BackgroundGridWidgetHelper::GetKeyboardTypeDesc(i));
	}
	
	ui->keyboardTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetKeyboardTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->keyboardTypeComboBox->addItem(BackgroundGridWidgetHelper::GetKeyboardTypeDesc(i));
	}
	
	ui->returnKeyTypeComboBox->clear();
	itemsCount = BackgroundGridWidgetHelper::GetReturnKeyTypesCount();
	for (int i = 0; i < itemsCount; i ++)
	{
		ui->returnKeyTypeComboBox->addItem(BackgroundGridWidgetHelper::GetReturnKeyTypeDesc(i));
	}
}

void UITextFieldPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
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

void UITextFieldPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value)
{
	if (senderWidget == NULL)
    {
        Logger::Error("UITextFieldPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();

	if (senderWidget == ui->alignComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAlignType(selectedIndex));
    }
	else if (senderWidget == ui->autoCapitalizationTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAutoCapitalizationType(selectedIndex));
    }
	else if (senderWidget == ui->autoCorrectionTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAutoCorrectionType(selectedIndex));
    }
	else if (senderWidget == ui->spellCheckingTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetSpellCheckingType(selectedIndex));
    }
	else if (senderWidget == ui->keyboardAppearanceTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetKeyboardAppearanceType(selectedIndex));
    }
	else if (senderWidget == ui->keyboardTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetKeyboardType(selectedIndex));
    }
	else if (senderWidget == ui->returnKeyTypeComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetReturnKeyType(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void UITextFieldPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
	if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->alignComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetAlignTypeDescByType(propertyValue));
    }
	else if (comboBoxWidget == ui->autoCapitalizationTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									BackgroundGridWidgetHelper::GetAutoCapitalizationTypeDescByType(propertyValue));
	}
	else if (comboBoxWidget == ui->autoCorrectionTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									   BackgroundGridWidgetHelper::GetAutoCorrectionTypeDescByType(propertyValue));
	}
	else if (comboBoxWidget == ui->spellCheckingTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									   BackgroundGridWidgetHelper::GetSpellCheckingTypeDescByType(propertyValue));
	}
	else if (comboBoxWidget == ui->keyboardAppearanceTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									   BackgroundGridWidgetHelper::GetKeyboardAppearanceTypeDescByType(propertyValue));
	}
	else if (comboBoxWidget == ui->keyboardTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									   BackgroundGridWidgetHelper::GetKeyboardTypeDescByType(propertyValue));
	}
	else if (comboBoxWidget == ui->returnKeyTypeComboBox)
	{
		UpdateWidgetPalette(comboBoxWidget, propertyName);
		return SetComboboxSelectedItem(comboBoxWidget,
									   BackgroundGridWidgetHelper::GetReturnKeyTypeDescByType(propertyValue));
	}

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}
