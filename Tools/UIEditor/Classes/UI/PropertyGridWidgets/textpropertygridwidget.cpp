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
#include "ui_uitextfieldpropertygridwidget.h"

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

static const QString TEXT_PROPERTY_BLOCK_NAME = "Text";

TextPropertyGridWidget::TextPropertyGridWidget(QWidget *parent) :
    UITextFieldPropertyGridWidget(parent)
{
    SetPropertyBlockName(TEXT_PROPERTY_BLOCK_NAME);
	InsertLocalizationFields();
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

TextPropertyGridWidget::~TextPropertyGridWidget()
{
	delete localizationKeyNameLineEdit;
	delete localizationKeyTextLineEdit;
	delete localizationKeyNameLabel;
	delete localizationKeyTextLabel;
	delete multilineCheckBox;
	delete multilineBySymbolCheckBox;
}

void TextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // All these properties are state-aware.
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontSelectButton, false, true);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::FONT_COLOR_PROPERTY_NAME, ui->textColorWidget, false, true);

    // Shadow properties are also state-aware
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox, false, true);
    RegisterColorWidgetForProperty(propertiesMap, PropertyNames::SHADOW_COLOR, ui->shadowColorWidget, false, true);
    // Localized Text Key is handled through generic Property mechanism, but we need to update the
    // Localization Value widget each time Localization Key is changes.
    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, localizationKeyNameLineEdit, false, true);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);

	bool enableTextAlignComboBox = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UITextFieldMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UIButtonMetadata*>(activeMetadata)		!= NULL);

	ui->alignComboBox->setEnabled(enableTextAlignComboBox);
	
	bool isUITextField = (dynamic_cast<UITextFieldMetadata*>(activeMetadata) != NULL);
	ui->isPasswordCheckbox->setVisible(isUITextField);
	ui->autoCapitalizationTypeLabel->setVisible(isUITextField);
	ui->autoCapitalizationTypeComboBox->setVisible(isUITextField);
	ui->autoCorrectionTypeLabel->setVisible(isUITextField);
	ui->autoCorrectionTypeComboBox->setVisible(isUITextField);
	ui->spellCheckingTypeLabel->setVisible(isUITextField);
	ui->spellCheckingTypeComboBox->setVisible(isUITextField);
	ui->keyboardAppearanceTypeLabel->setVisible(isUITextField);
	ui->keyboardAppearanceTypeComboBox->setVisible(isUITextField);
	ui->keyboardTypeLabel->setVisible(isUITextField);
	ui->keyboardTypeComboBox->setVisible(isUITextField);
	ui->returnKeyTypeLabel->setVisible(isUITextField);
	ui->returnKeyTypeComboBox->setVisible(isUITextField);
	ui->isReturnKeyAutomatically->setVisible(isUITextField);
    ui->isPasswordCheckbox->setVisible(isUITextField);

	bool isUIStaticText = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL);

	multilineCheckBox->setEnabled(isUIStaticText);
	multilineCheckBox->setVisible(isUIStaticText);
	multilineBySymbolCheckBox->setEnabled(false); // false by default - this checkbox depends on multilineCheckBox one.
	multilineBySymbolCheckBox->setVisible(isUIStaticText);

	if (isUIStaticText)
	{
		// Register checkbox widget for property Multiline only for UIStaticText
		RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_MULTILINE, multilineCheckBox, false, true);
		RegisterCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_PROPERTY_MULTILINE_BY_SYMBOL, multilineBySymbolCheckBox, false, true);
	}

    // Fitting Type is needed for all text-aware controls but UITextField.
    ui->fittingTypeComboBox->setVisible(!isUITextField);
    ui->fittingLabel->setVisible(!isUITextField);

    if (false == isUITextField)
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

    UpdateLocalizationValue();

    RegisterGridWidgetAsStateAware();
}

void TextPropertyGridWidget::InsertLocalizationFields()
{
	ui->textLineEdit->setEnabled(false);
	ui->textLineEdit->setVisible(false);
	ui->textLabel->setVisible(false);
	
	this->resize(300, 372);
    this->setMinimumSize(QSize(300, 372));

	ui->groupBox->resize(300, 372);
    ui->groupBox->setMinimumSize(QSize(300, 372));
	
	localizationKeyNameLabel = new QLabel(ui->groupBox);
	localizationKeyNameLabel->setObjectName(QString::fromUtf8("localizationKeyNameLabel"));
	localizationKeyNameLabel->setGeometry(QRect(10, 25, 281, 16));
	localizationKeyNameLabel->setText(QString::fromUtf8("Localization Key:"));
	localizationKeyNameLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyNameLineEdit->setObjectName(QString::fromUtf8("localizationKeyNameLineEdit"));
	localizationKeyNameLineEdit->setGeometry(QRect(10, 45, 281, 22));
	
	localizationKeyTextLabel = new QLabel(ui->groupBox);
	localizationKeyTextLabel->setObjectName(QString::fromUtf8("localizationKeyTextLabel"));
	localizationKeyTextLabel->setGeometry(QRect(10, 75, 281, 16));
	localizationKeyTextLabel->setText(QString::fromUtf8("Localization Value for current language:"));
	localizationKeyTextLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyTextLineEdit->setObjectName(QString::fromUtf8("localizationKeyTextLineEdit"));
	localizationKeyTextLineEdit->setEnabled(false);
	localizationKeyTextLineEdit->setGeometry(QRect(10, 95, 281, 22));
	localizationKeyTextLineEdit->setReadOnly(true);
	
	multilineCheckBox = new QCheckBox(ui->groupBox);
	multilineCheckBox->setObjectName(QString::fromUtf8("multilineCheckBox"));
	multilineCheckBox->setGeometry(QRect(10, 125, 200, 20));
	multilineCheckBox->setText(QString::fromUtf8("Multiline"));

	multilineBySymbolCheckBox = new QCheckBox(ui->groupBox);
	multilineBySymbolCheckBox->setObjectName(QString::fromUtf8("multilineBySymbolCheckBox"));
	multilineBySymbolCheckBox->setGeometry(QRect(10, 145, 200, 20));
	multilineBySymbolCheckBox->setText(QString::fromUtf8("Multiline by Symbol"));
	multilineBySymbolCheckBox->setEnabled(false);

	ui->fontNameLabel->setGeometry(QRect(10, 175, 31, 16));
	ui->fontSizeSpinBox->setGeometry(QRect(234, 171, 57, 25));
	ui->fontSelectButton->setGeometry(QRect(50, 166, 181, 38));
    
	ui->fontColorLabel->setGeometry(QRect(10, 250, 71, 16));
	ui->textColorWidget->setGeometry(QRect(105, 248, 184, 21));
	ui->shadowOffsetLabel->setGeometry(QRect(10, 282, 91, 16));
	ui->offsetYLabel->setGeometry(QRect(210, 282, 36, 16));
	ui->shadowColorLabel->setGeometry(QRect(10, 322, 91, 16));
	ui->shadowOffsetXSpinBox->setGeometry(QRect(140, 278, 57, 25));
	ui->shadowOffsetYSpinBox->setGeometry(QRect(230, 278, 57, 25));
    
	ui->shadowColorWidget->setGeometry(QRect(105, 310, 184, 21));
	ui->offsetXLabel->setGeometry(QRect(120, 282, 16, 16));
    
	ui->AlignLabel->setGeometry(QRect(10, 346, 62, 16));
	ui->alignComboBox->setGeometry(QRect(80, 340, 211, 26));
}

void TextPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();
    UnregisterLineEditWidget(localizationKeyNameLineEdit);

    // Don't unregister multiline property for UIButton and UITextField
    if (dynamic_cast<UIStaticTextMetadata*>(this->activeMetadata) != NULL)
    {
        UnregisterCheckBoxWidget(multilineCheckBox);
        UnregisterCheckBoxWidget(multilineBySymbolCheckBox);
    }

    UITextFieldPropertyGridWidget::Cleanup();
}

void TextPropertyGridWidget::UpdateLocalizationValue()
{
    if ( !this->activeMetadata )
    {
        return;
    }
    
    // Key is known now - determine and set the value.
    QString localizationKey = this->localizationKeyNameLineEdit->text();
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(localizationKey));
	this->localizationKeyTextLineEdit->setText(WideString2QStrint(localizationValue));
    
    // Also update the "dirty" style for the "Value"
    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(this->localizationKeyNameLineEdit);
    if (iter != this->propertyGridWidgetsMap.end())
    {
        UpdateWidgetPalette(this->localizationKeyTextLineEdit, iter->second.getProperty().name());
    }
}

void TextPropertyGridWidget::HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
    // When the UI Control State is changed. we need to update Localization Key/Value.
    BasePropertyGridWidget::HandleSelectedUIControlStatesChanged(newStates);
    UpdateLocalizationValue();
}

void TextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);

    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty)
{
    // Yuri Coder, 2013/10/24. "Multiline By Symbol" checkbox should be unchecked and disabled if
    // "Multiline" one is unchecked - see please DF-2393.
    if (checkBoxWidget && multilineCheckBox && checkBoxWidget == multilineCheckBox)
    {
        bool multilineChecked = (checkBoxWidget->checkState() == Qt::Checked);
        multilineBySymbolCheckBox->setEnabled(multilineChecked);

        if (!multilineChecked)
        {
            multilineBySymbolCheckBox->setCheckState(Qt::Unchecked);
        }
    }

    UITextFieldPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);
}

void TextPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
	if (!this->activeMetadata)
    {
        return;
    }
    
    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);
    
    if (comboBoxWidget == ui->fittingTypeComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(ui->fittingTypeComboBox, BackgroundGridWidgetHelper::GetFittingTypeDescByType(propertyValue) );
    }
    
    return UITextFieldPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

void TextPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                                               const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("TextPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
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

    // No postprocessing was applied - use the generic process.
    UITextFieldPropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}
