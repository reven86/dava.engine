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



#include "VisibilityToolView.h"
#include "ui_VisibilityToolView.h"

#include "Classes/Qt/Main/QtMainWindowHandler.h"

VisibilityToolView::VisibilityToolView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::VisibilityToolView)
{
	ui->setupUi(this);
	
	Init();
}

VisibilityToolView::~VisibilityToolView()
{
	delete ui;
}

void VisibilityToolView::Init()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();

	connect(ui->buttonVisibilityToolEnable, SIGNAL(clicked()), handler, SLOT(ToggleVisibilityTool()));

	ui->buttonVisibilityToolSave->blockSignals(true);
	ui->buttonVisibilityToolSetArea->blockSignals(true);
	ui->buttonVisibilityToolSetPoint->blockSignals(true);
	ui->sliderVisibilityToolAreaSize->blockSignals(true);

	connect(ui->buttonVisibilityToolSave,		SIGNAL(clicked()),
			handler,							SLOT(SaveTextureVisibilityTool()));
	connect(ui->buttonVisibilityToolSetArea,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityAreaVisibilityTool()));
	connect(ui->buttonVisibilityToolSetPoint,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityPointVisibilityTool()));
	connect(ui->sliderVisibilityToolAreaSize,	SIGNAL(valueChanged(int)),
			handler,							SLOT(ChangleAreaSizeVisibilityTool(int)));

	handler->RegisterWidgetsVisibilityTool(ui->buttonVisibilityToolEnable,
										   ui->buttonVisibilityToolSave,
										   ui->buttonVisibilityToolSetPoint,
										   ui->buttonVisibilityToolSetArea,
										   ui->sliderVisibilityToolAreaSize);

	handler->SetWidgetsStateVisibilityTool(false);
}
