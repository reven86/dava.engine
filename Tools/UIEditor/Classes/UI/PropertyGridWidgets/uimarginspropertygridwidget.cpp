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
