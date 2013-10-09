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



#include "TilemaskEditorPropertiesView.h"
#include "ui_tilemaskEditorProperties.h"

#include "../Scene/SceneEditor2.h"
#include "TextureBrowser/TextureConvertor.h"

#include "Qt/Scene/SceneSignals.h"
#include "../Main/mainwindow.h"

#include <QMessageBox>

TilemaskEditorPropertiesView::TilemaskEditorPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::TilemaskEditorPropertiesView)
,	activeScene(NULL)
{
	ui->setupUi(this);

	Init();
}

TilemaskEditorPropertiesView::~TilemaskEditorPropertiesView()
{
	delete ui;
}

void TilemaskEditorPropertiesView::Init()
{
	InitBrushImages();
	toolbarAction = QtMainWindow::Instance()->GetUI()->actionTileMapEditor;

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));

	connect(ui->buttonEnableTilemaskEditor, SIGNAL(clicked()), this, SLOT(Toggle()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->comboBrushImage, SIGNAL(currentIndexChanged(int)), this, SLOT(SetToolImage(int)));
	connect(ui->sliderStrength, SIGNAL(valueChanged(int)), this, SLOT(SetStrength(int)));
	connect(ui->comboTileTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDrawTexture(int)));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void TilemaskEditorPropertiesView::InitBrushImages()
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

void TilemaskEditorPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void TilemaskEditorPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	activeScene = NULL;
}

void TilemaskEditorPropertiesView::SetWidgetsState(bool enabled)
{
	ui->buttonEnableTilemaskEditor->blockSignals(true);
	ui->buttonEnableTilemaskEditor->setCheckable(enabled);
	ui->buttonEnableTilemaskEditor->setChecked(enabled);
	ui->buttonEnableTilemaskEditor->blockSignals(false);
	toolbarAction->setChecked(enabled);

	QString buttonText = enabled ? tr("Disable Tile Mask Editor") : tr("Enable Tile Mask Editor");
	ui->buttonEnableTilemaskEditor->setText(buttonText);

	ui->sliderBrushSize->setEnabled(enabled);
	ui->comboBrushImage->setEnabled(enabled);
	ui->sliderStrength->setEnabled(enabled);
	ui->comboTileTexture->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void TilemaskEditorPropertiesView::BlockAllSignals(bool block)
{
	ui->sliderBrushSize->blockSignals(block);
	ui->comboBrushImage->blockSignals(block);
	ui->sliderStrength->blockSignals(block);
	ui->comboTileTexture->blockSignals(block);
}

void TilemaskEditorPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->tilemaskEditorSystem->IsLandscapeEditingEnabled();
	int32 brushSize = scene->tilemaskEditorSystem->GetBrushSize();
	float32 strength = scene->tilemaskEditorSystem->GetStrength();
	int32 strengthVal = (int32)(strength * 2.f * ui->sliderStrength->maximum());
	uint32 tileTexture = scene->tilemaskEditorSystem->GetTileTextureIndex();
	int32 toolImage = scene->tilemaskEditorSystem->GetToolImage();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderBrushSize->setValue(brushSize);
	ui->sliderStrength->setValue(strengthVal);
	ui->comboTileTexture->setCurrentIndex(tileTexture);
	ui->comboBrushImage->setCurrentIndex(toolImage);
	BlockAllSignals(!enabled);
}

void TilemaskEditorPropertiesView::UpdateTileTextures()
{
	if (!activeScene)
	{
		return;
	}

	ui->comboTileTexture->clear();

	QSize iconSize = ui->comboTileTexture->iconSize();
	iconSize = iconSize.expandedTo(QSize(150, 32));
	ui->comboTileTexture->setIconSize(iconSize);

	for (int32 i = 0; i < (int32)activeScene->tilemaskEditorSystem->GetTileTextureCount(); ++i)
	{
		Texture* tileTexture = activeScene->tilemaskEditorSystem->GetTileTexture(i);

		uint32 previewWidth = Min(tileTexture->GetWidth(), 150);
		uint32 previewHeight = Min(tileTexture->GetHeight(), 32);

		Image* tileImage = tileTexture->CreateImageFromMemory();
		tileImage->ResizeCanvas(previewWidth, previewHeight);

		QImage img = TextureConvertor::FromDavaImage(tileImage);
		QIcon icon = QIcon(QPixmap::fromImage(img));

		ui->comboTileTexture->addItem(icon, "");
	}
}

void TilemaskEditorPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->tilemaskEditorSystem->IsLandscapeEditingEnabled())
	{
		if (activeScene->tilemaskEditorSystem->DisableLandscapeEdititing())
		{
			SetWidgetsState(false);
		}
		else
		{
			// show "Couldn't disable tilemask editing" message box
		}
	}
	else
	{
		if (activeScene->tilemaskEditorSystem->EnableLandscapeEditing())
		{
			SetWidgetsState(true);

			UpdateTileTextures();

			SetBrushSize(ui->sliderBrushSize->value());
			SetStrength(ui->sliderStrength->value());
			SetToolImage(ui->comboBrushImage->currentIndex());
			SetDrawTexture(ui->comboTileTexture->currentIndex());
		}
		else
		{
			QMessageBox::critical(0, "Error enabling Tile Mask Editor",
								  "Error enabling Tile Mask Editor.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}

	toolbarAction->setChecked(activeScene->tilemaskEditorSystem->IsLandscapeEditingEnabled());
}

void TilemaskEditorPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->tilemaskEditorSystem->SetBrushSize(brushSize);
}

void TilemaskEditorPropertiesView::SetToolImage(int imageIndex)
{
	if (!activeScene)
	{
		return;
	}

	QString s = ui->comboBrushImage->itemData(imageIndex).toString();

	if (!s.isEmpty())
	{
		FilePath fp(s.toStdString());
		activeScene->tilemaskEditorSystem->SetToolImage(fp, imageIndex);
	}
}

void TilemaskEditorPropertiesView::SetStrength(int strength)
{
	if (!activeScene)
	{
		return;
	}

	float32 max = 2.f * ui->sliderStrength->maximum();
	activeScene->tilemaskEditorSystem->SetStrength(strength / max);
}

void TilemaskEditorPropertiesView::SetDrawTexture(int textureIndex)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->tilemaskEditorSystem->SetTileTexture(textureIndex);
}
