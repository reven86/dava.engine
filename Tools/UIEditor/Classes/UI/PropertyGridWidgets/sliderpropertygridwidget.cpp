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


#include "sliderpropertygridwidget.h"
#include "ui_sliderpropertygridwidget.h"
#include "backgroundpropertygridwidget.h"

#include "PropertyNames.h"
#include "BackgroundGridWidgetHelper.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "TexturePacker/ResourcePacker2D.h"

#include <QFileDialog>

static const QString SLIDER_PROPERTY_BLOCK_NAME = "Slider options";

SliderPropertyGridWidget::SliderPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::SliderPropertyGridWidget)
{
    ui->setupUi(this);

    minBgControl = new BackgroundPropertyGridWidget("Min Background", "Min");
    maxBgControl = new BackgroundPropertyGridWidget("Max Background", "Max");

    ui->groupBox->layout()->addWidget(minBgControl);
    ui->groupBox->layout()->addWidget(maxBgControl);

    SetPropertyBlockName(SLIDER_PROPERTY_BLOCK_NAME);
	ConnectToSignals();
	
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

SliderPropertyGridWidget::~SliderPropertyGridWidget()
{
    disconnect(ui->valuePosition, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    delete ui;
}

void SliderPropertyGridWidget::ConnectToSignals()
{
    // Create connection between slider and double spinbox
    connect(ui->valuePosition, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
}

void SliderPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    minBgControl->Initialize(activeMetadata);
    maxBgControl->Initialize(activeMetadata);

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SLIDER_VALUE_PROPERTY_NAME, ui->valueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SLIDER_MIN_VALUE_PROPERTY_NAME, ui->minValueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SLIDER_MAX_VALUE_PROPERTY_NAME, ui->maxValueSpin);
}

void SliderPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    minBgControl->Cleanup();
    maxBgControl->Cleanup();

	UnregisterDoubleSpinBoxWidget(ui->valueSpin);
    UnregisterDoubleSpinBoxWidget(ui->minValueSpin);
    UnregisterDoubleSpinBoxWidget(ui->maxValueSpin);
}

void SliderPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget,
														const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (doubleSpinBoxWidget == NULL)
    {
        Logger::Error("SliderPropertyGridWidget::ProcessDoubleSpinBoxValueChanged: senderWidget is NULL!");
        return;
    }

    // Try to process this control-specific widgets.	
	if ((doubleSpinBoxWidget == ui->valueSpin) ||
		(doubleSpinBoxWidget == ui->maxValueSpin) || (doubleSpinBoxWidget == ui->minValueSpin))
	{
		// Don't update the property if the text wasn't actually changed.
		double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
		if (curValue == value)
		{
			return;
		}

		BaseCommand* command = new ChangeDoublePropertyCommand(activeMetadata, iter->second, value);
  	  	CommandsController::Instance()->ExecuteCommand(command);
   		SafeRelease(command);
		return;
	}

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessDoubleSpinBoxValueChanged(doubleSpinBoxWidget, iter, value);
}

void SliderPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
																			const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

	double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                 					     curProperty.name());

    WidgetSignalsBlocker blocker(doubleSpinBoxWidget);
    doubleSpinBoxWidget->setValue(propertyValue);
	UpdateWidgetPalette(doubleSpinBoxWidget, curProperty.name());
	// Update related properties for concrete spin box
	if (doubleSpinBoxWidget == ui->valueSpin)
	{
		//Upate value of slider
		ui->valuePosition->setValue((int)propertyValue);		
		return;
    }
	else if (doubleSpinBoxWidget == ui->maxValueSpin) 
	{
		//Update max value
		ui->valuePosition->setMaximum((int)propertyValue);
		ui->valueSpin->setMaximum(propertyValue);
		return;
	}
	else if (doubleSpinBoxWidget == ui->minValueSpin)
	{
		//Update min value
		ui->valuePosition->setMinimum((int)propertyValue);
		ui->valueSpin->setMinimum(propertyValue);
		return;
	}
	
    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, curProperty);
}

void SliderPropertyGridWidget::OnSliderValueChanged(int value)
{
	ui->valueSpin->setValue(value);
}
