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



#include "HangingObjectsView.h"
#include "ui_HangingObjectsView.h"
#include <stdlib.h> 
#include "Project/ProjectManager.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "../SceneEditor/EditorConfig.h"

HangingObjectsView::HangingObjectsView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::HangingObjectsView)
{
	ui->setupUi(this);
	
	Init();
}

HangingObjectsView::~HangingObjectsView()
{
	delete ui;
}

void HangingObjectsView::Init()
{
	// TODO: mainwindow
	/*
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(this, SIGNAL(Clicked(float,bool)), handler, SLOT(ToggleHangingObjects(float,bool)));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(Clicked()));
	connect(ui->checkBoxEnable, SIGNAL(stateChanged(int)), this, SLOT(CheckBoxChangeState(int )));
	
	ui->btnUpdate->blockSignals(true);
	QtMainWindowHandler::Instance()->RegisterHangingObjectsWidgets(ui->checkBoxEnable,
		ui->doubleSpinBoxHeight,
		ui->btnUpdate);

	handler->SetHangingObjectsWidgetsState(false);
	ui->checkBoxEnable->setEnabled(true);
	*/
}


void HangingObjectsView::Clicked()
{
	float value = (float)ui->doubleSpinBoxHeight->value();
	emit Clicked(value, ui->checkBoxEnable->isChecked());
}

void HangingObjectsView::CheckBoxChangeState(int newState)
{
	if(newState == Qt::Unchecked)
	{
		ui->doubleSpinBoxHeight->setEnabled(false);
		ui->btnUpdate->setEnabled(false);
	}
	if(newState == Qt::Checked)
	{
		ui->doubleSpinBoxHeight->setEnabled(true);
		ui->btnUpdate->setEnabled(true);
	}
	this->Clicked();
}
