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



#include "VisibilityToolPropertiesView.h"
#include "ui_VisibilityToolPropertiesView.h"

#include "Main/QtUtils.h"
#include "../Scene/SceneSignals.h"
#include "../Scene/SceneEditor2.h"

#include <QFileDialog>

VisibilityToolPropertiesView::VisibilityToolPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::VisibilityToolPropertiesView)
,	activeScene(NULL)
{
	ui->setupUi(this);

	Init();
}

VisibilityToolPropertiesView::~VisibilityToolPropertiesView()
{
	delete ui;
}

void VisibilityToolPropertiesView::Init()
{
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(VisibilityToolStateChanged(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)),
			this, SLOT(SetVisibilityToolButtonsState(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)));

	connect(ui->buttonEnableVisibilityTool, SIGNAL(clicked()), this, SLOT(Toggle()));
	connect(ui->buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(ui->buttonSetVisibilityPoint, SIGNAL(clicked()), this, SLOT(SetVisibilityPoint()));
	connect(ui->buttonSetVisibilityArea, SIGNAL(clicked()), this, SLOT(SetVisibilityArea()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), this, SLOT(SetVisibilityAreaSize(int)));

	SetWidgetsState(false);
}

void VisibilityToolPropertiesView::SetWidgetsState(bool enabled)
{
	ui->buttonEnableVisibilityTool->blockSignals(true);
	ui->buttonEnableVisibilityTool->setCheckable(enabled);
	ui->buttonEnableVisibilityTool->setChecked(enabled);
	ui->buttonEnableVisibilityTool->blockSignals(false);

	QString toggleButtonText = enabled ? tr("Disable Visibility Tool"): tr("Enable Visibility Tool");
	ui->buttonEnableVisibilityTool->setText(toggleButtonText);

	ui->buttonSaveTexture->setEnabled(enabled);
	ui->buttonSetVisibilityPoint->setEnabled(enabled);
	ui->buttonSetVisibilityArea->setEnabled(enabled);
	ui->sliderBrushSize->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void VisibilityToolPropertiesView::BlockAllSignals(bool block)
{
	ui->buttonSaveTexture->blockSignals(block);
	ui->buttonSetVisibilityPoint->blockSignals(block);
	ui->buttonSetVisibilityArea->blockSignals(block);
	ui->sliderBrushSize->blockSignals(block);
}

void VisibilityToolPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void VisibilityToolPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	activeScene = NULL;
}

void VisibilityToolPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->visibilityToolSystem->IsLandscapeEditingEnabled();
	int32 brushSize = scene->visibilityToolSystem->GetBrushSize();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderBrushSize->setValue(brushSize);
	BlockAllSignals(!enabled);
}

void VisibilityToolPropertiesView::SetVisibilityToolButtonsState(SceneEditor2* scene,
																 VisibilityToolSystem::eVisibilityToolState state)
{
	if (!activeScene || scene != activeScene)
	{
		return;
	}

	bool pointButton = false;
	bool areaButton = false;

	switch (state)
	{
		case VisibilityToolSystem::VT_STATE_SET_AREA:
			areaButton = true;
			break;

		case VisibilityToolSystem::VT_STATE_SET_POINT:
			pointButton = true;
			break;

		default:
			break;
	}
	bool b;

	b = ui->buttonSetVisibilityPoint->signalsBlocked();
	ui->buttonSetVisibilityPoint->blockSignals(true);
	ui->buttonSetVisibilityPoint->setChecked(pointButton);
	ui->buttonSetVisibilityPoint->blockSignals(b);

	b = ui->buttonSetVisibilityArea->signalsBlocked();
	ui->buttonSetVisibilityArea->blockSignals(true);
	ui->buttonSetVisibilityArea->setChecked(areaButton);
	ui->buttonSetVisibilityArea->blockSignals(b);
}

void VisibilityToolPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->visibilityToolSystem->IsLandscapeEditingEnabled())
	{
		if (activeScene->visibilityToolSystem->DisableLandscapeEdititing())
		{
			SetWidgetsState(false);
		}
		else
		{
			// show "Couldn't disable visibility tool" message box
		}
	}
	else
	{
		if (activeScene->visibilityToolSystem->EnableLandscapeEditing())
		{
			SetWidgetsState(true);

			SetVisibilityAreaSize(ui->sliderBrushSize->value());
		}
		else
		{
			QMessageBox::critical(0, "Error enabling Visibility Check Tool",
								  "Error enabling Visibility Check Tool.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}
}

void VisibilityToolPropertiesView::SaveTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString("Save visibility tool texture"),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString("PNG image (*.png)"));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		activeScene->visibilityToolSystem->SaveTexture(selectedPathname);
	}
}

void VisibilityToolPropertiesView::SetVisibilityPoint()
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetVisibilityPoint();
}

void VisibilityToolPropertiesView::SetVisibilityArea()
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetVisibilityArea();
}

void VisibilityToolPropertiesView::SetVisibilityAreaSize(int areaSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetBrushSize(areaSize);
}
