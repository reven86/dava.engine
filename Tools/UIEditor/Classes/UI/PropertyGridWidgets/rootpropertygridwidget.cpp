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


#include "rootpropertygridwidget.h"
#include "HierarchyTreeController.h"
#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "PropertiesHelper.h"
#include "LibraryController.h"

#include <QEvent>
#include <QMessageBox>

RootPropertyGridWidget::RootPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent)
{
}

RootPropertyGridWidget::~RootPropertyGridWidget()
{
}

bool RootPropertyGridWidget::eventFilter(QObject *obj, QEvent *event)
{
	QWidget *eventWidget = qobject_cast<QLineEdit*>(obj);
	if (event->type() == QEvent::FocusOut && eventWidget)
	{
		event->ignore();
		return true;
	}
    
    return QWidget::eventFilter(obj, event);
}

void RootPropertyGridWidget::HandleLineEditEditingFinished(QLineEdit* senderWidget)
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
	
	// Do not change aggregator or screen name - if such name already present at platform
	if (HierarchyTreeController::Instance()->GetActivePlatform()->IsAggregatorOrScreenNamePresent(senderWidget->text()) ||
			HierarchyTreeController::Instance()->GetTree().IsPlatformNamePresent(senderWidget->text()))
	{
		QMessageBox msgBox;
		msgBox.setText("Agreggator, Screen or Platform with the same name already exist. Please fill the name field with unique value.");
		msgBox.exec();
			
		return;
	}

    // Do not allow library control name
    if(LibraryController::Instance()->IsControlNameExists(senderWidget->text()))
    {
        QMessageBox msgBox;
        msgBox.setText("A library control with the same name already exist. Please fill the name field with unique value.");
        msgBox.exec();
        return;
    }
	
	BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, senderWidget->text());
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}

