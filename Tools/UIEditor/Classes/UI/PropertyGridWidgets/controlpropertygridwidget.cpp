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


#include "controlpropertygridwidget.h"
#include "ui_controlpropertygridwidget.h"

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

#include "PropertyNames.h"
#include "BaseMetadata.h"

#include "BaseCommand.h"
#include "ChangePropertyCommand.h"

#include "UIAggregatorMetadata.h"

using namespace DAVA;

static const QString DEFAULT_CONTROL_PROPERTY_BLOCK_NAME = "UI Control";
static const QString CUSTOM_CONTROL_PROPERTY_BLOCK_NAME = "Custom Control";

ControlPropertyGridWidget::ControlPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ControlPropertyGridWidget),
	currentWidgetState(STATE_DEFAULT_CONTROL)
{
    ui->setupUi(this);
	
	this->ui->frameCustomControlData->setHidden(true);
	this->ui->frameMorphToCustomControl->setHidden(true);

    this->ui->objectNameLineEdit->setValidator(new QRegExpValidator(HierarchyTreeNode::GetNameRegExp(), this));
    this->ui->customControlLineEdit->setValidator(new QRegExpValidator(HierarchyTreeNode::GetNameRegExp(), this));
    this->ui->tagLineEdit->setValidator(new QIntValidator(-INT_MAX, INT_MAX, this));
    
    this->ui->objectNameLineEdit->installEventFilter(this);

	SetWidgetState(STATE_DEFAULT_CONTROL, true);
	ConnectToSignals();
}

ControlPropertyGridWidget::~ControlPropertyGridWidget()
{
	DisconnectFromSignals();
    delete ui;
}

void ControlPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterLineEditWidgetForProperty(propertiesMap, "UIControlClassName", ui->classNameLineEdit);
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->objectNameLineEdit, true);
    RegisterLineEditWidgetForProperty(propertiesMap, "Tag", ui->tagLineEdit);
	RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::CUSTOM_CONTROL_NAME, ui->customControlLineEdit);
	
	UpdatePropertiesForSubcontrol();
}

void ControlPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->objectNameLineEdit);
    UnregisterLineEditWidget(ui->tagLineEdit);
	UnregisterLineEditWidget(ui->customControlLineEdit);
}

// Change the state of the widget.
void ControlPropertyGridWidget::SetWidgetState(eWidgetState newState, bool forceUpdate)
{
	if (!forceUpdate && newState == currentWidgetState)
	{
		return;
	}

	static const int WIDGET_HEIGHT_DEFAULT = 160;
	static const int WIDGET_HEIGHT_CUSTOM = 200;
	static const int GROUP_YPOS = 120;

	switch (newState)
	{
		case STATE_DEFAULT_CONTROL:
		{
			this->resize(this->width(), WIDGET_HEIGHT_DEFAULT);
			this->setMaximumHeight(WIDGET_HEIGHT_DEFAULT);
			this->setMinimumHeight(WIDGET_HEIGHT_DEFAULT);

			this->ui->groupWidgetFrame->resize(this->ui->groupWidgetFrame->width(),
											   WIDGET_HEIGHT_DEFAULT - 1);
			this->ui->frameMorphToCustomControl->move(0, GROUP_YPOS);
			
			this->ui->frameCustomControlData->setHidden(true);
			this->ui->frameMorphToCustomControl->setHidden(false);
			
		    SetPropertyBlockName(DEFAULT_CONTROL_PROPERTY_BLOCK_NAME);
			
			break;
		}

		case STATE_CUSTOM_CONTROL:
		{
			this->resize(this->width(), WIDGET_HEIGHT_CUSTOM);
			this->setMaximumHeight(WIDGET_HEIGHT_CUSTOM);
			this->setMinimumHeight(WIDGET_HEIGHT_CUSTOM);

			this->ui->groupWidgetFrame->resize(this->ui->groupWidgetFrame->width(),
											   WIDGET_HEIGHT_CUSTOM - 1);
			this->ui->frameCustomControlData->move(0, GROUP_YPOS);
			
			this->ui->frameCustomControlData->setHidden(false);
			this->ui->frameMorphToCustomControl->setHidden(true);
			
			SetPropertyBlockName(CUSTOM_CONTROL_PROPERTY_BLOCK_NAME);
			
			break;
		}

		default:
		{
			break;
		}
	}

	this->currentWidgetState = newState;
}

void ControlPropertyGridWidget::ConnectToSignals()
{
	connect(this->ui->btnMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnMorphToCustomControlClicked()));
	connect(this->ui->btnResetMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnResetMorphToCustomControlClicked()));
}

void ControlPropertyGridWidget::DisconnectFromSignals()
{
	disconnect(this->ui->btnMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnMorphToCustomControlClicked()));
	disconnect(this->ui->btnResetMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnResetMorphToCustomControlClicked()));
}

void ControlPropertyGridWidget::OnMorphToCustomControlClicked()
{
	// Just set the widget state to Custom Control. If the name of the Custom Control
	// will not be explicitely entered by the user, the state will be reset.
	SetWidgetState(STATE_CUSTOM_CONTROL);
}

void ControlPropertyGridWidget::OnResetMorphToCustomControlClicked()
{
	// Reset the state of the Control by changing the property.
	PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(ui->customControlLineEdit);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnResetMorphToCustomControlClicked - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

	BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, QString());
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
	
	// The state will be changed later on - in the UpdateLineEditWidgetWithPropertyValue().
}

void ControlPropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
                                                                   const QMetaProperty& curProperty)
{
	BasePropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(lineEditWidget, curProperty);
	
	if (lineEditWidget != this->ui->customControlLineEdit)
	{
		return;
	}
	
	// Determine the new widget state depending on the value.
	eWidgetState widgetState = this->ui->customControlLineEdit->text().isEmpty() ?
		STATE_DEFAULT_CONTROL : STATE_CUSTOM_CONTROL;
	SetWidgetState(widgetState);
}

void ControlPropertyGridWidget::UpdatePropertiesForSubcontrol()
{
	if (!activeMetadata || !activeMetadata->GetParamsCount())
	{
		return;
	}

	bool isSubcontrol = ActiveControlIsSubcontrol();

	// Several properties can't be changed for subcontrols.
	this->ui->customControlLineEdit->setReadOnly(isSubcontrol);
	this->ui->objectNameLineEdit->setReadOnly(isSubcontrol);
	this->ui->btnMorphToCustomControl->setEnabled(!isSubcontrol);
	
	// Hide morph button for UI Aggregator
	UIAggregatorMetadata *UIAggregatorMeta = dynamic_cast<UIAggregatorMetadata*>(activeMetadata);
	this->ui->btnMorphToCustomControl->setHidden(UIAggregatorMeta ? true : false);
}

bool ControlPropertyGridWidget::eventFilter(QObject *object, QEvent *event)
{
    if ((event->type() == QEvent::FocusOut) && (object == ui->objectNameLineEdit) && (ui->objectNameLineEdit->text() != BasePropertyGridWidget::MULTIPLE_VALUE))
    {
        ui->objectNameLineEdit->setStyleSheet("font:normal;");
    }

    if ((event->type() == QEvent::FocusIn) && (object == ui->objectNameLineEdit) && (ui->objectNameLineEdit->text() == BasePropertyGridWidget::MULTIPLE_VALUE))
    {
        ui->objectNameLineEdit->setStyleSheet("font:normal;");
        ui->objectNameLineEdit->setText("");
    }
    return  QObject::eventFilter(object, event);;
}
