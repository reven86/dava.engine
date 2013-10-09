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



#include "HeightmapEditorPropertiesView.h"
#include "ui_heightmapEditorProperties.h"

#include "Qt/Scene/SceneSignals.h"
#include "../Main/mainwindow.h"

#include <QMessageBox>

HeightmapEditorPropertiesView::HeightmapEditorPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::HeightmapEditorPropertiesView)
,	activeScene(NULL)
,	toolbarAction(NULL)
{
	ui->setupUi(this);

	Init();
}

HeightmapEditorPropertiesView::~HeightmapEditorPropertiesView()
{
	delete ui;
}

void HeightmapEditorPropertiesView::Init()
{
	InitBrushImages();
	toolbarAction = QtMainWindow::Instance()->GetUI()->actionHeightMapEditor;

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(DropperHeightChanged(SceneEditor2*, double)),
			this, SLOT(SetDropperHeight(SceneEditor2*, double)));

	connect(ui->sliderStrength, SIGNAL(valueChanged(int)), ui->labelStrength, SLOT(setNum(int)));
	connect(ui->sliderAverageStrength, SIGNAL(valueChanged(int)), ui->labelAverageStrength, SLOT(setNum(int)));

	connect(ui->buttonEnableHeightmapEditor, SIGNAL(clicked()), this, SLOT(Toggle()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->comboBrushImage, SIGNAL(currentIndexChanged(int)), this, SLOT(SetToolImage(int)));
	connect(ui->radioRelAbs, SIGNAL(clicked()), this, SLOT(SetRelativeDrawing()));
	connect(ui->radioAvg, SIGNAL(clicked()), this, SLOT(SetAverageDrawing()));
	connect(ui->radioAbsDrop, SIGNAL(clicked()), this, SLOT(SetAbsoluteDrawing()));
	connect(ui->radioDropper, SIGNAL(clicked()), this, SLOT(SetDropper()));
	connect(ui->radioCopyPaste, SIGNAL(clicked()), this, SLOT(SetHeightmapCopyPaste()));
	connect(ui->sliderStrength, SIGNAL(valueChanged(int)), this, SLOT(SetStrength(int)));
	connect(ui->sliderAverageStrength, SIGNAL(valueChanged(int)), this, SLOT(SetAverageStrength(int)));
	connect(ui->checkboxHeightmap, SIGNAL(stateChanged(int)), this, SLOT(SetCopyPasteHeightmap(int)));
	connect(ui->checkboxTilemask, SIGNAL(stateChanged(int)), this, SLOT(SetCopyPasteTilemask(int)));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void HeightmapEditorPropertiesView::InitBrushImages()
{
	QSize iconSize = ui->comboBrushImage->iconSize();
	iconSize = iconSize.expandedTo(QSize(32, 32));
	ui->comboBrushImage->setIconSize(iconSize);

	FilePath toolsPath("~res:/LandscapeEditor/Tools/");
	FileList *fileList = new FileList(toolsPath);
	for(int32 iFile = 0; iFile < fileList->GetCount(); ++iFile)
	{
		String filename = fileList->GetFilename(iFile);
		if(fileList->GetPathname(iFile).IsEqualToExtension(".png"))
		{
			String fullname = fileList->GetPathname(iFile).GetAbsolutePathname();

			FilePath f = fileList->GetPathname(iFile);
			f.ReplaceExtension("");

			QString qFullname = QString::fromStdString(fullname);
			QIcon toolIcon(qFullname);
			ui->comboBrushImage->addItem(toolIcon, f.GetFilename().c_str(), QVariant(qFullname));
		}
	}
}

void HeightmapEditorPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void HeightmapEditorPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	activeScene = NULL;
}

void HeightmapEditorPropertiesView::SetDropperHeight(SceneEditor2* scene, double height)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}

	ui->labelDropperHeight->setText(QString::number(height));
}

void HeightmapEditorPropertiesView::SetWidgetsState(bool enabled)
{
	ui->buttonEnableHeightmapEditor->blockSignals(true);
	ui->buttonEnableHeightmapEditor->setCheckable(enabled);
	ui->buttonEnableHeightmapEditor->setChecked(enabled);
	ui->buttonEnableHeightmapEditor->blockSignals(false);
	toolbarAction->setChecked(enabled);

	QString buttonText = enabled ? tr("Disable Height Map Editor") : tr("Enable Height Map Editor");
	ui->buttonEnableHeightmapEditor->setText(buttonText);

	ui->sliderBrushSize->setEnabled(enabled);
	ui->comboBrushImage->setEnabled(enabled);
	ui->radioRelAbs->setEnabled(enabled);
	ui->radioAvg->setEnabled(enabled);
	ui->radioAbsDrop->setEnabled(enabled);
	ui->sliderStrength->setEnabled(enabled);
	ui->sliderAverageStrength->setEnabled(enabled);
	ui->radioDropper->setEnabled(enabled);
	ui->radioCopyPaste->setEnabled(enabled);
	ui->checkboxHeightmap->setEnabled(enabled);
	ui->checkboxTilemask->setEnabled(enabled);

	BlockAllSignals(!enabled);
}

void HeightmapEditorPropertiesView::BlockAllSignals(bool block)
{
	ui->sliderBrushSize->blockSignals(block);
	ui->comboBrushImage->blockSignals(block);
	ui->radioRelAbs->blockSignals(block);
	ui->radioAvg->blockSignals(block);
	ui->radioAbsDrop->blockSignals(block);
	ui->sliderStrength->blockSignals(block);
	ui->sliderAverageStrength->blockSignals(block);
	ui->radioDropper->blockSignals(block);
	ui->radioCopyPaste->blockSignals(block);
	ui->checkboxHeightmap->blockSignals(block);
	ui->checkboxTilemask->blockSignals(block);
}

void HeightmapEditorPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->heightmapEditorSystem->IsLandscapeEditingEnabled();
	int32 brushSize = scene->heightmapEditorSystem->GetBrushSize();
	float32 strength = scene->heightmapEditorSystem->GetStrength();
	float32 averageStrength = scene->heightmapEditorSystem->GetAverageStrength();
	int32 averageStrengthVal = (int32)(averageStrength * ui->sliderAverageStrength->maximum());
	int32 toolImage = scene->heightmapEditorSystem->GetToolImage();
	HeightmapEditorSystem::eHeightmapDrawType drawingType = scene->heightmapEditorSystem->GetDrawingType();
	bool copyPasteHeightmap = scene->heightmapEditorSystem->GetCopyPasteHeightmap();
	bool copyPasteTilemask = scene->heightmapEditorSystem->GetCopyPasteTilemask();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderBrushSize->setValue(brushSize);
	ui->sliderStrength->setValue(strength);
	ui->sliderAverageStrength->setValue(averageStrengthVal);
	ui->comboBrushImage->setCurrentIndex(toolImage);
	ui->checkboxHeightmap->setChecked(copyPasteHeightmap);
	ui->checkboxTilemask->setChecked(copyPasteTilemask);

	ui->labelStrength->setNum((int32)strength);
	ui->labelAverageStrength->setNum(averageStrengthVal);

	ui->radioRelAbs->setChecked(false);
	ui->radioAvg->setChecked(false);
	ui->radioAbsDrop->setChecked(false);
	ui->radioDropper->setChecked(false);
	ui->radioCopyPaste->setChecked(false);

	switch (drawingType)
	{
		case HeightmapEditorSystem::HEIGHTMAP_COPY_PASTE:
			ui->radioCopyPaste->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE:
			ui->radioRelAbs->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_AVERAGE:
			ui->radioAvg->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
			ui->radioAbsDrop->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DROPPER:
			ui->radioDropper->setChecked(true);
			break;

		default:
			break;
	}

	BlockAllSignals(!enabled);
}

void HeightmapEditorPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->heightmapEditorSystem->IsLandscapeEditingEnabled())
	{
		if (activeScene->heightmapEditorSystem->DisableLandscapeEdititing())
		{
			SetWidgetsState(false);
		}
		else
		{
			// show "Couldn't disable heightmap editing" message box
		}
	}
	else
	{
		if (activeScene->heightmapEditorSystem->EnableLandscapeEditing())
		{
			SetWidgetsState(true);

			SetBrushSize(ui->sliderBrushSize->value());
			SetStrength(ui->sliderStrength->value());
			SetAverageStrength(ui->sliderAverageStrength->value());
			SetToolImage(ui->comboBrushImage->currentIndex());
			SetCopyPasteHeightmap(ui->checkboxHeightmap->checkState());
			SetCopyPasteTilemask(ui->checkboxTilemask->checkState());
		}
		else
		{
			QMessageBox::critical(0, "Error enabling Height Map editor",
								  "Error enabling Height Map editor.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}

	toolbarAction->setChecked(activeScene->heightmapEditorSystem->IsLandscapeEditingEnabled());
}

void HeightmapEditorPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetBrushSize(brushSize);
}

void HeightmapEditorPropertiesView::SetToolImage(int toolImage)
{
	if (!activeScene)
	{
		return;
	}

	QString s = ui->comboBrushImage->itemData(toolImage).toString();

	if (!s.isEmpty())
	{
		FilePath fp(s.toStdString());
		activeScene->heightmapEditorSystem->SetToolImage(fp, toolImage);
	}
}

void HeightmapEditorPropertiesView::SetRelativeDrawing()
{
	if (!activeScene)
	{
		return;
	}

	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE);
}

void HeightmapEditorPropertiesView::SetAverageDrawing()
{
	if (!activeScene)
	{
		return;
	}

	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_AVERAGE);
}

void HeightmapEditorPropertiesView::SetAbsoluteDrawing()
{
	if (!activeScene)
	{
		return;
	}

	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE_DROPPER);
}

void HeightmapEditorPropertiesView::SetDropper()
{
	if (!activeScene)
	{
		return;
	}

	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DROPPER);
}

void HeightmapEditorPropertiesView::SetHeightmapCopyPaste()
{
	if (!activeScene)
	{
		return;
	}

	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_COPY_PASTE);
}

void HeightmapEditorPropertiesView::SetStrength(int strength)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetStrength(strength);
}

void HeightmapEditorPropertiesView::SetAverageStrength(int averageStrength)
{
	if (!activeScene)
	{
		return;
	}

	float32 avStr = (float32)averageStrength;
	float32 max = ui->sliderAverageStrength->maximum();
	float32 v = avStr / max;
	activeScene->heightmapEditorSystem->SetAverageStrength(v);
}

void HeightmapEditorPropertiesView::SetCopyPasteHeightmap(int state)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetCopyPasteHeightmap(state == Qt::Checked);
}

void HeightmapEditorPropertiesView::SetCopyPasteTilemask(int state)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetCopyPasteTilemask(state == Qt::Checked);
}

void HeightmapEditorPropertiesView::SetDrawingType(HeightmapEditorSystem::eHeightmapDrawType type)
{
	if (!activeScene)
	{
		return;
	}

	BlockAllSignals(true);

	ui->radioRelAbs->setChecked(false);
	ui->radioAvg->setChecked(false);
	ui->radioAbsDrop->setChecked(false);
	ui->radioDropper->setChecked(false);
	ui->radioCopyPaste->setChecked(false);

	switch (type)
	{
		default:
			type = HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE:
			ui->radioRelAbs->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_AVERAGE:
			ui->radioAvg->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
			ui->radioAbsDrop->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DROPPER:
			ui->radioDropper->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_COPY_PASTE:
			ui->radioCopyPaste->setChecked(true);
			break;
	}

	BlockAllSignals(false);

	activeScene->heightmapEditorSystem->SetDrawingType(type);
}
