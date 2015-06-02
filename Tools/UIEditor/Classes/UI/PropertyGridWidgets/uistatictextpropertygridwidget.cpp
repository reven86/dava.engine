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


#include "uistatictextpropertygridwidget.h"
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

UIStaticTextPropertyGridWidget::UIStaticTextPropertyGridWidget(QWidget *parent) :
TextPropertyGridWidget(parent)
{
}

UIStaticTextPropertyGridWidget::~UIStaticTextPropertyGridWidget()
{
}

void UIStaticTextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	TextPropertyGridWidget::Initialize(activeMetadata);
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // All these properties are state-aware.
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::FONT_COLOR_PROPERTY_NAME, ui->textColorWidget, false, true);
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_COLOR_INHERIT_TYPE_PROPERTY_NAME, ui->colorInheritTypeCombobox, false, true);
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_PER_PIXEL_ACCURACY_TYPE_PROPERTY_NAME,
    													ui->perPixelAccuracyTypeCombobox, false, true);

    // Localized Text Key is handled through generic Property mechanism, but we need to update the
    // Localization Value widget each time Localization Key is changes.
    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, ui->localizationKeyNameLineEdit, false, true);

    //bool isUITextField = (dynamic_cast<UITextFieldMetadata*>(activeMetadata) != NULL);
    // in any case uitexfieldpropertygridwidget is not used with UIStaticText metadata
    bool isUITextField = false;
    
    //if (false == isUITextField)
    {
        WidgetSignalsBlocker blocker(ui->fittingTypeComboBox);
        ui->fittingTypeComboBox->clear();
        int itemsCount = BackgroundGridWidgetHelper::GetFittingTypesCount();
        for (int i = 0; i < itemsCount; i ++)
        {
            ui->fittingTypeComboBox->addItem(BackgroundGridWidgetHelper::GetFittingTypeDesc(i));
        }
        
        RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_FITTING_TYPE_PROPERTY_NAME, ui->fittingTypeComboBox, false, true);
    }
    
    //bool isUIStaticText = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL);

    //DF-3280 multiline should be enabled for UIStaticText and UIButton
    ui->multilineCheckBox->setEnabled(true);
    ui->multilineBySymbolCheckBox->setEnabled(false); // false by default - this checkbox depends on multilineCheckBox one.
    ui->multilineWidget->setVisible(true);

    // Register checkbox widget for property Multiline only for (DF-3280) UIStaticText and UIButton
    RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_MULTILINE, ui->multilineCheckBox, false, true);
    RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_MULTILINE_BY_SYMBOL, ui->multilineBySymbolCheckBox, false, true);
    
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
    
    // Fitting Type is needed for all text-aware controls but UITextField.
    ui->fittingWidget->setVisible(!isUITextField);
    
    ui->localizationKeyWidget->setVisible(!isUITextField);
    ui->localizationValueWidget->setVisible(!isUITextField);
    
    ui->localizationKeyTextLineEdit->setEnabled(true);
    ui->localizationKeyTextLineEdit->setReadOnly(true);

    UpdateLocalizationValue();
    
    RegisterGridWidgetAsStateAware();
}

void UIStaticTextPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();
    UnregisterLineEditWidget(ui->localizationKeyNameLineEdit);
    UnregisterColorWidget(ui->textColorWidget);

    // DF-3280 multiline should be enabled for UIStaticText and UIButton - unregister checkbox
    //if (dynamic_cast<UIStaticTextMetadata*>(this->activeMetadata) != NULL)
    {
        UnregisterCheckBoxWidget(ui->multilineCheckBox);
        UnregisterCheckBoxWidget(ui->multilineBySymbolCheckBox);
    }
    
    UnregisterComboBoxWidget(ui->fittingTypeComboBox);
    UnregisterComboBoxWidget(ui->colorInheritTypeCombobox);
    UnregisterComboBoxWidget(ui->perPixelAccuracyTypeCombobox);
    
    TextPropertyGridWidget::Cleanup();
}

void UIStaticTextPropertyGridWidget::UpdateLocalizationValue()
{
    if ( !this->activeMetadata )
    {
        return;
    }
    
    // Key is known now - determine and set the value.
    QString localizationKey = ui->localizationKeyNameLineEdit->text();
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(localizationKey));
	ui->localizationKeyTextLineEdit->setText(WideString2QString(localizationValue));
    
    // Also update the "dirty" style for the "Value"
    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(ui->localizationKeyNameLineEdit);
    if (iter != this->propertyGridWidgetsMap.end())
    {
        UpdateWidgetPalette(ui->localizationKeyTextLineEdit, iter->second.getProperty().name());
    }
}

void UIStaticTextPropertyGridWidget::HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
    // When the UI Control State is changed. we need to update Localization Key/Value.
    BasePropertyGridWidget::HandleSelectedUIControlStatesChanged(newStates);
    UpdateLocalizationValue();
}

void UIStaticTextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    TextPropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(ui->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void UIStaticTextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    TextPropertyGridWidget::HandleChangePropertyFailed(propertyName);
    
    if (IsWidgetBoundToProperty(ui->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void UIStaticTextPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty)
{
    TextPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);

    // Yuri Coder, 2013/10/24. "Multiline By Symbol" checkbox should be unchecked and disabled if
    // "Multiline" one is unchecked - see please DF-2393.
    if (checkBoxWidget && ui->multilineCheckBox && checkBoxWidget == ui->multilineCheckBox)
    {
        bool multilineChecked = (checkBoxWidget->checkState() == Qt::Checked);
        ui->multilineBySymbolCheckBox->setEnabled(multilineChecked);
        
        if (!multilineChecked)
        {
            ui->multilineBySymbolCheckBox->setCheckState(Qt::Unchecked);
        }
    }
}

void UIStaticTextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
	if (!this->activeMetadata)
    {
        return;
    }
    
    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    
    if (comboBoxWidget == ui->fittingTypeComboBox)
    {
        int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);
        
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(ui->fittingTypeComboBox, BackgroundGridWidgetHelper::GetFittingTypeDescByType(propertyValue) );
    }
    
    if (comboBoxWidget == ui->colorInheritTypeCombobox)
    {
        int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
            BackgroundGridWidgetHelper::GetColorInheritTypeDescByType((UIControlBackground::eColorInheritType)propertyValue));
    }
    
    if (comboBoxWidget == ui->perPixelAccuracyTypeCombobox)
    {
        int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
            BackgroundGridWidgetHelper::GetPerPixelAccuracyTypeDescByType((UIControlBackground::ePerPixelAccuracyType)propertyValue));
    }

    return TextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

void UIStaticTextPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("UIStaticTextPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    if (senderWidget == ui->fittingTypeComboBox)
    {
        int selectedIndex = senderWidget->currentIndex();
        int curFittingType = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
        int newFittingType = BackgroundGridWidgetHelper::GetFittingType(selectedIndex);
        
        if (curFittingType == newFittingType)
        {
            return;
        }
        
        BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, newFittingType);
        CommandsController::Instance()->ExecuteCommand(command);
        SafeRelease(command);
        
        return;
    }

    if (senderWidget == ui->colorInheritTypeCombobox)
    {
        int selectedIndex = senderWidget->currentIndex();
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetColorInheritType(selectedIndex));
    }
    
    if (senderWidget == ui->perPixelAccuracyTypeCombobox)
    {
        int selectedIndex = senderWidget->currentIndex();
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetPerPixelAccuracyType(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    TextPropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void UIStaticTextPropertyGridWidget::FillComboboxes()
{
    TextPropertyGridWidget::FillComboboxes();

    WidgetSignalsBlocker colorInheritTypeBlocker(ui->colorInheritTypeCombobox);
    WidgetSignalsBlocker perPixelAccuracyTypeBlocker(ui->perPixelAccuracyTypeCombobox);
    
    ui->colorInheritTypeCombobox->clear();
    ui->perPixelAccuracyTypeCombobox->clear();

    int32 itemsCount = BackgroundGridWidgetHelper::GetColorInheritTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->colorInheritTypeCombobox->addItem(BackgroundGridWidgetHelper::GetColorInheritTypeDesc(i));
    }
    
    itemsCount = BackgroundGridWidgetHelper::GetPerPixelAccuracyTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->perPixelAccuracyTypeCombobox->addItem(BackgroundGridWidgetHelper::GetPerPixelAccuracyTypeDesc(i));
    }
}
