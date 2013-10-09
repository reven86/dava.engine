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

#include "scrollcontrolpropertygridwidget.h"
#include "ui_scrollcontrolpropertygridwidget.h"

#include "ScrollPropertyGridWidgetHelper.h"
#include "UIScrollBarMetadata.h"

#include "WidgetSignalsBlocker.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"

static const QString SCROLL_PROPERTY_BLOCK_NAME = "Scroll";

ScrollControlPropertyGridWidget::ScrollControlPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ScrollControlPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(SCROLL_PROPERTY_BLOCK_NAME);
}

ScrollControlPropertyGridWidget::~ScrollControlPropertyGridWidget()
{
    delete ui;
}

void ScrollControlPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);	
	FillComboboxes();
	
	// Build the properties map to make the properties search faster.
	PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

	RegisterComboBoxWidgetForProperty(propertiesMap, "ScrollOrientation", ui->orientationComboBox, false, true);
}

void ScrollControlPropertyGridWidget::Cleanup()
{	
	UnregisterComboBoxWidget(ui->orientationComboBox);
	
	BasePropertyGridWidget::Cleanup();
}

void ScrollControlPropertyGridWidget::FillComboboxes()
{
	WidgetSignalsBlocker orientationBlocked(ui->orientationComboBox);
	
	ui->orientationComboBox->clear();
    int itemsCount = ScrollPropertyGridWidgetHelper::GetOrientationCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->orientationComboBox->addItem(ScrollPropertyGridWidgetHelper::GetOrientationDesc(i));
    }
}

void ScrollControlPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("ScrollControlPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
	if (senderWidget == ui->orientationComboBox)
	{
		CustomProcessComboboxValueChanged(iter, ScrollPropertyGridWidgetHelper::GetOrientation(senderWidget->currentIndex()));
		return;
	}

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void ScrollControlPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
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

void ScrollControlPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
	if (comboBoxWidget == ui->orientationComboBox)
	{
	    UpdateWidgetPalette(comboBoxWidget, propertyName);
		
        SetComboboxSelectedItem(comboBoxWidget, ScrollPropertyGridWidgetHelper::GetOrientationDescByType(
																		(UIScrollBar::eScrollOrientation)propertyValue));
		return;
	}

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}