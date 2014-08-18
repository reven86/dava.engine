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


#include "flagspropertygridwidget.h"
#include "ui_flagspropertygridwidget.h"

#include "WidgetSignalsBlocker.h"
#include "UIControlStateHelper.h"
#include "PropertiesHelper.h"
#include "UIStaticTextMetadata.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"

static const QString FLAGS_PROPERTY_BLOCK_NAME = "Flags";

FlagsPropertyGridWidget::FlagsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::FlagsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(FLAGS_PROPERTY_BLOCK_NAME);

	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

FlagsPropertyGridWidget::~FlagsPropertyGridWidget()
{
    delete ui;
}

void FlagsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Visible", ui->visibleCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Input", ui->inputCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "ClipContents", ui->clipContentsCheckbox);
	
	RegisterComboBoxWidgetForProperty(propertiesMap, "InitialState", ui->initialStateComboBox);
    
    bool disableInputFlag = dynamic_cast<UIStaticTextMetadata*>(activeMetadata);
    ui->inputCheckBox->setEnabled(!disableInputFlag);
}

void FlagsPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterCheckBoxWidget(ui->visibleCheckBox);
    UnregisterCheckBoxWidget(ui->inputCheckBox);
    UnregisterCheckBoxWidget(ui->clipContentsCheckbox);

	UnregisterComboBoxWidget(ui->initialStateComboBox);
}

void FlagsPropertyGridWidget::FillComboboxes()
{
    WidgetSignalsBlocker initialStateBlocker(ui->initialStateComboBox);

	ui->initialStateComboBox->clear();

    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState controlState = UIControlStateHelper::GetUIControlState(i);
		QString stateName = UIControlStateHelper::GetUIControlStateName(controlState);
        ui->initialStateComboBox->addItem(stateName, QVariant(controlState));
    }
}

void FlagsPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget,
														  const PROPERTYGRIDWIDGETSITER& iter,
														  const QString& value)
{
	if (senderWidget == NULL)
    {
        Logger::Error("FlagsPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }

    if (senderWidget == ui->initialStateComboBox)
    {
	    int selectedIndex = senderWidget->currentIndex();
		int selectedValue = (int)UIControlStateHelper::GetUIControlState(selectedIndex);
		int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata,
																   iter->second.getProperty().name());
		// Don't update the property if the text wasn't actually changed.
		if (curValue == selectedValue)
		{
			return;
		}

		BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, selectedValue);
		CommandsController::Instance()->ExecuteCommand(command);
		SafeRelease(command);

		return;
    }
	
	// No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void FlagsPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }
	
    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

	// Is it our combobox?
    if (comboBoxWidget == ui->initialStateComboBox)
    {
        return SetComboboxSelectedItem(comboBoxWidget,
									   UIControlStateHelper::GetUIControlStateName((UIControl::eControlState)propertyValue));
    }
	
    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

