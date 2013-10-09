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

BaseAddEntityDialog::BaseAddEntityDialog(QWidget* parent)
:	QDialog(parent),
	entity(NULL),
	ui(new Ui::BaseAddEntityDialog)
{
	ui->setupUi(this);
	setAcceptDrops(false);
	setWindowModality(Qt::NonModal);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);	
	setAttribute( Qt::WA_MacAlwaysShowToolWindow);// on top of all applications
}

BaseAddEntityDialog::~BaseAddEntityDialog()
{
	delete ui;
	SafeRelease(entity);
}

void BaseAddEntityDialog::showEvent ( QShowEvent * event )
{
	QDialog::showEvent(event);
	if(!entity)
	{
		return;
	}
	PropertyEditor* pEditor = ui->propEditor;
	pEditor->SetNode(entity);
	pEditor->expandAll();
	int height= ui->verticalLayout->sizeHint().height();
	QRect rect = geometry();
	rect.setHeight(height);
	setMinimumHeight(height);
	setGeometry(rect);
}

void BaseAddEntityDialog::hideEvent ( QHideEvent * event )
{
	QDialog::hideEvent(event);
	ui->propEditor->SetNode(NULL);
}

void BaseAddEntityDialog::SetEntity(DAVA::Entity* _entity)
{
	SafeRelease(entity);
	
	entity = _entity;
	SafeRetain(entity);
	if(entity)
	{
		setWindowTitle(QString("Add ") + QString(entity->GetName().c_str()));
	}
}

void BaseAddEntityDialog::AddControlToUserContainer(QWidget* widget)
{
	ui->userContentLayout->addWidget(widget);
}

void BaseAddEntityDialog::RemoveControlFromUserContainer(QWidget* widget)
{
	setMinimumHeight(minimumHeight() - widget->height());
	ui->userContentLayout->removeWidget(widget);
}

void BaseAddEntityDialog::RemoveAllControlsFromUserContainer()
{
	QVBoxLayout* container = ui->userContentLayout;
	while (QLayoutItem* item = container->takeAt(0))
	{
		if (QWidget* widget = item->widget())
		{
			container->removeWidget( widget);
			setMinimumHeight(minimumHeight() - widget->height());
		}
	}
}

void BaseAddEntityDialog::GetIncludedControls(QList<QWidget*>& includedWidgets)
{
	QVBoxLayout* container = ui->userContentLayout;
	for (int i = 0; i < container->count(); ++i)
	{
		QWidget* child = container->itemAt(i)->widget();
		if(child)
		{
			includedWidgets.append(child);
		}
	}
}

