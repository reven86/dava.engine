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
#include "ui_textpropertygridwidget.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "UITextControlMetadata.h"
#include "PropertiesHelper.h"
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "WidgetSignalsBlocker.h"
#include "fontmanagerdialog.h"
#include "PropertyNames.h"
#include "ResourcesManageHelper.h"
#include "BackgroundGridWidgetHelper.h"
#include "UIStaticTextMetadata.h"
#include "UITextFieldMetadata.h"
#include "UIButtonMetadata.h"

#include "StringUtils.h"

#include <QLabel>

using namespace DAVA;

UITextFieldPropertyGridWidget::UITextFieldPropertyGridWidget(QWidget *parent) :
TextPropertyGridWidget(parent)
{
}

UITextFieldPropertyGridWidget::~UITextFieldPropertyGridWidget()
{
}

void UITextFieldPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	TextPropertyGridWidget::Initialize(activeMetadata);

    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_NAME, ui->textLineEdit);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::TEXT_COLOR_PROPERTY_NAME, ui->textColorWidget);
    
	RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::IS_PASSWORD_PROPERTY_NAME, ui->isPasswordCheckbox);
    
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::AUTO_CAPITALIZATION_TYPE_PROPERTY_NAME, ui->autoCapitalizationTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::AUTO_CORRECTION_TYPE_PROPERTY_NAME, ui->autoCorrectionTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::SPELL_CHECKING_TYPE_PROPERTY_NAME, ui->spellCheckingTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::KEYBOARD_APPEARANCE_TYPE_PROPERTY_NAME, ui->keyboardAppearanceTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::KEYBOARD_TYPE_PROPERTY_NAME, ui->keyboardTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::RETURN_KEY_TYPE_PROPERTY_NAME, ui->returnKeyTypeComboBox);
    
	RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::IS_RETURN_KEY_PROPERTY_NAME, ui->isReturnKeyAutomatically);
    
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::MAX_TEXT_LENGTH_PROPERTY_NAME, ui->maxTextLengthSpinBox);

	//bool isUITextField = (dynamic_cast<UITextFieldMetadata*>(activeMetadata) != NULL);
    bool isUITextField = true; // in any case uitexfieldpropertygridwidget is only used with UITextField metadata
    
    ui->textWidget->setVisible(isUITextField);
    
    ui->isPasswordWidget->setVisible(isUITextField);
    ui->autoCapitalizationTypeWidget->setVisible(isUITextField);
    ui->autoCorrectionTypeWidget->setVisible(isUITextField);
    ui->spellCheckingTypeWidget->setVisible(isUITextField);
    ui->keyboardAppearanceWidget->setVisible(isUITextField);
    ui->keyboardTypeWidget->setVisible(isUITextField);
    ui->returnKeyTypeWidget->setVisible(isUITextField);
    ui->isReturnKeyAutoWidget->setVisible(isUITextField);
    ui->isPasswordWidget->setVisible(isUITextField);
    ui->maxTextLengthWidget->setVisible(isUITextField);
    
	//bool isUIStaticText = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL);
    // in any case uitexfieldpropertygridwidget is not used with UIStaticText metadata
    
	ui->multilineCheckBox->setEnabled(false);
	ui->multilineBySymbolCheckBox->setEnabled(false); // false by default - this checkbox depends on multilineCheckBox one.
    ui->multilineWidget->setVisible(false);
    
    // Fitting Type is needed for all text-aware controls but UITextField.
    ui->fittingWidget->setVisible(!isUITextField);
    ui->localizationKeyWidget->setVisible(!isUITextField);
    ui->localizationValueWidget->setVisible(!isUITextField);
}

void UITextFieldPropertyGridWidget::Cleanup()
{
	UnregisterCheckBoxWidget(ui->isPasswordCheckbox);
    UnregisterColorWidget(ui->textColorWidget);

	UnregisterComboBoxWidget(ui->autoCapitalizationTypeComboBox);
	UnregisterComboBoxWidget(ui->autoCorrectionTypeComboBox);
	UnregisterComboBoxWidget(ui->spellCheckingTypeComboBox);
	UnregisterComboBoxWidget(ui->keyboardAppearanceTypeComboBox);
	UnregisterComboBoxWidget(ui->keyboardTypeComboBox);
	UnregisterComboBoxWidget(ui->returnKeyTypeComboBox);
	UnregisterCheckBoxWidget(ui->isReturnKeyAutomatically);
    UnregisterSpinBoxWidget(ui->maxTextLengthSpinBox);

    TextPropertyGridWidget::Cleanup();
}

void UITextFieldPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                                         const QString& value)
{
	if (senderWidget == NULL)
    {
        Logger::Error("TextPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
	if (senderWidget == ui->autoCapitalizationTypeComboBox)
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
    TextPropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
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
    if (comboBoxWidget == ui->autoCapitalizationTypeComboBox)
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
    TextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

void UITextFieldPropertyGridWidget::FillComboboxes()
{
    TextPropertyGridWidget::FillComboboxes();
    
    int itemsCount = 0;
	
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
