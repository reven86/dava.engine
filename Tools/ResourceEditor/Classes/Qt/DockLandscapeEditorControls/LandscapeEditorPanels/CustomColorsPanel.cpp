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


#include "CustomColorsPanel.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Tools/SliderWidget/SliderWidget.h"
#include "Deprecated/EditorConfig.h"
#include "Project/ProjectManager.h"
#include "Constants.h"
#include "Main/QtUtils.h"
#include "Qt/DockLandscapeEditorControls/LandscapeEditorShortcutManager.h"
#include "QtTools/FileDialog/FileDialog.h"
#include "Tools/PathDescriptor/PathDescriptor.h"

#include <QLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QLabel>

CustomColorsPanel::CustomColorsPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
,	comboColor(NULL)
,	sliderWidgetBrushSize(NULL)
,	buttonSaveTexture(NULL)
,	buttonLoadTexture(NULL)
{
	InitUI();
	ConnectToSignals();
    InitColors();
}

CustomColorsPanel::~CustomColorsPanel()
{
}

bool CustomColorsPanel::GetEditorEnabled()
{
	return GetActiveScene()->customColorsSystem->IsLandscapeEditingEnabled();
}

void CustomColorsPanel::SetWidgetsState(bool enabled)
{
	comboColor->setEnabled(enabled);
	sliderWidgetBrushSize->setEnabled(enabled);
	buttonSaveTexture->setEnabled(enabled);
	buttonLoadTexture->setEnabled(enabled);
}

void CustomColorsPanel::BlockAllSignals(bool block)
{
	comboColor->blockSignals(block);
	sliderWidgetBrushSize->blockSignals(block);
	buttonSaveTexture->blockSignals(block);
	buttonLoadTexture->blockSignals(block);
}

void CustomColorsPanel::StoreState()
{
	KeyedArchive* customProperties = GetOrCreateCustomProperties(GetActiveScene())->GetArchive();
    customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN, sliderWidgetBrushSize->GetRangeMin());
    customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX, sliderWidgetBrushSize->GetRangeMax());
}

void CustomColorsPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	bool enabled = sceneEditor->customColorsSystem->IsLandscapeEditingEnabled();
	int32 brushSize = BrushSizeSystemToUI(sceneEditor->customColorsSystem->GetBrushSize());
	int32 colorIndex = sceneEditor->customColorsSystem->GetColor();
	
	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;
	
	KeyedArchive* customProperties = GetCustomPropertiesArchieve(sceneEditor);
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN, DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX, DEF_BRUSH_MAX_SIZE);
	}
	
	SetWidgetsState(enabled);
	
	BlockAllSignals(true);
	sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	sliderWidgetBrushSize->SetValue(brushSize);
	comboColor->setCurrentIndex(colorIndex);
	BlockAllSignals(!enabled);
}

void CustomColorsPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	
	comboColor = new QComboBox(this);
	sliderWidgetBrushSize = new SliderWidget(this);
	buttonSaveTexture = new QPushButton(this);
	buttonLoadTexture = new QPushButton(this);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	QHBoxLayout* layoutBrushSize = new QHBoxLayout();
	QLabel* labelBrushSize = new QLabel();
	labelBrushSize->setText(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_CAPTION.c_str());
	layoutBrushSize->addWidget(labelBrushSize);
	layoutBrushSize->addWidget(sliderWidgetBrushSize);

	layout->addWidget(comboColor);
	layout->addLayout(layoutBrushSize);
	layout->addWidget(buttonLoadTexture);
	layout->addWidget(buttonSaveTexture);
	layout->addSpacerItem(spacer);
	
	setLayout(layout);
	
	SetWidgetsState(false);
	BlockAllSignals(true);
	
	sliderWidgetBrushSize->Init(false, DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);
	sliderWidgetBrushSize->SetRangeBoundaries(ResourceEditor::BRUSH_MIN_BOUNDARY, ResourceEditor::BRUSH_MAX_BOUNDARY);
	buttonSaveTexture->setText(ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str());
	buttonLoadTexture->setText(ResourceEditor::CUSTOM_COLORS_LOAD_CAPTION.c_str());
}

void CustomColorsPanel::ConnectToSignals()
{
	connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	connect(SceneSignals::Instance(), SIGNAL(CustomColorsTextureShouldBeSaved(SceneEditor2*)),
			this, SLOT(SaveTextureIfNeeded(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(CustomColorsToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(comboColor, SIGNAL(currentIndexChanged(int)), this, SLOT(SetColor(int)));
	connect(buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(buttonLoadTexture, SIGNAL(clicked()), this, SLOT(LoadTexture()));
}

void CustomColorsPanel::ProjectOpened(const QString &path)
{
	InitColors();
}

void CustomColorsPanel::InitColors()
{
	comboColor->clear();
	
	QSize iconSize = comboColor->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	comboColor->setIconSize(iconSize);
	
	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_COLORS);
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_DESCRIPTION);
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);
		
		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);
		
		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);
		
		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		comboColor->addItem(icon, description.c_str());
	}
}

// these functions are designed to convert values from sliders in ui
// to the values suitable for custom colors system
int32 CustomColorsPanel::BrushSizeUIToSystem(int32 uiValue)
{
	int32 systemValue = uiValue * ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return systemValue;
}

int32 CustomColorsPanel::BrushSizeSystemToUI(int32 systemValue)
{
	int32 uiValue = systemValue / ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return uiValue;
}
// end of convert functions ==========================

void CustomColorsPanel::SetBrushSize(int brushSize)
{
	GetActiveScene()->customColorsSystem->SetBrushSize(BrushSizeUIToSystem(brushSize));
}

void CustomColorsPanel::SetColor(int color)
{
	GetActiveScene()->customColorsSystem->SetColor(color);
}

bool CustomColorsPanel::SaveTexture()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	FilePath selectedPathname = sceneEditor->customColorsSystem->GetCurrentSaveFileName();
    if (!FileSystem::Instance()->Exists(selectedPathname))
    {
		selectedPathname = sceneEditor->GetScenePath().GetDirectory();
	}
	
    const QString text = "Custom colors texture is not saved. Do you want to save it?";
    QString filePath;
    for ( ;; )
    {
        filePath = FileDialog::getSaveFileName( NULL, QString( ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str() ),
            QString( selectedPathname.GetAbsolutePathname().c_str() ),
            PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter);

        if ( filePath.isEmpty() )
        {
            QMessageBox::StandardButton result = QMessageBox::question( NULL, "Save changes", text );
            if ( result == QMessageBox::Yes )
            {
                continue;
            }
        }
        break;
    }

    selectedPathname = PathnameToDAVAStyle( filePath );
	
	if (selectedPathname.IsEmpty())
	{
		return false;
	}
	
	sceneEditor->customColorsSystem->SaveTexture(selectedPathname);
	return true;
}

void CustomColorsPanel::LoadTexture()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	FilePath currentPath = sceneEditor->customColorsSystem->GetCurrentSaveFileName();

    if (!FileSystem::Instance()->Exists(currentPath))
    {
		currentPath = sceneEditor->GetScenePath().GetDirectory();
	}
	
	FilePath selectedPathname = GetOpenFileName(ResourceEditor::CUSTOM_COLORS_LOAD_CAPTION,
												currentPath,
												PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter.toStdString());
	if(!selectedPathname.IsEmpty())
	{
		sceneEditor->customColorsSystem->LoadTexture(selectedPathname);
	}
}

void CustomColorsPanel::SaveTextureIfNeeded(SceneEditor2* scene)
{
	if (scene != GetActiveScene())
	{
		return;
	}

	FilePath selectedPathname = scene->customColorsSystem->GetCurrentSaveFileName();
	if(!selectedPathname.IsEmpty())
	{
		scene->customColorsSystem->SaveTexture(selectedPathname);
	}
	else
	{
		SaveTexture();
	}
}

void CustomColorsPanel::ConnectToShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSize()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			this, SLOT(IncreaseBrushSizeLarge()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			this, SLOT(DecreaseBrushSizeLarge()));

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_NEXT), SIGNAL(activated()),
			this, SLOT(NextTexture()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_PREV), SIGNAL(activated()),
			this, SLOT(PrevTexture()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(true);
	shortcutManager->SetTextureSwitchingShortcutsEnabled(true);
}

void CustomColorsPanel::DisconnectFromShortcuts()
{
	LandscapeEditorShortcutManager* shortcutManager = LandscapeEditorShortcutManager::Instance();

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(IncreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_SMALL), SIGNAL(activated()),
			   this, SLOT(DecreaseBrushSize()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_INCREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(IncreaseBrushSizeLarge()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_BRUSH_SIZE_DECREASE_LARGE), SIGNAL(activated()),
			   this, SLOT(DecreaseBrushSizeLarge()));

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_NEXT), SIGNAL(activated()),
			   this, SLOT(NextTexture()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_TEXTURE_PREV), SIGNAL(activated()),
			   this, SLOT(PrevTexture()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(false);
	shortcutManager->SetTextureSwitchingShortcutsEnabled(false);
}

void CustomColorsPanel::IncreaseBrushSize()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void CustomColorsPanel::DecreaseBrushSize()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void CustomColorsPanel::IncreaseBrushSizeLarge()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									+ ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void CustomColorsPanel::DecreaseBrushSizeLarge()
{
	sliderWidgetBrushSize->SetValue(sliderWidgetBrushSize->GetValue()
									- ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void CustomColorsPanel::PrevTexture()
{
	int32 curIndex = comboColor->currentIndex();
	if (curIndex)
	{
		comboColor->setCurrentIndex(curIndex - 1);
	}
}

void CustomColorsPanel::NextTexture()
{
	int32 curIndex = comboColor->currentIndex();
	if (curIndex < comboColor->count() - 1)
	{
		comboColor->setCurrentIndex(curIndex + 1);
	}
}
