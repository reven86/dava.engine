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


#include "alignspropertygridwidget.h"
#include "ui_alignspropertygridwidget.h"

#include "PropertyNames.h"
#include "ChangeAlignPropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "PropertiesGridController.h"

static const QString ALIGN_PROPERTY_BLOCK_NAME = "Align options";

AlignsPropertyGridWidget::AlignsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::AlignsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(ALIGN_PROPERTY_BLOCK_NAME);
	
	// Create align checkbox-spinbox map
	// This map should help to manage checkbox/spinbox releations
	alignWidgetsMap = ALIGNWIDGETSMAP();
	alignWidgetsMap.insert(std::make_pair(ui->leftAlignCheckBox, ui->leftAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->hcenterAlignCheckBox, ui->hcenterAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->rightAlignCheckBox, ui->rigthAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->topAlignCheckBox, ui->topAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->vcenterAlignCheckBox, ui->vcenterAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->bottomAlignCheckBox, ui->bottomAlignSpinBox));

	// This event filter will prevent mouse wheel event when hovered control is not yet selected
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

AlignsPropertyGridWidget::~AlignsPropertyGridWidget()
{
    delete ui;
}

void AlignsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::LEFT_ALIGN_ENABLED, ui->leftAlignCheckBox);
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::HCENTER_ALIGN_ENABLED, ui->hcenterAlignCheckBox);
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::RIGHT_ALIGN_ENABLED, ui->rightAlignCheckBox);
	
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::TOP_ALIGN_ENABLED, ui->topAlignCheckBox);
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::VCENTER_ALIGN_ENABLED, ui->vcenterAlignCheckBox);
	RegisterAlignCheckBoxWidgetForProperty(propertiesMap, PropertyNames::BOTTOM_ALIGN_ENABLED, ui->bottomAlignCheckBox);

    // Register the widgets.
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::LEFT_ALIGN, ui->leftAlignSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::HCENTER_ALIGN, ui->hcenterAlignSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::RIGHT_ALIGN, ui->rigthAlignSpinBox);
	
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::TOP_ALIGN, ui->topAlignSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::VCENTER_ALIGN, ui->vcenterAlignSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::BOTTOM_ALIGN, ui->bottomAlignSpinBox);
}

void AlignsPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
	
	UnregisterAlignCheckBoxWidget(ui->leftAlignCheckBox);
	UnregisterAlignCheckBoxWidget(ui->hcenterAlignCheckBox);
	UnregisterAlignCheckBoxWidget(ui->rightAlignCheckBox);
	UnregisterAlignCheckBoxWidget(ui->topAlignCheckBox);
	UnregisterAlignCheckBoxWidget(ui->vcenterAlignCheckBox);
	UnregisterAlignCheckBoxWidget(ui->bottomAlignCheckBox);	
	
	UnregisterSpinBoxWidget(ui->leftAlignSpinBox);
	UnregisterSpinBoxWidget(ui->hcenterAlignSpinBox);
	UnregisterSpinBoxWidget(ui->rigthAlignSpinBox);
	UnregisterSpinBoxWidget(ui->topAlignSpinBox);
	UnregisterSpinBoxWidget(ui->vcenterAlignSpinBox);
	UnregisterSpinBoxWidget(ui->bottomAlignSpinBox);
}

void AlignsPropertyGridWidget::RegisterAlignCheckBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap,
															   const char* propertyName,
                                                               QCheckBox* checkBoxWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(checkBoxWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));

    UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);

    // Register the signal for this widget.
    connect(checkBoxWidget, SIGNAL(stateChanged(int)), this, SLOT(OnAlignCheckBoxStateChanged(int)));
}

void AlignsPropertyGridWidget::UnregisterAlignCheckBoxWidget(QCheckBox* checkBoxWidget)
{
    disconnect(checkBoxWidget, SIGNAL(stateChanged(int)), this, SLOT(OnAlignCheckBoxStateChanged(int)));
}

void AlignsPropertyGridWidget::OnAlignCheckBoxStateChanged(int state)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QCheckBox* senderWidget = dynamic_cast<QCheckBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnCheckBoxStateChanged - sender is NULL!");
        return;
    }

    // After the first change don't allow the checkbox to be tristate.
    senderWidget->setTristate(false);
    
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnCheckBoxStateChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

	// Don't update the property if the text wasn't actually changed.
	bool isPropertyValueDiffers = false;
    bool curValue = PropertiesHelper::GetAllPropertyValues<bool>(this->activeMetadata, iter->second.getProperty().name(),
																 isPropertyValueDiffers);

	// In case we have differences in property values for different widgets - force update the states too.
	bool newValue = (state == Qt::Checked);
	if ((curValue == newValue) && !isPropertyValueDiffers)
	{
		return;
	}

    BaseCommand* command = new ChangeAlignPropertyCommand(activeMetadata, iter->second, newValue);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void AlignsPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget,
                                                                   const QMetaProperty& curProperty)
{
	BasePropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);

	UpdateCheckBoxSates();
	UpdateSpinBoxState(checkBoxWidget);
}

void AlignsPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
	// Each time aligns properties are changes we should notify all listeners
	// (first of all in rect widget) to force recalculation of related properties
	// In other words if we changed align option we should force change of related
	// property in Rect Property Grid widget
	CommandsController::Instance()->EmitUpdatePropertyValues();
}

void AlignsPropertyGridWidget::UpdateCheckBoxSates()
{
	// Horizontal align
	// Only two horizontal align properties can be set per time
	// We have to disable third checkbox if two other checkboxes already selected
	bool disableCheckBox = ui->hcenterAlignCheckBox->isChecked() && ui->rightAlignCheckBox->isChecked();
    ui->leftAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->leftAlignCheckBox->isChecked() && ui->rightAlignCheckBox->isChecked();
    ui->hcenterAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->leftAlignCheckBox->isChecked() && ui->hcenterAlignCheckBox->isChecked();
    ui->rightAlignCheckBox->setDisabled(disableCheckBox);
	
	// Vertical align
    disableCheckBox = ui->vcenterAlignCheckBox->isChecked() && ui->bottomAlignCheckBox->isChecked();
    ui->topAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->topAlignCheckBox->isChecked() && ui->bottomAlignCheckBox->isChecked();
    ui->vcenterAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->topAlignCheckBox->isChecked() && ui->vcenterAlignCheckBox->isChecked();
    ui->bottomAlignCheckBox->setDisabled(disableCheckBox);
}

void AlignsPropertyGridWidget::UpdateSpinBoxState(QCheckBox *buddyWidget)
{
	WidgetSignalsBlocker blocker(buddyWidget);
	// Get current state of checkbox
	bool state = buddyWidget->isChecked();
	
	// Update appropriate spinbox state
	ALIGNWIDGETSMAPITER iter = this->alignWidgetsMap.find(buddyWidget);
    if (iter != this->alignWidgetsMap.end())
    {
     	QSpinBox *spinBox = iter->second;
		// Change spinbox state according to checkbox state
		spinBox->setEnabled(state);
    }
}
