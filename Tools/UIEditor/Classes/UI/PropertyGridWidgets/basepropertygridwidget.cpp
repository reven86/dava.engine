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


#include <QLayout>
#include <QColorDialog>
#include <QMessageBox>

#include "basepropertygridwidget.h"
#include "ui_basepropertygridwidget.h"

#include "PropertiesGridController.h"
#include "HierarchyTreeController.h"
#include "WidgetSignalsBlocker.h"
#include "SubcontrolsHelper.h"
#include "ResourcesManageHelper.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "InvokeMethodCommand.h"

const QString BasePropertyGridWidget::MULTIPLE_VALUE = "multiple";

BasePropertyGridWidget::BasePropertyGridWidget(QWidget *parent) :
    QWidget(parent),
    activeMetadata(NULL),
    ui(new Ui::BasePropertyGridWidget)
{
    PreparePalettes();
    ui->setupUi(this);
    //Register DAVA::Font to QT
    qRegisterMetaType<Font*>("Font");
}

BasePropertyGridWidget::~BasePropertyGridWidget()
{
    delete ui;
}

void BasePropertyGridWidget::SetPropertyBlockName(const QString& blockName)
{
    this->ui->blockNameLabel->setText(blockName);
}

void BasePropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    connect(CommandsController::Instance(), SIGNAL(UpdatePropertyValues()),
            this, SLOT(OnPropertiesChangedExternally()));
    connect(CommandsController::Instance(), SIGNAL(ChangePropertySucceeded(const QString&)),
            this, SLOT(OnChangePropertySucceeded(const QString&)));
    connect(CommandsController::Instance(), SIGNAL(ChangePropertyFailed(const QString&)),
            this, SLOT(OnChangePropertyFailed(const QString&)));
    
    this->activeMetadata = activeMetadata;
}

void BasePropertyGridWidget::Cleanup()
{
    disconnect(CommandsController::Instance(), SIGNAL(UpdatePropertyValues()),
            this, SLOT(OnPropertiesChangedExternally()));
    disconnect(CommandsController::Instance(), SIGNAL(ChangePropertySucceeded(const QString&)),
            this, SLOT(OnChangePropertySucceeded(const QString&)));
    disconnect(CommandsController::Instance(), SIGNAL(ChangePropertyFailed(const QString&)),
            this, SLOT(OnChangePropertyFailed(const QString&)));

    if (this->activeMetadata)
    {
        this->activeMetadata->CleanupParams();
        this->activeMetadata = NULL; // will be released by the owner.
    }
}

// Build the properties map in the "name - value" way to make the search faster.
BasePropertyGridWidget::PROPERTIESMAP BasePropertyGridWidget::BuildMetadataPropertiesMap()
{
    if (activeMetadata == NULL)
    {
        Logger::Error("Call to BuildMetadataPropertiesMap() without activeMetaData defined!");
        return BasePropertyGridWidget::PROPERTIESMAP();
    }
    
    BasePropertyGridWidget::PROPERTIESMAP propertiesMap;
    int32 count = activeMetadata->metaObject()->propertyCount();
    for (int32 i = 0; i < count; i ++)
    {
        const QMetaProperty& curProperty = activeMetadata->metaObject()->property(i);
        //Logger::FrameworkDebug("Property name %s", curProperty.name());
        propertiesMap.insert(std::make_pair(curProperty.name(), curProperty));
    }
    
    return propertiesMap;
}

// Register grid control as state-aware.
void BasePropertyGridWidget::RegisterGridWidgetAsStateAware()
{
	connect(PropertiesGridController::Instance(),
			SIGNAL(SelectedUIControlStatesChanged(Vector<UIControl::eControlState>)),
			this,
			SLOT(OnSelectedUIControlStatesChanged(Vector<UIControl::eControlState>)));
}

void BasePropertyGridWidget::UnregisterGridWidgetAsStateAware()
{
	disconnect(PropertiesGridController::Instance(),
			SIGNAL(SelectedUIControlStatesChanged(Vector<UIControl::eControlState>)),
			this,
			SLOT(OnSelectedUIControlStatesChanged(Vector<UIControl::eControlState>)));
    
}

void BasePropertyGridWidget::RegisterLineEditWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QLineEdit* lineEditWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(lineEditWidget,
                                                 PropertyGridWidgetData(curProperty, needUpdateTree, stateAware)));

    // Update the widget with the current property value.
    UpdateLineEditWidgetWithPropertyValue(lineEditWidget, curProperty);

    // Register the signal for this widget.
    connect(lineEditWidget, SIGNAL(editingFinished()), this, SLOT(OnLineEditEditingFinished()));
}

void BasePropertyGridWidget::RegisterSpinBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                              QSpinBox* spinBoxWidget,
                                                              bool needUpdateTree, bool stateAware)
{
    spinBoxWidget->setKeyboardTracking(false);
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(spinBoxWidget,
                                                 PropertyGridWidgetData(curProperty, needUpdateTree, stateAware)));
    
    UpdateSpinBoxWidgetWithPropertyValue(spinBoxWidget, curProperty);
    
    // Register the signal for this widget.
    connect(spinBoxWidget, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
}

void BasePropertyGridWidget::RegisterDoubleSpinBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                              QDoubleSpinBox* spinBoxWidget,
                                                              bool needUpdateTree, bool stateAware)
{
    spinBoxWidget->setKeyboardTracking(false);
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(spinBoxWidget,
                                                 PropertyGridWidgetData(curProperty, needUpdateTree, stateAware)));
    
    UpdateDoubleSpinBoxWidgetWithPropertyValue(spinBoxWidget, curProperty);
    
    // Register the signal for this widget.
    connect(spinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(OnDoubleSpinBoxValueChanged(double)));
}

void BasePropertyGridWidget::RegisterCheckBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
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
    connect(checkBoxWidget, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxStateChanged(int)));
}


void BasePropertyGridWidget::RegisterComboBoxWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QComboBox *comboBoxWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(comboBoxWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));
    
    UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
    
    // Register the signal for this widget.
    connect(comboBoxWidget, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
}

void BasePropertyGridWidget::RegisterPushButtonWidgetForProperty(const PROPERTIESMAP& propertiesMap, const char* propertyName,
                                                               QPushButton *pushButtonWidget,
                                                               bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }

    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(pushButtonWidget, PropertyGridWidgetData(curProperty,
                                                                                        needUpdateTree, stateAware)));

    UpdatePushButtonWidgetWithPropertyValue(pushButtonWidget, curProperty);

    // Register the signal for this widget.
    connect(pushButtonWidget, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
}

void BasePropertyGridWidget::RegisterColorWidgetForProperty(const PROPERTIESMAP &propertiesMap, const char *propertyName,
                                                                  QColorWidget *colorWidget,
                                                                  bool needUpdateTree, bool stateAware)
{
    PROPERTIESMAPCONSTITER iter = propertiesMap.find(propertyName);
    if (iter == propertiesMap.end())
    {
        Logger::Error("Unable to found property named %s in the properties map!", propertyName);
        return;
    }
    
    const QMetaProperty& curProperty = iter->second;
    propertyGridWidgetsMap.insert(std::make_pair(colorWidget, PropertyGridWidgetData(curProperty,
                                                                                           needUpdateTree, stateAware)));
    
    UpdateColorWidgetWithPropertyValue(colorWidget, curProperty);
    
    // Register the signal for this widget.
    connect(colorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(OnColorChanged(const QColor&)));
}

void BasePropertyGridWidget::RegisterPushButtonWidgetForInvokeMethod(QPushButton *pushButton, const String& methodName)
{
    invokableMethodsMap.insert(std::make_pair(pushButton, methodName));
    connect(pushButton, SIGNAL(clicked()), this, SLOT(OnInvokeMethodRequested()));
}

void BasePropertyGridWidget::UnregisterLineEditWidget(QLineEdit* lineEdit)
{
    disconnect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditEditingFinished()));
}

void BasePropertyGridWidget::UnregisterSpinBoxWidget(QSpinBox* spinBoxWidget)
{
    disconnect(spinBoxWidget, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxValueChanged(int)));
}

void BasePropertyGridWidget::UnregisterDoubleSpinBoxWidget(QDoubleSpinBox* spinBoxWidget)
{
    disconnect(spinBoxWidget, SIGNAL(valueChanged(double)), this, SLOT(OnDoubleSpinBoxValueChanged(double)));
}

void BasePropertyGridWidget::UnregisterCheckBoxWidget(QCheckBox* checkBoxWidget)
{
    disconnect(checkBoxWidget, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxStateChanged(int)));
}

void BasePropertyGridWidget::UnregisterComboBoxWidget(QComboBox* comboBoxWidget)
{
    disconnect(comboBoxWidget, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnComboBoxValueChanged(QString)));
}

void BasePropertyGridWidget::UnregisterPushButtonWidget(QPushButton *pushButtonWidget)
{
    disconnect(pushButtonWidget, SIGNAL(clicked()), this, SLOT(OnPushButtonClicked()));
}

void BasePropertyGridWidget::UnregisterPushButtonWidgetForInvokeMethod(QPushButton *pushButton)
{
    disconnect(pushButton, SIGNAL(clicked()), this, SLOT(OnInvokeMethodRequested()));
}

void BasePropertyGridWidget::UnregisterColorWidget(QColorWidget* colorWidget)
{
    disconnect(colorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(OnColorChanged(const QColor&)));
}

void BasePropertyGridWidget::OnLineEditEditingFinished()
{
    QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnLineEditValueChanged - sender is NULL!");
        return;
    }
    
    HandleLineEditEditingFinished(senderWidget);
}

void BasePropertyGridWidget::HandleLineEditEditingFinished(QLineEdit* senderWidget)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnLineEditValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
    
	
	// Don't update the property if the text wasn't actually changed. One exception though -
	// if the property values differ for different states, force update them. See please
	// DF-1987 for details.
	bool isValueDifferForStates = false;
    QString curValue = PropertiesHelper::GetAllPropertyValues<QString>(this->activeMetadata, iter->second.getProperty().name(), isValueDifferForStates);
	if (curValue == senderWidget->text() && !isValueDifferForStates)
	{
		return;
	}
	
	BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, senderWidget->text());
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnSpinBoxValueChanged(int value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QWidget* senderWidget = dynamic_cast<QWidget*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnSpinBoxValueChanged - sender is NULL!");
        return;
    }

    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnSpinBoxValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

	// Don't update the property if the text wasn't actually changed.
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}

    BaseCommand* command = new ChangePropertyCommand<float>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnDoubleSpinBoxValueChanged(double value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QDoubleSpinBox* senderWidget = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnDoubleSpinBoxValueChanged - sender is NULL!");
        return;
    }

    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnDoubleSpinBoxValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
	
	ProcessDoubleSpinBoxValueChanged(senderWidget, iter, value);
}

void BasePropertyGridWidget::OnCheckBoxStateChanged(int state)
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

    BaseCommand* command = new ChangePropertyCommand<bool>(activeMetadata, iter->second, newValue);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

void BasePropertyGridWidget::OnComboBoxValueChanged(QString value)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QComboBox* senderWidget = dynamic_cast<QComboBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnComboBoxValueChanged - sender is NULL!");
        return;
    }    
  
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnComboBoxValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
    
    ProcessComboboxValueChanged(senderWidget, iter, value);
}

void BasePropertyGridWidget::OnColorChanged(const QColor& color)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }

    QColorWidget* senderWidget = dynamic_cast<QColorWidget*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnColorChanged - sender is NULL!");
        return;
    }

	PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);

	if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnColorChanged - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

    BaseCommand* command = new ChangePropertyCommand<QColor>(activeMetadata, iter->second, color);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void BasePropertyGridWidget::OnPushButtonClicked()
{    
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnPushButtonClicked - sender is NULL!");
        return;
    }
    
    ProcessPushButtonClicked(senderWidget);
}

void BasePropertyGridWidget::OnInvokeMethodRequested()
{
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());
    if (senderWidget == NULL)
    {
        Logger::Error("OnInvokeMethodRequested - sender is NULL!");
        return;
    }

    Map<QWidget*, String>::iterator iter = invokableMethodsMap.find(senderWidget);
    if (iter == invokableMethodsMap.end())
    {
        Logger::Error("OnInvokeMethodRequested - no method registered for this control!");
        return;
    }

    ProcessInvokeMethodRequested(senderWidget, iter->second);
}

void BasePropertyGridWidget::ProcessInvokeMethodRequested(QWidget* /*widget*/, const String& methodName)
{
    if (!activeMetadata)
    {
        Logger::Error("ProcessInvokeMethodRequested - active metadata is not set!");
        return;
    }
    
    InvokeMethodCommand* command = new InvokeMethodCommand(activeMetadata, methodName);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void BasePropertyGridWidget::ProcessAttachedData(const PropertyGridWidgetData& attachedData)
{
    if (attachedData.IsNeedUpdateHierarchyTree())
    {
        HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
    }
}

void BasePropertyGridWidget::OnSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
	HandleSelectedUIControlStatesChanged(newStates);
}

void BasePropertyGridWidget::HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
    this->activeMetadata->SetUIControlStates(newStates);
    
    // Look through the map of widgets and update (re-read) properties attached to state-aware
    // widgets.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        const PropertyGridWidgetData& attachedData = iter->second;
        if (attachedData.IsStateAware() == false)
        {
            continue;
        }
        
        // The widget is state-aware. Update it according to its type.
        UpdateWidgetWithPropertyValue(iter);
    }
}

void BasePropertyGridWidget::UpdateWidgetWithPropertyValue(const PROPERTYGRIDWIDGETSITER& iter)
{
    QWidget* widget = iter->first;
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(widget);
    if (lineEdit)
    {
        UpdateLineEditWidgetWithPropertyValue(lineEdit, iter->second.getProperty());
        return;
    }

    QSpinBox* spinBoxWidget = dynamic_cast<QSpinBox*>(widget);
    if (spinBoxWidget)
    {
        UpdateSpinBoxWidgetWithPropertyValue(spinBoxWidget, iter->second.getProperty());
        return;
    }

    QDoubleSpinBox* doubleSpinBoxWidget = dynamic_cast<QDoubleSpinBox*>(widget);
    if (doubleSpinBoxWidget)
    {
        UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, iter->second.getProperty());
        return;
    }

    QCheckBox* checkBoxWidget = dynamic_cast<QCheckBox*>(widget);
    if (checkBoxWidget)
    {
        UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, iter->second.getProperty());
        return;
    }

    QComboBox* comboBoxWidget = dynamic_cast<QComboBox*>(widget);
    if (comboBoxWidget)
    {
        UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, iter->second.getProperty());
        return;
    }

    QColorWidget* colorWidget = dynamic_cast<QColorWidget*>(widget);
    if (colorWidget)
    {
        UpdateColorWidgetWithPropertyValue(colorWidget, iter->second.getProperty());
        return;
    }

    QPushButton *pushButtonWidget = dynamic_cast<QPushButton*>(widget);
    if (pushButtonWidget)
    {
        UpdatePushButtonWidgetWithPropertyValue(pushButtonWidget, iter->second.getProperty());
        return;
    }
   
    // Add other Widget Types here.
    Logger::Error("Unable to determine supported widget type for widget %s!", typeid(*widget).name());
}

void BasePropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
                                                                   const QMetaProperty& curProperty)
{
    // Property might have different values for different controls assigned.
    bool isPropertyValueDiffers = false;
    QString propertyValue =
        PropertiesHelper::GetAllPropertyValues<QString>(this->activeMetadata,
                                                        curProperty.name(),
                                                        isPropertyValueDiffers);
    
    {
        WidgetSignalsBlocker blocker(lineEditWidget);
        if (isPropertyValueDiffers)
        {
            lineEditWidget->setText(BasePropertyGridWidget::MULTIPLE_VALUE);
            lineEditWidget->setStyleSheet("font:bold;");
        }
        else
        {
            // Get the current value.
            lineEditWidget->setText(propertyValue);
            lineEditWidget->setStyleSheet("font:normal;");
        }
    }

    UpdateWidgetPalette(lineEditWidget, curProperty.name());
}

void BasePropertyGridWidget::UpdateSpinBoxWidgetWithPropertyValue(QSpinBox* spinBoxWidget,
                                                                  const QMetaProperty& curProperty)
{
    // Get the current value.
    float propertyValue = PropertiesHelper::GetAllPropertyValues<float>(this->activeMetadata,
                                                                        curProperty.name());
    
    {
        // For Spin Box set some value irregardless of isPropertyValueDiffers flag.
        WidgetSignalsBlocker blocker(spinBoxWidget);
        spinBoxWidget->setValue(propertyValue);
    }
    
    UpdateWidgetPalette(spinBoxWidget, curProperty.name());
}

void BasePropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget,
                                                                   const QMetaProperty& curProperty)
{
    // Get the current value.
    bool isPropertyValueDiffers = false;
    bool value = PropertiesHelper::GetAllPropertyValues<bool>(this->activeMetadata,
                                                              curProperty.name(),
                                                              isPropertyValueDiffers);

    {
        WidgetSignalsBlocker blocker(checkBoxWidget);
        if (isPropertyValueDiffers)
        {
            // For Check Box widget we'll set it to tri-state.
            checkBoxWidget->setTristate();
            checkBoxWidget->setCheckState(Qt::PartiallyChecked);
        }
        else
        {
            checkBoxWidget->setTristate(false);
            checkBoxWidget->setChecked(value);
        }
    }
}

void BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox*, const QMetaProperty&)
{
    // In case Base handler is called, particular handler for the DoubleSpinBox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue is called - you've forgot to create custom handler for some double spinbox!!!");
}

void BasePropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *, const PROPERTYGRIDWIDGETSITER &,
																const double)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::ProcessDoubleSpinBoxValueChanged is called - you've forgot to create custom handler for some combo!!!");
}

void BasePropertyGridWidget::UpdateColorWidgetWithPropertyValue(QColorWidget *colorWidget, const QMetaProperty &curProperty)
{
    // Get the current value.
    bool isPropertyValueDiffers = false;
    QColor propertyValue = PropertiesHelper::GetPropertyValue<QColor>(this->activeMetadata,
                                                          curProperty.name(), isPropertyValueDiffers);
    {
        //Set background color
        WidgetSignalsBlocker blocker(colorWidget);
        colorWidget->SetBackgroundColor(propertyValue);

        //Set background image if property is not equal to property for normal state
        colorWidget->SetDisplayMultipleColors(IsActiveStatePropertyDirty(curProperty.name()));
    }
}

void BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox*, const QMetaProperty &)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue is called - you've forgot to create custom handler for some combo!!!");
}

void BasePropertyGridWidget::ProcessComboboxValueChanged(QComboBox*, const PROPERTYGRIDWIDGETSITER&, const QString&)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::ProcessComboboxValueChanged is called - you've forgot to create custom handler for some combo!!!");
}

void BasePropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *, const QMetaProperty &)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue is called - you've forgot to create custom handler for button!!!");
}

void BasePropertyGridWidget::ProcessPushButtonClicked(QPushButton *)
{
    // In case Base handler is called, particular handler for the Combobox is missed - this is treated as error.
    Logger::Error("BasePropertyGridWidget::ProcessPushButtonClicked is called - you've forgot to create custom handler for this button!!!");
}

void BasePropertyGridWidget::SetComboboxSelectedItem(QComboBox* comboBoxWidget, const QString& selectedItemText, bool byToolTip)
{
    int index = -1;
    if (byToolTip)
    {
        index = comboBoxWidget->findData(QVariant(selectedItemText),Qt::ToolTipRole);
        if ( index != -1 )
        {
            comboBoxWidget->setCurrentIndex(index);
        }
    }
    else
    {
        index = comboBoxWidget->findText(selectedItemText);
        if ( index != -1 )
        {
            comboBoxWidget->setCurrentIndex(index);
        }
    }
}

void BasePropertyGridWidget::OnPropertiesChangedFromExternalSource()
{
    // Re-read all the properties related to this grid.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        UpdateWidgetWithPropertyValue(iter);
    }
}

void BasePropertyGridWidget::OnPropertiesChangedExternally()
{
    OnPropertiesChangedFromExternalSource();
}

void BasePropertyGridWidget::OnChangePropertySucceeded(const QString& propertyName)
{
    HandleChangePropertySucceeded(propertyName);
}

void BasePropertyGridWidget::OnChangePropertyFailed(const QString& propertyName)
{
    HandleChangePropertyFailed(propertyName);
}

void BasePropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    UpdateWidgetWithPropertyValue(propertyName);
}

void BasePropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    UpdateWidgetWithPropertyValue(propertyName);
}

void BasePropertyGridWidget::UpdateWidgetWithPropertyValue(const QString& propertyName)
{
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        const PropertyGridWidgetData& attachedData = iter->second;
        if (attachedData.getProperty().name() == propertyName)
        {
            UpdateWidgetWithPropertyValue(iter);
            
            // Also do the postprocessing.
            const PropertyGridWidgetData& attachedData = iter->second;
            ProcessAttachedData(attachedData);
            break;
        }
    }
}

bool BasePropertyGridWidget::IsWidgetBoundToProperty(QWidget* widget, const QString& propertyName) const
{
    BasePropertyGridWidget::PROPERTYGRIDWIDGETSCONSTITER iter = this->propertyGridWidgetsMap.find(widget);
    if (iter == this->propertyGridWidgetsMap.end())
    {
        return false;
    }
    
    return (iter->second.getProperty().name() == propertyName);
}

int BasePropertyGridWidget::GetPropertyIntValue(const QString &propertyName)
{
	if (activeMetadata == NULL)
    {
        return 0;
    }

	return PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, propertyName);
}

bool BasePropertyGridWidget::GetPropertyBooleanValue(const QString& propertyName) const
{
	if (activeMetadata == NULL)
    {
        return false;
    }

	return PropertiesHelper::GetAllPropertyValues<bool>(this->activeMetadata, propertyName);	
}

bool BasePropertyGridWidget::IsActiveStatePropertyDirty(const QString& propertyName) const
{
    if (activeMetadata == NULL)
    {
        return false;
    }

	bool res = false;

	const Vector<UIControl::eControlState> &states = activeMetadata->GetUIControlStates();
	for (uint32 i = 0; i < activeMetadata->GetStatesCount(); ++i)
	{
		UIControl::eControlState state = states[i];
		res |= IsPropertyDirty(propertyName, state);
	}

	return res;
}

bool BasePropertyGridWidget::IsPropertyDirty(const QString& propertyName, UIControl::eControlState state) const
{
    if (activeMetadata == NULL)
    {
        return false;
    }
    
    // Check for all attached Params.
    int paramsCount = this->activeMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = this->activeMetadata->GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        if (treeNode->GetExtraData().IsStatePropertyDirty(state, propertyName))
        {
            return true;
        }
    }
    
    // No "Dirty" flag for all attached Params.
    return false;
}

void BasePropertyGridWidget::UpdateWidgetPalette(QWidget* widget, const QString& propertyName)
{
    const QPalette& palette = IsActiveStatePropertyDirty(propertyName) ?
        GetWidgetPaletteForDirtyProperty() :
        GetWidgetPaletteForClearProperty();
    
    widget->setPalette(palette);
}

void BasePropertyGridWidget::PreparePalettes()
{
    dirtyPropertyPalette.setColor(QPalette::Text, Qt::red);
    clearPropertyPalette.setColor(QPalette::Text, QApplication::palette().text().color());
}

const QPalette& BasePropertyGridWidget::GetWidgetPaletteForDirtyProperty() const
{
    return this->dirtyPropertyPalette;
}

const QPalette& BasePropertyGridWidget::GetWidgetPaletteForClearProperty() const
{
    return this->clearPropertyPalette;
}

void BasePropertyGridWidget::InstallEventFiltersForWidgets(QWidget *widget)
{
	if (!widget)
		return;
		
	// Install event filter for all spinboxes on this Widget
	// We should block mouse wheel event for spinboxes which don't have focus
	Q_FOREACH( QAbstractSpinBox *spinBoxWidget, widget->findChildren<QAbstractSpinBox*>() )
	{
        spinBoxWidget->installEventFilter( this );
        spinBoxWidget->setFocusPolicy( Qt::StrongFocus );
    }
	// Install event filter for all comboboxes on this Widget
	// We should block mouse wheel event for comboboxes which don't have focus
	Q_FOREACH( QComboBox *comboBoxWidget, widget->findChildren<QComboBox*>() )
	{
        comboBoxWidget->installEventFilter( this );
        comboBoxWidget->setFocusPolicy( Qt::StrongFocus );
    }
	// Install event filter for all sliders on this Widget
	// We should block mouse wheel event for sliders which don't have focus
	Q_FOREACH( QSlider *sliderWidget, widget->findChildren<QSlider*>() )
	{
        sliderWidget->installEventFilter( this );
        sliderWidget->setFocusPolicy( Qt::StrongFocus );
    }

	Q_FOREACH( QLineEdit *lineWidget, widget->findChildren<QLineEdit*>() )
	{
        lineWidget->installEventFilter( this );
        lineWidget->setFocusPolicy( Qt::StrongFocus );
    }
}

bool BasePropertyGridWidget::eventFilter(QObject *obj, QEvent *event)
{
	// Try to cast sender object to one of the types - QComboBox, QAbstractSpinBox or QSlider
	// All these widget have to ignore mouse wheel event if they are not in focus
	QWidget *eventWidget = qobject_cast<QAbstractSpinBox*>(obj);
	if (!eventWidget)
	{
		eventWidget = qobject_cast<QComboBox*>(obj);
		if (!eventWidget)
		{
			eventWidget = qobject_cast<QSlider*>(obj);
		}
	}
	// If spinbox or combobox recieved a focus - we should allow to change its value with mouse wheel
	if ( event->type() == QEvent::FocusIn && eventWidget )
	{
		eventWidget->setFocusPolicy(Qt::WheelFocus);
	}
	// If spinbox or combobox lost a focus - disable wheel events for it
	if ( event->type() == QEvent::FocusOut && eventWidget )
	{
		eventWidget->setFocusPolicy(Qt::StrongFocus);
	}
	// Ignore wheel event if spinbox or combobox is not in focus
    if ( event->type() == QEvent::Wheel && eventWidget )
    {
		if (!eventWidget->hasFocus())
		{
        	event->ignore();
        	return true;
		}
    }
    
    return QWidget::eventFilter( obj, event );
}

bool BasePropertyGridWidget::ActiveControlIsSubcontrol()
{
	if (!activeMetadata || !activeMetadata->GetParamsCount())
	{
		return false;
	}
	
    int paramsCount = activeMetadata->GetParamsCount();
	const METADATAPARAMSVECT& params = activeMetadata->GetParams();
	
    for (int i = 0; i < paramsCount; i ++)
    {
		if (SubcontrolsHelper::ControlIsSubcontrol(params[i].GetUIControl()))
		{
			return true;
		}
	}

	return false;
}

QString BasePropertyGridWidget::PreprocessSpriteName(const QString& rawSpriteName)
{
    return ResourcesManageHelper::GetResourceRelativePath(rawSpriteName);
}
