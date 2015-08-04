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


#include "BaseAddEntityDialog.h"
#include "ui_BaseAddEntityDialog.h"
#include "Qt/Main/QtUtils.h"
#include <QSizePolicy>
#include <Qlabel>
#include "QScrollBar.h"
#include "Scene/System/BeastSystem.h"


#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataKeyedArchiveMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"
#include "Main/mainwindow.h"

BaseAddEntityDialog::BaseAddEntityDialog(QWidget* parent, QDialogButtonBox::StandardButtons buttons)
:	QDialog(parent),
	entity(NULL),
	ui(new Ui::BaseAddEntityDialog)
{
	ui->setupUi(this);
	setAcceptDrops(false);
	setWindowModality(Qt::NonModal);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);	
	setAttribute( Qt::WA_MacAlwaysShowToolWindow); // on top of all applications

	propEditor = ui->propertyEditor;
	propEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	propEditor->setTabKeyNavigation(false);
	propEditor->setAlternatingRowColors(true);
	propEditor->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	propEditor->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	propEditor->setIndentation(16);
	propEditor->SetEditTracking(true);
	connect(propEditor, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnItemEdited(const QModelIndex &)));

	ui->buttonBox->setStandardButtons(buttons);
    
    connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(CommandExecuted(SceneEditor2 *, const Command2*, bool)));
}

BaseAddEntityDialog::~BaseAddEntityDialog()
{
	propEditor->RemovePropertyAll();
	SafeRelease(entity);

	for(DAVA::Map<QWidget*, QWidget*>::iterator it = additionalWidgetMap.begin(); it != additionalWidgetMap.end(); ++it)
	{
		delete it->second;
	}
	additionalWidgetMap.clear();

	delete ui;
}

void BaseAddEntityDialog::showEvent(QShowEvent * event)
{
	QDialog::showEvent(event);
	propEditor->expandAll();
	PerformResize();
}

void BaseAddEntityDialog::PerformResize()
{
	QList<int> sizes;
	sizes.push_back(geometry().height() - ui->scrollAreaWidgetContents_2->geometry().height() - ui->buttonBox->height() - layout()->spacing() * 4);
	sizes.push_back(ui->scrollAreaWidgetContents_2->geometry().height());
	ui->splitter->setSizes(sizes);
}

QtPropertyData* BaseAddEntityDialog::AddInspMemberToEditor(void *object, const DAVA::InspMember * member)
{
	QtPropertyData* propData = QtPropertyDataIntrospection::CreateMemberData(object, member);
	propEditor->AppendProperty(member->Name().c_str(), propData);
	return propData;
}

QtPropertyData* BaseAddEntityDialog::AddKeyedArchiveMember(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::String& rowName)
{
	QtPropertyData*  propData = new QtPropertyKeyedArchiveMember(_archive, _key);
	propEditor->AppendProperty(rowName.c_str(), propData);
	return propData;
}

QtPropertyData* BaseAddEntityDialog::AddMetaObject(void *_object, const DAVA::MetaInfo *_meta, const String& rowName)
{
	QtPropertyData*  propData = new QtPropertyDataMetaObject( _object, _meta);
	propEditor->AppendProperty(rowName.c_str(), propData);
	return propData;
}

void BaseAddEntityDialog::SetEntity(DAVA::Entity* _entity)
{
	SafeRelease(entity);
	entity = SafeRetain(_entity);
}

void BaseAddEntityDialog::AddButton( QWidget* widget, eButtonAlign orientation)
{
	switch (orientation)
	{
		case BUTTON_ALIGN_LEFT:
			ui->lowerLayOut->insertWidget(0, widget);
			break;
		case BUTTON_ALIGN_RIGHT:
			ui->lowerLayOut->addWidget(widget);
			break;
		default:
			break;
	}
}

void BaseAddEntityDialog::AddButton( QWidget* widget, int32 position)
{
	ui->lowerLayOut->insertWidget(position, widget);
}

DAVA::Entity* BaseAddEntityDialog::GetEntity()
{
	return entity;
}

void BaseAddEntityDialog::AddControlToUserContainer(QWidget* widget)
{
	ui->userContentLayout->addWidget(widget);
}


void BaseAddEntityDialog::AddControlToUserContainer(QWidget* widget, const DAVA::String& labelString)
{
	QLabel* label = new QLabel(labelString.c_str(),this);
	int rowCount = ui->userContentLayout->rowCount();
	ui->userContentLayout->addWidget(label, rowCount, 0);
	ui->userContentLayout->addWidget(widget, rowCount, 1);
	additionalWidgetMap[widget] = label;
}

void BaseAddEntityDialog::RemoveControlFromUserContainer(QWidget* widget)
{
	ui->userContentLayout->removeWidget(widget);
	if(additionalWidgetMap.find(widget) != additionalWidgetMap.end())
	{
		QWidget* additionalWidget = additionalWidgetMap[widget];
		additionalWidgetMap.erase(additionalWidgetMap.find(widget));
		delete additionalWidget;
	}
}

void BaseAddEntityDialog::RemoveAllControlsFromUserContainer()
{
	QLayout* container = ui->userContentLayout;
	while (QLayoutItem* item = container->takeAt(0))
	{
		if (QWidget* widget = item->widget())
		{
			container->removeWidget( widget);
			if(additionalWidgetMap.find(widget) != additionalWidgetMap.end())
			{
				QWidget* additionalWidget = additionalWidgetMap[widget];
				additionalWidgetMap.erase(additionalWidgetMap.find(widget));
				delete additionalWidget;
			}
		}
	}
}

void BaseAddEntityDialog::GetIncludedControls(QList<QWidget*>& includedWidgets)
{
	QLayout* container = ui->userContentLayout;
	for (int i = 0; i < container->count(); ++i)
	{
		QWidget* child = container->itemAt(i)->widget();
		if(child)
		{
			includedWidgets.append(child);
		}
	}
}

void BaseAddEntityDialog::OnItemEdited(const QModelIndex &index)
{
	QtPropertyData *data = propEditor->GetProperty(index);
	Command2 *command = (Command2 *) data->CreateLastCommand();
	if(NULL != command)
	{
		SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
		if(NULL != curScene)
		{
			curScene->Exec(command);
		}
	}
}

void BaseAddEntityDialog::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
    if(propEditor)
    {
        propEditor->Update();
    }
}
