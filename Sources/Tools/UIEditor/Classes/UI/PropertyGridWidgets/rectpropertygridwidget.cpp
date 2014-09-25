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
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeX", ui->relativeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeY", ui->relativeYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteX", ui->absoluteXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteY", ui->absoluteYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeX", ui->sizeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeY", ui->sizeYSpinBox);
    
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotX", ui->pivotXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotY", ui->pivotYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "Angle", ui->angleSpinBox);

    connect(ui->centerPivotPointButton, SIGNAL(clicked()), this, SLOT(OnCenterPivotPointButtonClicked()));

	UpdateHorizontalWidgetsState();
	UpdateVerticalWidgetsState();
}

void RectPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    disconnect(ui->centerPivotPointButton, SIGNAL(clicked()), this, SLOT(OnCenterPivotPointButtonClicked()));

    UnregisterSpinBoxWidget(ui->relativeXSpinBox);
    UnregisterSpinBoxWidget(ui->relativeYSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteXSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteYSpinBox);
    UnregisterSpinBoxWidget(ui->sizeXSpinBox);
    UnregisterSpinBoxWidget(ui->sizeYSpinBox);
    UnregisterSpinBoxWidget(ui->pivotXSpinBox);
    UnregisterSpinBoxWidget(ui->pivotYSpinBox);
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
	ui->relativeXSpinBox->setDisabled(disableRelativeX);
	ui->absoluteXSpinBox->setDisabled(disableRelativeX);
		
	// Change size X spinbox state according to align properties
	bool disableSizeX = IsTwoAlignsEnabled(leftAlignEnabled, hcenterAlignEnabled, rightAlignEnabled);
	ui->sizeXSpinBox->setDisabled(disableSizeX);
}

void RectPropertyGridWidget::UpdateVerticalWidgetsState()
{
	// Get vertical align properties values
	bool topAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::TOP_ALIGN_ENABLED);
	bool vcenterAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::VCENTER_ALIGN_ENABLED);
	bool bottomAlignEnabled = BasePropertyGridWidget::GetPropertyBooleanValue(PropertyNames::BOTTOM_ALIGN_ENABLED);
		
	// Change relative Y position spinbox state according to align properties
	bool disableRelativeY = topAlignEnabled || vcenterAlignEnabled || bottomAlignEnabled;
	ui->relativeYSpinBox->setDisabled(disableRelativeY);
	ui->absoluteYSpinBox->setDisabled(disableRelativeY);
		
	// Change size Y spinbox state according to align properties
	bool disableSizeY = IsTwoAlignsEnabled(topAlignEnabled, vcenterAlignEnabled, bottomAlignEnabled);
	ui->sizeYSpinBox->setDisabled(disableSizeY);
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
