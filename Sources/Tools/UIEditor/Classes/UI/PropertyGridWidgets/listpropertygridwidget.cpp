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


#include "listpropertygridwidget.h"
#include "ui_listpropertygridwidget.h"

#include "ListPropertyGridWidgetHelper.h"
#include "UIListMetadata.h"

#include "WidgetSignalsBlocker.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"

static const QString LIST_PROPERTY_BLOCK_NAME = "List";

ListPropertyGridWidget::ListPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ListPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(LIST_PROPERTY_BLOCK_NAME);
}

ListPropertyGridWidget::~ListPropertyGridWidget()
{
    delete ui;
}

void ListPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);	
	FillComboboxes();
	
	// Build the properties map to make the properties search faster.
	PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	
	RegisterComboBoxWidgetForProperty(propertiesMap, "AggregatorID", ui->aggregatorsComboBox, false, true);
	RegisterComboBoxWidgetForProperty(propertiesMap, "Orientation", ui->orientationComboBox, false, true);
}

void ListPropertyGridWidget::Cleanup()
{
	BasePropertyGridWidget::Cleanup();

	UnregisterComboBoxWidget(ui->aggregatorsComboBox);
	UnregisterComboBoxWidget(ui->orientationComboBox);
}

void ListPropertyGridWidget::FillComboboxes()
{
    WidgetSignalsBlocker aggregatorsBlocker(ui->aggregatorsComboBox);
	WidgetSignalsBlocker orientationBlocked(ui->orientationComboBox);
	// Reset lists
	ui->aggregatorsComboBox->clear();
	nodeIDList.clear();

	// Add empty aggregator and set its id as "0"
	ui->aggregatorsComboBox->addItem(QString("none"));
	nodeIDList.push_back(0);
	
	// Put into combobox available aggregators names
	const HierarchyTreePlatformNode* platform = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platform)
	{
		const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = platform->GetChildNodes();
		for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
			 iter != items.end();
			 ++iter)
		{
			HierarchyTreeNode* node = (*iter);
			// Get aggregator node id and put it into internal list
			HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
			if (aggregatorNode)
			{
				ui->aggregatorsComboBox->addItem(aggregatorNode->GetName());
				nodeIDList.push_back(aggregatorNode->GetId());
			}
		}
	}
	
	ui->orientationComboBox->clear();
    int itemsCount = ListPropertyGridWidgetHelper::GetOrientationCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->orientationComboBox->addItem(ListPropertyGridWidgetHelper::GetOrientationDesc(i));
    }
}

void ListPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("BackGroundPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
    if (senderWidget == ui->aggregatorsComboBox)
    {
		CustomProcessComboboxValueChanged(iter, nodeIDList.at(selectedIndex));
		return;
    }
	else if (senderWidget == ui->orientationComboBox)
	{
		CustomProcessComboboxValueChanged(iter, ListPropertyGridWidgetHelper::GetOrientation(selectedIndex));
		return;
	}

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void ListPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
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

void ListPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->aggregatorsComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
		int index = nodeIDList.indexOf(propertyValue);
		
        SetComboboxSelectedItem(comboBoxWidget, ui->aggregatorsComboBox->itemText(index));
		return;
    }
	else if (comboBoxWidget == ui->orientationComboBox)
	{
	    UpdateWidgetPalette(comboBoxWidget, propertyName);
		
        SetComboboxSelectedItem(comboBoxWidget,  ListPropertyGridWidgetHelper::GetOrientationDescByType(
																		(UIList::eListOrientation)propertyValue));
		return;
	}

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}