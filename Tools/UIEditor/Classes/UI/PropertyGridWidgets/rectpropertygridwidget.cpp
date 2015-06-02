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


#include "rectpropertygridwidget.h"
#include "ui_rectpropertygridwidget.h"
#include "PropertiesGridController.h"
#include "PropertyNames.h"

#include "CommandsController.h"
#include "CenterPivotPointCommand.h"

#include "WidgetSignalsBlocker.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Rect";

RectPropertyGridWidget::RectPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::RectPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
	// This event filter will prevent mouse wheel event when hovered control is not yet selected
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

RectPropertyGridWidget::~RectPropertyGridWidget()
{
    delete ui;
}

void RectPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "RelativeX", ui->relativeXDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "RelativeY", ui->relativeYDoubleSpinBox);

    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "AbsoluteX", ui->absoluteXDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "AbsoluteY", ui->absoluteYDoubleSpinBox);

    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "SizeX", ui->sizeXDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "SizeY", ui->sizeYDoubleSpinBox);
    
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "PivotX", ui->pivotXDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, "PivotY", ui->pivotYDoubleSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "Angle", ui->angleSpinBox);

    connect(ui->centerPivotPointButton, SIGNAL(clicked()), this, SLOT(OnCenterPivotPointButtonClicked()));

	UpdateHorizontalWidgetsState();
	UpdateVerticalWidgetsState();
}

void RectPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    disconnect(ui->centerPivotPointButton, SIGNAL(clicked()), this, SLOT(OnCenterPivotPointButtonClicked()));

    UnregisterDoubleSpinBoxWidget(ui->relativeXDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->relativeYDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->absoluteXDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->absoluteYDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->sizeXDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->sizeYDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->pivotXDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->pivotYDoubleSpinBox);
    UnregisterSpinBoxWidget(ui->angleSpinBox);
}

void RectPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
	//If one of the align option state is changed we should check it and disable/enable appropriate Relative postion or size spinbox(es)
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
	
	if (propertyName == PropertyNames::LEFT_ALIGN_ENABLED ||
		propertyName == PropertyNames::RIGHT_ALIGN_ENABLED ||
		propertyName == PropertyNames::HCENTER_ALIGN_ENABLED)
	{
		UpdateHorizontalWidgetsState();
	}

	if (propertyName == PropertyNames::TOP_ALIGN_ENABLED ||
		propertyName == PropertyNames::BOTTOM_ALIGN_ENABLED ||
		propertyName == PropertyNames::VCENTER_ALIGN_ENABLED)
	{
		UpdateVerticalWidgetsState();
	}
}

void RectPropertyGridWidget::UpdateHorizontalWidgetsState()
{
	// Get horizontal align properties values
	bool leftAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::LEFT_ALIGN_ENABLED);
	bool hcenterAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::HCENTER_ALIGN_ENABLED);
	bool rightAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::RIGHT_ALIGN_ENABLED);
		
	// Change relative X position spinbox state according to align properties
	bool disableRelativeX = leftAlignEnabled || hcenterAlignEnabled || rightAlignEnabled;
	ui->relativeXDoubleSpinBox->setDisabled(disableRelativeX);
	ui->absoluteXDoubleSpinBox->setDisabled(disableRelativeX);
		
	// Change size X spinbox state according to align properties
	bool disableSizeX = IsTwoAlignsEnabled(leftAlignEnabled, hcenterAlignEnabled, rightAlignEnabled);
	ui->sizeXDoubleSpinBox->setDisabled(disableSizeX);
}

void RectPropertyGridWidget::UpdateVerticalWidgetsState()
{
	// Get vertical align properties values
	bool topAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::TOP_ALIGN_ENABLED);
	bool vcenterAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::VCENTER_ALIGN_ENABLED);
	bool bottomAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::BOTTOM_ALIGN_ENABLED);
		
	// Change relative Y position spinbox state according to align properties
	bool disableRelativeY = topAlignEnabled || vcenterAlignEnabled || bottomAlignEnabled;
	ui->relativeYDoubleSpinBox->setDisabled(disableRelativeY);
	ui->absoluteYDoubleSpinBox->setDisabled(disableRelativeY);
		
	// Change size Y spinbox state according to align properties
	bool disableSizeY = IsTwoAlignsEnabled(topAlignEnabled, vcenterAlignEnabled, bottomAlignEnabled);
	ui->sizeYDoubleSpinBox->setDisabled(disableSizeY);
}

bool RectPropertyGridWidget::IsTwoAlignsEnabled(bool first, bool center, bool second)
{
	return ((first && center) || (center && second) || (first && second));
}

void RectPropertyGridWidget::OnCenterPivotPointButtonClicked()
{
	PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	PROPERTIESMAPITER pivotPointPropertyIter = propertiesMap.find("Pivot");
	if (pivotPointPropertyIter == propertiesMap.end())
	{
		DVASSERT(false);
		return;
	}

	BaseCommand* command = new CenterPivotPointCommand(activeMetadata, pivotPointPropertyIter->second);
	CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);

	CommandsController::Instance()->EmitUpdatePropertyValues();
}

void RectPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *spinBox, const QMetaProperty& curProperty)
{
    double value = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                                  curProperty.name());
    {
        WidgetSignalsBlocker blocker(spinBox);
        AdjustablePointDoubleSpinBox* customSpinBox = dynamic_cast<AdjustablePointDoubleSpinBox*>(spinBox);
        if (customSpinBox)
        {
            customSpinBox->SetValueAndAdjustPoint(value);
        }
        else
        {
            spinBox->setValue(value);
        }
    }

    UpdateWidgetPalette(spinBox, curProperty.name());
}

void RectPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox* /*doubleSpinBox*/, const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (activeMetadata == NULL)
    {
        return;
    }

    double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}

    BaseCommand* command = new ChangeDoublePropertyCommand(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}
