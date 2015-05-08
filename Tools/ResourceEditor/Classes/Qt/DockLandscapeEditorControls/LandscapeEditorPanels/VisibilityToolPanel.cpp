#include "VisibilityToolPanel.h"
#include "../../Scene/SceneSignals.h"
#include "../../Scene/SceneEditor2.h"
#include "Tools/SliderWidget/SliderWidget.h"
#include "Constants.h"
#include "Main/QtUtils.h"
#include "../LandscapeEditorShortcutManager.h"

#include "QtTools/FileDialog/FileDialog.h"

#include <QLayout>
#include <QPushButton>
#include <QLabel>

VisibilityToolPanel::VisibilityToolPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
,	buttonSetVisibilityPoint(NULL)
,	buttonSetVisibilityArea(NULL)
,	buttonSaveTexture(NULL)
,	sliderWidgetAreaSize(NULL)
{
	InitUI();
	ConnectToSignals();
}

VisibilityToolPanel::~VisibilityToolPanel()
{
}

bool VisibilityToolPanel::GetEditorEnabled()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	if (!sceneEditor)
	{
		return false;
	}

	return sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
}

void VisibilityToolPanel::SetWidgetsState(bool enabled)
{
	buttonSetVisibilityPoint->setEnabled(enabled);
	buttonSetVisibilityArea->setEnabled(enabled);
	buttonSaveTexture->setEnabled(enabled);
	sliderWidgetAreaSize->setEnabled(enabled);
}

void VisibilityToolPanel::BlockAllSignals(bool block)
{
	buttonSetVisibilityPoint->blockSignals(block);
	buttonSetVisibilityArea->blockSignals(block);
	buttonSaveTexture->blockSignals(block);
	sliderWidgetAreaSize->blockSignals(block);
}

void VisibilityToolPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	buttonSetVisibilityPoint = new QPushButton(this);
	buttonSetVisibilityArea = new QPushButton(this);
	buttonSaveTexture = new QPushButton(this);
	sliderWidgetAreaSize = new SliderWidget(this);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout* layoutBrushSize = new QVBoxLayout();
	QLabel* labelBrushSize = new QLabel();
	labelBrushSize->setText(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_CAPTION.c_str());
	layoutBrushSize->addWidget(labelBrushSize);
	layoutBrushSize->addWidget(sliderWidgetAreaSize);

	layout->addWidget(buttonSetVisibilityPoint);
	layout->addLayout(layoutBrushSize);
	layout->addWidget(buttonSetVisibilityArea);
	layout->addWidget(buttonSaveTexture);
	layout->addSpacerItem(spacer);

	setLayout(layout);

	SetWidgetsState(false);
	BlockAllSignals(true);

	sliderWidgetAreaSize->Init(false, DEF_AREA_MAX_SIZE, DEF_AREA_MIN_SIZE, DEF_AREA_MIN_SIZE);
	sliderWidgetAreaSize->SetRangeBoundaries(ResourceEditor::BRUSH_MIN_BOUNDARY, ResourceEditor::BRUSH_MAX_BOUNDARY);
	buttonSetVisibilityPoint->setText(ResourceEditor::VISIBILITY_TOOL_SET_POINT_CAPTION.c_str());
	buttonSetVisibilityPoint->setCheckable(true);
	buttonSetVisibilityArea->setText(ResourceEditor::VISIBILITY_TOOL_SET_AREA_CAPTION.c_str());
	buttonSetVisibilityArea->setCheckable(true);
	buttonSaveTexture->setText(ResourceEditor::VISIBILITY_TOOL_SAVE_TEXTURE_CAPTION.c_str());
}

void VisibilityToolPanel::ConnectToSignals()
{
	connect(SceneSignals::Instance(),
			SIGNAL(VisibilityToolStateChanged(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)),
			this,
			SLOT(SetVisibilityToolButtonsState(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)));
	connect(SceneSignals::Instance(), SIGNAL(VisibilityToolToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(buttonSetVisibilityPoint, SIGNAL(clicked()), this, SLOT(SetVisibilityPoint()));
	connect(buttonSetVisibilityArea, SIGNAL(clicked()), this, SLOT(SetVisibilityArea()));
	connect(sliderWidgetAreaSize, SIGNAL(ValueChanged(int)), this, SLOT(SetVisibilityAreaSize(int)));
}

void VisibilityToolPanel::StoreState()
{
	KeyedArchive* customProperties = GetOrCreateCustomProperties(GetActiveScene())->GetArchive();
    customProperties->SetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MIN,
                               sliderWidgetAreaSize->GetRangeMin());
    customProperties->SetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MAX,
                               sliderWidgetAreaSize->GetRangeMax());
}

void VisibilityToolPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();

	bool enabled = sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
	int32 brushSize = AreaSizeSystemToUI(sceneEditor->visibilityToolSystem->GetBrushSize());
	VisibilityToolSystem::eVisibilityToolState state = sceneEditor->visibilityToolSystem->GetState();

	int32 areaSizeMin = DEF_AREA_MIN_SIZE;
	int32 areaSizeMax = DEF_AREA_MAX_SIZE;

	KeyedArchive* customProperties = GetCustomPropertiesArchieve(sceneEditor);
	if (customProperties)
	{
		areaSizeMin = customProperties->GetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MIN, DEF_AREA_MIN_SIZE);
		areaSizeMax = customProperties->GetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MAX, DEF_AREA_MAX_SIZE);
	}
	
	SetWidgetsState(enabled);
	
	BlockAllSignals(true);
	sliderWidgetAreaSize->SetRangeMin(areaSizeMin);
	sliderWidgetAreaSize->SetRangeMax(areaSizeMax);
	sliderWidgetAreaSize->SetValue(brushSize);
	buttonSetVisibilityPoint->setChecked(state == VisibilityToolSystem::VT_STATE_SET_POINT);
	buttonSetVisibilityArea->setChecked(state == VisibilityToolSystem::VT_STATE_SET_AREA);
	BlockAllSignals(!enabled);
}

// these functions are designed to convert values from sliders in ui
// to the values suitable for visibility tool system
int32 VisibilityToolPanel::AreaSizeUIToSystem(int32 uiValue)
{
	int32 systemValue = uiValue * ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return systemValue;
}

int32 VisibilityToolPanel::AreaSizeSystemToUI(int32 systemValue)
{
	int32 uiValue = systemValue / ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return uiValue;
}
// end of convert functions ==========================

void VisibilityToolPanel::SetVisibilityToolButtonsState(SceneEditor2* scene,
														VisibilityToolSystem::eVisibilityToolState state)
{
	if (scene != GetActiveScene())
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

	b = buttonSetVisibilityPoint->signalsBlocked();
	buttonSetVisibilityPoint->blockSignals(true);
	buttonSetVisibilityPoint->setChecked(pointButton);
	buttonSetVisibilityPoint->blockSignals(b);
	
	b = buttonSetVisibilityArea->signalsBlocked();
	buttonSetVisibilityArea->blockSignals(true);
	buttonSetVisibilityArea->setChecked(areaButton);
	buttonSetVisibilityArea->blockSignals(b);
}

void VisibilityToolPanel::SaveTexture()
{
	FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = FileDialog::getSaveFileName(NULL,
													QString(ResourceEditor::VISIBILITY_TOOL_SAVE_CAPTION.c_str()),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString(ResourceEditor::VISIBILITY_TOOL_FILE_FILTER.c_str()));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		GetActiveScene()->visibilityToolSystem->SaveTexture(selectedPathname);
	}
}

void VisibilityToolPanel::SetVisibilityPoint()
{
	GetActiveScene()->visibilityToolSystem->SetVisibilityPoint();
}

void VisibilityToolPanel::SetVisibilityArea()
{
	GetActiveScene()->visibilityToolSystem->SetVisibilityArea();
}

void VisibilityToolPanel::SetVisibilityAreaSize(int areaSize)
{
	GetActiveScene()->visibilityToolSystem->SetBrushSize(AreaSizeUIToSystem(areaSize));
}

void VisibilityToolPanel::ConnectToShortcuts()
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

	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT), SIGNAL(activated()),
			this, SLOT(SetVisibilityPoint()));
	connect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA), SIGNAL(activated()),
			this, SLOT(SetVisibilityArea()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(true);
	shortcutManager->SetVisibilityToolShortcutsEnabled(true);
}

void VisibilityToolPanel::DisconnectFromShortcuts()
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

	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_POINT), SIGNAL(activated()),
			   this, SLOT(SetVisibilityPoint()));
	disconnect(shortcutManager->GetShortcutByName(ResourceEditor::SHORTCUT_VISIBILITY_TOOL_SET_AREA), SIGNAL(activated()),
			   this, SLOT(SetVisibilityArea()));
	
	shortcutManager->SetBrushSizeShortcutsEnabled(false);
	shortcutManager->SetVisibilityToolShortcutsEnabled(false);
}

void VisibilityToolPanel::IncreaseBrushSize()
{
	sliderWidgetAreaSize->SetValue(sliderWidgetAreaSize->GetValue()
								   + ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void VisibilityToolPanel::DecreaseBrushSize()
{
	sliderWidgetAreaSize->SetValue(sliderWidgetAreaSize->GetValue()
								   - ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_SMALL);
}

void VisibilityToolPanel::IncreaseBrushSizeLarge()
{
	sliderWidgetAreaSize->SetValue(sliderWidgetAreaSize->GetValue()
								   + ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}

void VisibilityToolPanel::DecreaseBrushSizeLarge()
{
	sliderWidgetAreaSize->SetValue(sliderWidgetAreaSize->GetValue()
								   - ResourceEditor::SLIDER_WIDGET_CHANGE_VALUE_STEP_LARGE);
}
