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
#include "PropertyNames.h"

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

	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::SCROLL_ORIENTATION, ui->orientationComboBox, false, true);
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::SCROLL_BAR_DELEGATE_NAME, ui->scrollViewsComboBox, false, true);
}

void ScrollControlPropertyGridWidget::Cleanup()
{	
	UnregisterComboBoxWidget(ui->orientationComboBox);
    UnregisterComboBoxWidget(ui->scrollViewsComboBox);
	
	BasePropertyGridWidget::Cleanup();
}

void ScrollControlPropertyGridWidget::FillComboboxes()
{
	WidgetSignalsBlocker orientationBlocked(ui->orientationComboBox);
    WidgetSignalsBlocker scrollViewsBlocked(ui->scrollViewsComboBox);
	
	ui->orientationComboBox->clear();
    ui->scrollViewsComboBox->clear();
    int itemsCount = ScrollPropertyGridWidgetHelper::GetOrientationCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->orientationComboBox->addItem(ScrollPropertyGridWidgetHelper::GetOrientationDesc(i));
    }
    ui->scrollViewsComboBox->addItem(QString(""));
    ui->scrollViewsComboBox->setItemData(0, "None",Qt::ToolTipRole);
    FillScrollViewsComboBox(HierarchyTreeController::Instance()->GetActiveScreen()->GetChildNodes());

    
}

void ScrollControlPropertyGridWidget::FillScrollViewsComboBox(const HierarchyTreeNode::HIERARCHYTREENODESLIST nodes)
{
    HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it=nodes.begin();
    for (; it!=nodes.end(); ++it)
    {
        const HierarchyTreeControlNode * controlNode = static_cast<HierarchyTreeControlNode *>(*it);
        UIControl* control = controlNode->GetUIObject();
        UIScrollBarDelegate* scroll = dynamic_cast<UIScrollBarDelegate*>(control);
        if (NULL != scroll)
        {
            QString delegatePath = QString::fromStdString(UIControlHelpers::GetControlPath(control));
            ui->scrollViewsComboBox->addItem(QString::fromStdString(control->GetName()));
            uint32 itemCount = ui->scrollViewsComboBox->count();
            ui->scrollViewsComboBox->setItemData(itemCount-1, delegatePath, Qt::ToolTipRole);
        }
        FillScrollViewsComboBox((*it)->GetChildNodes());
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
		CustomProcessOrientationValueChanged(iter, ScrollPropertyGridWidgetHelper::GetOrientation(senderWidget->currentIndex()));
		return;
	}
    if (senderWidget == ui->scrollViewsComboBox)
    {
        int32 index = ui->scrollViewsComboBox->findText(value);
        QVariant controlPath = ui->scrollViewsComboBox->itemData(index,Qt::ToolTipRole);
        CustomProcessScrollViewValueChanged(iter, controlPath.toString());
        return;
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void ScrollControlPropertyGridWidget::CustomProcessOrientationValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
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
void ScrollControlPropertyGridWidget::CustomProcessScrollViewValueChanged(const PROPERTYGRIDWIDGETSITER& iter, QString value)
{
    QString curValue = PropertiesHelper::GetAllPropertyValues<QString>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}
    
    BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, value);
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
    

    // Firstly check the custom comboboxes.
	if (comboBoxWidget == ui->orientationComboBox)
	{
        int propertyValueOrientation = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);
	    UpdateWidgetPalette(comboBoxWidget, propertyName);
		
        SetComboboxSelectedItem(comboBoxWidget, ScrollPropertyGridWidgetHelper::GetOrientationDescByType(
																		(UIScrollBar::eScrollOrientation)propertyValueOrientation));
		return;
	}

    if (comboBoxWidget == ui->scrollViewsComboBox)
	{
        QString propertyValueName = PropertiesHelper::GetPropertyValue<QString>(this->activeMetadata, propertyName, isPropertyValueDiffers);
	    UpdateWidgetPalette(comboBoxWidget, propertyName);
		
        SetComboboxSelectedItem(comboBoxWidget, propertyValueName, true);
		return;
	}
    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}