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


#include "scrollviewpropertygridwidget.h"
#include "ui_scrollviewpropertygridwidget.h"

#include "PropertyNames.h"
#include "BackgroundGridWidgetHelper.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "PropertyNames.h"
//#include "ResourcePacker.h"

#include <QFileDialog>

static const QString SCROLL_VIEW_PROPERTY_BLOCK_NAME = "Scroll area options";

ScrollViewPropertyGridWidget::ScrollViewPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ScrollViewPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(SCROLL_VIEW_PROPERTY_BLOCK_NAME);
    ConnectToSignals();

    BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

ScrollViewPropertyGridWidget::~ScrollViewPropertyGridWidget()
{
    delete ui;
}

void ScrollViewPropertyGridWidget::ConnectToSignals()
{
    // Create connection between slider and double spinbox
    connect(ui->scrollHSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    connect(ui->scrollVSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
}

void ScrollViewPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::HORIZONTAL_SCROLL_POSITION, ui->scrollHPositionSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::VERTICAL_SCROLL_POSITION, ui->scrollVPositionSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SCROLL_CONTENT_SIZE_X, ui->contentSizeXSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SCROLL_CONTENT_SIZE_Y, ui->contentSizeYSpinBox);
	
	UpdateMaximumValue();
}

void ScrollViewPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    UnregisterDoubleSpinBoxWidget(ui->scrollHPositionSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->scrollVPositionSpinBox);
	UnregisterSpinBoxWidget(ui->contentSizeXSpinBox);
	UnregisterSpinBoxWidget(ui->contentSizeYSpinBox);
}

void ScrollViewPropertyGridWidget::UpdateMaximumValue()
{
	int sizeX = BasePropertyGridWidget::GetPropertyIntValue(PropertyNames::SIZE_X);
	int contentSizeX = ui->contentSizeXSpinBox->value();
	int maxX = contentSizeX - sizeX;
	if (maxX > 0)
	{
		ui->scrollHPositionSpinBox->setMaximum(maxX);
		ui->scrollHSlider->setMaximum(maxX);
	}	
	
	int sizeY = BasePropertyGridWidget::GetPropertyIntValue(PropertyNames::SIZE_Y);
	int contentSizeY = ui->contentSizeYSpinBox->value();
	int maxY = contentSizeY - sizeY;
	if (maxY > 0)
	{
		ui->scrollVPositionSpinBox->setMaximum(maxY);
		ui->scrollVSlider->setMaximum(maxY);
	}
}

void ScrollViewPropertyGridWidget::OnPropertiesChangedFromExternalSource()
{
    BasePropertyGridWidget::OnPropertiesChangedFromExternalSource();
	UpdateMaximumValue();
}

void ScrollViewPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
	//If one of the align option state is changed we should check it and disable/enable appropriate Relative postion or size spinbox(es)
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
	
	if ((propertyName == PropertyNames::SIZE_X) || (propertyName == PropertyNames::SIZE_Y))
	{
		UpdateMaximumValue();
	}
}

void ScrollViewPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget,
                                                        const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (doubleSpinBoxWidget == NULL)
    {
        Logger::Error("ScrollViewPropertyGridWidget::ProcessDoubleSpinBoxValueChanged: senderWidget is NULL!");
        return;
    }

    // Try to process this control-specific widgets.
    if (doubleSpinBoxWidget == ui->scrollHPositionSpinBox || doubleSpinBoxWidget == ui->scrollVPositionSpinBox)
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

void ScrollViewPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
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
    if (doubleSpinBoxWidget == ui->scrollHPositionSpinBox)
    {
         //Upate value of slider
        ui->scrollHSlider->setValue((int)propertyValue);
        return;
    }
    else if (doubleSpinBoxWidget == ui->scrollVPositionSpinBox)
    {
        //Upate value of slider
        ui->scrollVSlider->setValue((int)propertyValue);
        return;
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, curProperty);
}

void ScrollViewPropertyGridWidget::OnSliderValueChanged(int value)
{
    QWidget* senderWidget = (QWidget*) QObject::sender();

    if (senderWidget == ui->scrollHSlider)
    {
        ui->scrollHPositionSpinBox->setValue(value);
    }
    else if (senderWidget == ui->scrollVSlider)
    {
        ui->scrollVPositionSpinBox->setValue(value);
    }
}
