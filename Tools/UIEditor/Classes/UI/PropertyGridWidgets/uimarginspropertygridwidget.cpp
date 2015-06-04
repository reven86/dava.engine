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


#include "uimarginspropertygridwidget.h"
#include "ui_uimarginspropertygridwidget.h"

#include "PropertyNames.h"
#include "WidgetSignalsBlocker.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"

UIMarginsPropertyGridWidget::UIMarginsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::UIMarginsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName("");
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);

}

void UIMarginsPropertyGridWidget::SetPropertyPrefix(const String& prefix)
{
    propertyPrefix = prefix;
}

UIMarginsPropertyGridWidget::~UIMarginsPropertyGridWidget()
{
    delete ui;
}

void UIMarginsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    RegisterGridWidgetAsStateAware();

    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, GetPrefixedPropertyName(PropertyNames::LEFT_MARGIN_PROPERTY_NAME).c_str(), ui->leftDoubleSpinBox, false, true);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, GetPrefixedPropertyName(PropertyNames::TOP_MARGIN_PROPERTY_NAME).c_str(), ui->topDoubleSpinBox, false, true);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, GetPrefixedPropertyName(PropertyNames::RIGHT_MARGIN_PROPERTY_NAME).c_str(), ui->rightDoubleSpinBox, false, true);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, GetPrefixedPropertyName(PropertyNames::BOTTOM_MARGIN_PROPERTY_NAME).c_str(), ui->bottomDoubleSpinBox, false, true);

    connect(ui->resetMarginsButton, SIGNAL(clicked()), this, SLOT(OnResetUIMarginsClicked()));
//  UpdateUI();
}

void UIMarginsPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();

    disconnect(ui->resetMarginsButton, SIGNAL(clicked()), this, SLOT(OnResetUIMarginsClicked()));
  
    UnregisterDoubleSpinBoxWidget(ui->leftDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->topDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->rightDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->bottomDoubleSpinBox);
    
    BasePropertyGridWidget::Cleanup();
}

String UIMarginsPropertyGridWidget::GetPrefixedPropertyName(const char* propertyName) const
{
    if (propertyPrefix.empty())
    {
        return propertyName;
    }
    
    return Format("%s%s", propertyPrefix.c_str(), propertyName);
}

void UIMarginsPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *spinBoxWidget,
                                                                             const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }
    
	double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                                          curProperty.name());
    WidgetSignalsBlocker blocker(spinBoxWidget);
    spinBoxWidget->setValue(propertyValue);

    UpdateWidgetPalette(spinBoxWidget, curProperty.name());
    UpdateUI();
}

void UIMarginsPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox* /*doubleSpinBox*/,
                                                                   const PROPERTYGRIDWIDGETSITER &iter,
                                                                   const double value)
{
    double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
    if (curValue == value)
    {
        return;
    }

    BaseCommand* command = new ChangeDoublePropertyCommand(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void UIMarginsPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    UpdateUI();
}

void UIMarginsPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);
    UpdateUI();
}

void UIMarginsPropertyGridWidget::UpdateUI()
{
    if (!activeMetadata)
    {
        return;
    }

    UpdateMaxMarginValues();
    UpdateResetMarginsButtonVisible();
}

void UIMarginsPropertyGridWidget::UpdateMaxMarginValues()
{
    float sizeX = PropertiesHelper::GetAllPropertyValues<float>(activeMetadata, PropertyNames::SIZE_X);
    ui->rightDoubleSpinBox->setMaximum(sizeX - ui->leftDoubleSpinBox->value());
    ui->leftDoubleSpinBox->setMaximum(sizeX - ui->rightDoubleSpinBox->value());

   float sizeY = PropertiesHelper::GetAllPropertyValues<float>(activeMetadata, PropertyNames::SIZE_Y);
    ui->bottomDoubleSpinBox->setMaximum(sizeY - ui->topDoubleSpinBox->value());
    ui->topDoubleSpinBox->setMaximum(sizeY - ui->bottomDoubleSpinBox->value());
}

void UIMarginsPropertyGridWidget::UpdateResetMarginsButtonVisible()
{
    bool needShowButton = !(FLOAT_EQUAL(ui->leftDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->topDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->rightDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->bottomDoubleSpinBox->value(), 0.0f));
    ui->resetMarginsButton->setVisible(needShowButton);
}

void UIMarginsPropertyGridWidget::OnResetUIMarginsClicked()
{
    if (!activeMetadata)
    {
        return;
    }
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	PROPERTIESMAPITER uiMarginsIter = propertiesMap.find(GetPrefixedPropertyName(PropertyNames::MARGINS_PROPERTY_NAME));
	DVASSERT(uiMarginsIter != propertiesMap.end());

    BaseCommand* command = new ChangePropertyCommand<QRectF>(activeMetadata, PropertyGridWidgetData(uiMarginsIter->second, false, false), QRectF());
	CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);

	CommandsController::Instance()->EmitUpdatePropertyValues();
}
