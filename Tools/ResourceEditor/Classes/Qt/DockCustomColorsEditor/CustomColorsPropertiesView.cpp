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



#include "CustomColorsPropertiesView.h"
#include "ui_CustomColorsProperties.h"

#include "Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "../SceneEditor/EditorConfig.h"
#include "../Scene/SceneSignals.h"

#include <QFileDialog>
#include <QMessageBox>

CustomColorsPropertiesView::CustomColorsPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::CustomColorsPropertiesView)
,	activeScene(NULL)
{
	ui->setupUi(this);

	Init();
}

CustomColorsPropertiesView::~CustomColorsPropertiesView()
{
	delete ui;
}

void CustomColorsPropertiesView::Init()
{
	connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));

	connect(ui->buttonEnableCustomColorsEditor, SIGNAL(clicked()), this, SLOT(Toggle()));
	connect(SceneSignals::Instance(), SIGNAL(CustomColorsTextureShouldBeSaved(SceneEditor2*)),
			this, SLOT(NeedSaveCustomColorsTexture(SceneEditor2*)));

	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->comboColor, SIGNAL(currentIndexChanged(int)), this, SLOT(SetColor(int)));
	connect(ui->buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(ui->buttonLoadTexture, SIGNAL(clicked()), this, SLOT(LoadTexture()));

	SetWidgetsState(false);
}

void CustomColorsPropertiesView::InitColors()
{
	QSize iconSize = ui->comboColor->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	ui->comboColor->setIconSize(iconSize);

	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues("LandscapeCustomColorsDescription");
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);

		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);

		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);

		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		ui->comboColor->addItem(icon, description.c_str());
	}
}

void CustomColorsPropertiesView::ProjectOpened(const QString& path)
{
	InitColors();
}

void CustomColorsPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->customColorsSystem->IsLandscapeEditingEnabled())
	{
		if (activeScene->customColorsSystem->DisableLandscapeEdititing())
		{
			SetWidgetsState(false);
		}
		else
		{
			// show "Couldn't disable custom colors editing" message box
		}
	}
	else
	{
		if (activeScene->customColorsSystem->EnableLandscapeEditing())
		{
			SetWidgetsState(true);

			SetBrushSize(ui->sliderBrushSize->value());
			SetColor(ui->comboColor->currentIndex());
		}
		else
		{
			QMessageBox::critical(0, "Error enabling Custom Colors editor", "Error enabling Custom Colors editor.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}
}

void CustomColorsPropertiesView::SetWidgetsState(bool enabled)
{
	ui->buttonEnableCustomColorsEditor->blockSignals(true);
	ui->buttonEnableCustomColorsEditor->setCheckable(enabled);
	ui->buttonEnableCustomColorsEditor->setChecked(enabled);
	ui->buttonEnableCustomColorsEditor->blockSignals(false);

	QString buttonText = enabled ? tr("Disable Custom Colors Editor") : tr("Enable Custom Colors Editor");
	ui->buttonEnableCustomColorsEditor->setText(buttonText);

	ui->buttonSaveTexture->setEnabled(enabled);
	ui->sliderBrushSize->setEnabled(enabled);
	ui->comboColor->setEnabled(enabled);
	ui->buttonLoadTexture->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void CustomColorsPropertiesView::BlockAllSignals(bool block)
{
	ui->buttonSaveTexture->blockSignals(block);
	ui->sliderBrushSize->blockSignals(block);
	ui->comboColor->blockSignals(block);
	ui->buttonLoadTexture->blockSignals(block);
}

void CustomColorsPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->customColorsSystem->IsLandscapeEditingEnabled();
	int32 brushSize = scene->customColorsSystem->GetBrushSize();
	int32 colorIndex = scene->customColorsSystem->GetColor();

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderBrushSize->setValue(brushSize);
	ui->comboColor->setCurrentIndex(colorIndex);
	BlockAllSignals(!enabled);
}

void CustomColorsPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void CustomColorsPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	activeScene = NULL;
}

void CustomColorsPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->customColorsSystem->SetBrushSize(brushSize);
}

void CustomColorsPropertiesView::SetColor(int color)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->customColorsSystem->SetColor(color);
}

void CustomColorsPropertiesView::SaveTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath selectedPathname = activeScene->customColorsSystem->GetCurrentSaveFileName();
	if (selectedPathname.IsEmpty())
	{
		selectedPathname = activeScene->GetScenePath().GetDirectory();
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save texture"),
													QString(selectedPathname.GetAbsolutePathname().c_str()),
													QString("PNG image (*.png)"));
	selectedPathname = PathnameToDAVAStyle(filePath);

	if (!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->SaveTexture(selectedPathname);
	}
}

void CustomColorsPropertiesView::LoadTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath currentPath = activeScene->customColorsSystem->GetCurrentSaveFileName();
	if (currentPath.IsEmpty())
	{
		currentPath = activeScene->GetScenePath().GetDirectory();
	}

	FilePath selectedPathname = GetOpenFileName(String("Load texture"), currentPath, String("PNG image (*.png)"));
	if(!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->LoadTexture(selectedPathname);
	}
}

void CustomColorsPropertiesView::NeedSaveCustomColorsTexture(SceneEditor2* scene)
{
	if (!activeScene)
	{
		return;
	}

	FilePath selectedPathname = scene->customColorsSystem->GetCurrentSaveFileName();
	if(!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->SaveTexture(selectedPathname);
	}
	else
	{
		SaveTexture();
	}
}
