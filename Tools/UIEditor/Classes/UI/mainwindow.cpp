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


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"

#include "HierarchyTreeController.h"
#include "rectpropertygridwidget.h"
#include "createplatformdlg.h"
#include "createplatformdlg.h"
#include "createscreendlg.h"
#include "fontmanagerdialog.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"

#include "CommandsController.h"
#include "ItemsCommand.h"
#include "GuideCommands.h"

#include "Dialogs/createaggregatordlg.h"
#include "Dialogs/importdialog.h"
#include "Dialogs/importdialog.h"
#include "Dialogs/localizationeditordialog.h"
#include "Dialogs/previewsettingsdialog.h"
#include "Dialogs/errorslistdialog.h"

#include "ImportCommands.h"
#include "AlignDistribute/AlignDistributeEnums.h"
#include "DefaultScreen.h"
#include "ColorHelper.h"

#include "Grid/GridVisualizer.h"

#include "Ruler/RulerController.h"

#include "EditorFontManager.h"
#include "CopyPasteController.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QColorDialog>
#include <QDateTime>

#define SPIN_SCALE 10.f

const QString APP_NAME = "UIEditor";
const QString APP_COMPANY = "DAVA";
const QString APP_GEOMETRY = "geometry";
const QString APP_STATE = "windowstate";

static const int SCALE_PERCENTAGES[] =
{
	10,   25,   50,  75,
	100,  200,  400, 800,
	1200, 1600, 3200
};

static const int32 DEFAULT_SCALE_PERCENTAGE_INDEX = 4; // 100%
static const char* PERCENTAGE_FORMAT = "%1 %";

static const char* COLOR_PROPERTY_ID = "color";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    backgroundFrameUseCustomColorAction(NULL),
    backgroundFrameSelectCustomColorAction(NULL)
{
	screenChangeUpdate = false;
	
    ui->setupUi(this);
    
	ui->davaGlWidget->setFocus();
    connect(ui->davaGlWidget, SIGNAL(DavaGLWidgetResized()), this, SLOT(OnGLWidgetResized()));

	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());

    findField = new QLineEdit();
    findField->setValidator(new QRegExpValidator(HierarchyTreeNode::GetNameRegExp(), this));
    ui->findToolBar->addWidget(findField);
    ui->findToolBar->addSeparator();
    connect(findField, SIGNAL(returnPressed() ), this, SLOT(OnSearchPressed()));
    connect(ui->actionFind,SIGNAL(triggered()),this, SLOT(OnSearchPressed()));
    
	int32 scalesCount = COUNT_OF(SCALE_PERCENTAGES);

	// Setup the Scale Slider.
	ui->scaleSlider->setTickInterval(1);
	ui->scaleSlider->setMinimum(0);
	ui->scaleSlider->setMaximum(scalesCount - 1);
	ui->scaleSlider->setValue(DEFAULT_SCALE_PERCENTAGE_INDEX);
	connect(ui->scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(OnScaleSliderValueChanged(int)));

	// Setup the Scale Combo.
	for (int32 i = 0; i < scalesCount; i ++)
	{
		ui->scaleCombo->addItem(QString(PERCENTAGE_FORMAT).arg(SCALE_PERCENTAGES[i]));
	}

	ui->scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
	ui->scaleCombo->lineEdit()->setMaxLength(6);
	ui->scaleCombo->setInsertPolicy(QComboBox::NoInsert);
	connect(ui->scaleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnScaleComboIndexChanged(int)));
	connect(ui->scaleCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(OnScaleComboTextEditingFinished()));

	connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));
	connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));

    // Setup rulers.
    ui->horizontalRuler->SetOrientation(RulerWidget::Horizontal);
    ui->verticalRuler->SetOrientation(RulerWidget::Vertical);

    connect(RulerController::Instance(),
            SIGNAL(HorisontalRulerSettingsChanged(const RulerSettings &)),
            ui->horizontalRuler,
            SLOT(OnRulerSettingsChanged(const RulerSettings &)));
    connect(RulerController::Instance(),
            SIGNAL(VerticalRulerSettingsChanged(const RulerSettings &)),
            ui->verticalRuler,
            SLOT(OnRulerSettingsChanged(const RulerSettings &)));
    
    connect(RulerController::Instance(),
            SIGNAL(HorisontalRulerMarkPositionChanged(int)),
            ui->horizontalRuler,
            SLOT(OnMarkerPositionChanged(int)));
    connect(RulerController::Instance(),
            SIGNAL(VerticalRulerMarkPositionChanged(int)),
            ui->verticalRuler,
            SLOT(OnMarkerPositionChanged(int)));
    
    RulerController::Instance()->UpdateRulers();

    connect(ui->actionFontManager, SIGNAL(triggered()), this, SLOT(OnOpenFontManager()));
    connect(ui->actionLocalizationManager, SIGNAL(triggered()), this, SLOT(OnOpenLocalizationManager()));

	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectCreated()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectClosed()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectLoaded()),
			this,
			SLOT(OnProjectLoaded()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedScreenChanged(const HierarchyTreeScreenNode*)),
			this,
			SLOT(OnSelectedScreenChanged()));
	
    connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &, HierarchyTreeController::eExpandControlType)),
			this,
			SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &,HierarchyTreeController::eExpandControlType)));

	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(CreateNewScreen()),
			this,
			SLOT(OnNewScreen()));
	
	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(CreateNewAggregator()),
			this,
			SLOT(OnNewAggregator()));

	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(ImportScreenOrAggregator()),
			this,
			SLOT(OnImportScreenOrAggregator()));

    connect(ui->previewSettingsDockWidget->widget(),
            SIGNAL(PreviewModeChanged(int)),
            this,
            SLOT(OnPreviewModeChanged(int)));

	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScaleRequest(float)),
			this,
			SLOT(OnUpdateScaleRequest(float)));
	
	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScreenPositionRequest(const QPoint&)),
			this,
			SLOT(OnUpdateScreenPositionRequest(const QPoint&)));
	
	this->ui->actionUndo->setEnabled(false);
	this->ui->actionRedo->setEnabled(false);
	connect(CommandsController::Instance(),
			SIGNAL(UndoRedoAvailabilityChanged()),
			this,
			SLOT(OnUndoRedoAvailabilityChanged()));
	
	connect(CommandsController::Instance(),
			SIGNAL(UndoRedoAvailabilityChanged()),
			this,
			SLOT(OnChangePropertySucceeded()));

	connect(CommandsController::Instance(),
			SIGNAL(UnsavedChangesNumberChanged()),
			this,
			SLOT(OnUnsavedChangesNumberChanged()));

    connect(this->ui->actionPreview,
            SIGNAL(triggered()),
            this,
            SLOT(OnPreviewTriggered()));
    
    connect(this->ui->actionScreenshot,
            SIGNAL(triggered()),
            this,
            SLOT(OnScreenshot()));

    connect(this->ui->actionSetScreenshotFolder,
            SIGNAL(triggered()),
            this,
            SLOT(OnSetScreenshotFolder()));
    
    connect(this->ui->actionEditPreviewSettings,
            SIGNAL(triggered()),
            this,
            SLOT(OnEditPreviewSettings()));

    connect(this->ui->horizontalRuler,
            SIGNAL(GuideDropped(Qt::DropAction)),
            this,
            SLOT(OnGuideDropped(Qt::DropAction)));

    connect(this->ui->verticalRuler,
            SIGNAL(GuideDropped(Qt::DropAction)),
            this,
            SLOT(OnGuideDropped(Qt::DropAction)));

    DefaultScreen* defaultScreen = ScreenWrapper::Instance()->GetActiveScreen();
    connect(defaultScreen, SIGNAL(DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST&)),
            this->ui->hierarchyDockWidgetContents, SLOT(OnDeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST&)));

	InitMenu();
	RestoreMainWindowState();
	CreateHierarchyDockWidgetToolbar();
    UpdatePreviewButton();

    SetAlignEnabled(false);
    SetDistributeEnabled(false);
}

MainWindow::~MainWindow()
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        SafeDelete(recentPojectActions[i]);
    }
	
	SaveMainWindowState();
	delete findField;
    delete ui;
}

void MainWindow::SaveMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
    settings.setValue(APP_GEOMETRY, saveGeometry());
    settings.setValue(APP_STATE, saveState());
}

void MainWindow::RestoreMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
	// Check settings befor applying it
	if (!settings.value(APP_GEOMETRY).isNull() && settings.value(APP_GEOMETRY).isValid())
	{
    	restoreGeometry(settings.value(APP_GEOMETRY).toByteArray());
	}
	if (!settings.value(APP_STATE).isNull() && settings.value(APP_STATE).isValid())
	{
    	restoreState(settings.value(APP_STATE).toByteArray());
	}
}

void MainWindow::CreateHierarchyDockWidgetToolbar()
{
	QMainWindow *window = new QMainWindow(0);
 	QToolBar *toolBar = new QToolBar(window);
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    
	// Change size of icons to 16x16 (default is 32x32)
	toolBar->setIconSize(QSize(16, 16));
	// Set actions for toolbar
 	toolBar->addAction(ui->actionNew_platform);
	toolBar->addAction(ui->actionNew_screen);
	toolBar->addAction(ui->actionNew_aggregator);
	toolBar->addAction(ui->actionImport_Platform);
	// Disable moving of toolbar
	toolBar->setMovable(false);
	// Set toolbar position
	window->setCentralWidget(ui->hierarchyDockWidgetContents);
	window->addToolBar(toolBar);
	window->setParent(ui->hierarchyDockWidget);
	// link toolbar to hierarchyDockWidget
 	ui->hierarchyDockWidget->setWidget(window);
}

void MainWindow::UpdateScaleControls()
{
	float32 newScale = ScreenWrapper::Instance()->GetScale();
	float32 scaleInPercents = ScreenWrapper::Instance()->GetScale() * 100;

	float32 minDistance = FLT_MAX;
	int32 nearestScaleIndex = 0;

	// Setup the nearest scale index.
	if (scaleInPercents > SCALE_PERCENTAGES[0])
	{
		for (uint32 i = 0; i < COUNT_OF(SCALE_PERCENTAGES); i ++)
		{
			float32 curDistance = fabsf(scaleInPercents - SCALE_PERCENTAGES[i]);
			if (curDistance < minDistance)
			{
				minDistance = curDistance;
				nearestScaleIndex = i;
			}
		}
	}
	else
	{
		minDistance = 0.0f;
	}

	// Done, perform update.
	int32 scaleValue = SCALE_PERCENTAGES[nearestScaleIndex];
	UpdateScaleSlider(scaleValue);
	
	if (FLOAT_EQUAL(minDistance, 0.0f))
	{
		// Exact match.
		UpdateScaleComboIndex(nearestScaleIndex);
	}
	else
	{
		// Set the current scale as-is.
		this->ui->scaleCombo->setEditText(QString("%1 %").arg(scaleInPercents));
	}

    // Re-update the scale on the Screen Wrapper level to sync everything.
    NotifyScaleUpdated(newScale);
}

void MainWindow::OnUpdateScaleRequest(float scaleDelta)
{
    // TODO! temporary solution to don't add new class variable, should be refactored
    // together with the new Scale Control.
    if (!ui->scaleSlider->isVisible())
    {
        return;
    }
    
	// Lookup for the next/prev index.
	int32 curScaleIndex = ui->scaleSlider->value();
	if (scaleDelta > 0)
	{
		curScaleIndex ++;
		int32 maxIndex = (int32)(COUNT_OF(SCALE_PERCENTAGES) - 1);
		if (curScaleIndex > maxIndex)
		{
			curScaleIndex = maxIndex;
		}
	}
	else
	{
		curScaleIndex --;
		if (curScaleIndex < 0)
		{
			curScaleIndex = 0;
		}
	}

	UpdateScaleAndScaleSliderByIndex(curScaleIndex);
	UpdateScaleComboIndex(curScaleIndex);
}

void MainWindow::OnScaleComboIndexChanged(int value)
{
	UpdateScaleAndScaleSliderByIndex(value);
}

void MainWindow::OnScaleComboTextEditingFinished()
{
    // TODO! temporary solution to don't add new class variable, should be refactored
    // together with the new Scale Control.
    if (!ui->scaleCombo->isVisible())
    {
        return;
    }

	// Firstly verify whether the value is already set.
	QString curTextValue = ui->scaleCombo->currentText().trimmed();
	int scaleValue = 0;
	if (curTextValue.endsWith(" %"))
	{
		int endCharPos = curTextValue.lastIndexOf(" %");
		QString remainderNumber = curTextValue.left(endCharPos);
		scaleValue = remainderNumber.toInt();
	}
	else
	{
		// Try to parse the value.
		scaleValue = curTextValue.toFloat();
	}

	// Do the validation.
	bool needSetDefaultIndex = false;
	if ((scaleValue < SCALE_PERCENTAGES[0]) ||
		(scaleValue > SCALE_PERCENTAGES[COUNT_OF(SCALE_PERCENTAGES) - 1]))
	{
		// The value is wrong or can't be parsed, use the default one.
		scaleValue = SCALE_PERCENTAGES[DEFAULT_SCALE_PERCENTAGE_INDEX];
		needSetDefaultIndex = true;
	}

	// Update the value in the combo.
	ui->scaleCombo->blockSignals(true);
	ui->scaleCombo->lineEdit()->blockSignals(true);

	ui->scaleCombo->setEditText(QString(PERCENTAGE_FORMAT).arg((int)scaleValue));
	if (needSetDefaultIndex)
	{
		ui->scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
	}

	ui->scaleCombo->lineEdit()->blockSignals(false);
	ui->scaleCombo->blockSignals(false);

	// Update the rest of the controls.
	UpdateScale(scaleValue);
	UpdateScaleSlider(scaleValue);
}

void MainWindow::OnScaleSliderValueChanged(int value)
{
	int32 scaleValue = SCALE_PERCENTAGES[value];
	UpdateScale(scaleValue);
	UpdateScaleComboIndex(value);
}

void MainWindow::UpdateScaleAndScaleSliderByIndex(int32 index)
{
	if (index < 0 || index >= (int)COUNT_OF(SCALE_PERCENTAGES))
	{
		return;
	}
	
	int32 scaleValue = SCALE_PERCENTAGES[index];
	UpdateScale(scaleValue);
	UpdateScaleSlider(scaleValue);
}

void MainWindow::UpdateScale(int32 newScalePercents)
{
	// Scale is sent in percents, so need to divide by 100.
	float prevScale = ScreenWrapper::Instance()->GetScale();
	float newScale = (float)newScalePercents / 100;
	
	if (FLOAT_EQUAL(prevScale, newScale))
	{
		return;
	}
	
	// Remember the current "absolute" view area position to position the same point
	// under mouse cursor after scale level will be changed.
	Vector2 cursorPos = ScreenWrapper::Instance()->GetCursorPosition();
	Vector2 scenePosition = CalculateScenePositionForPoint(ui->davaGlWidget->rect(), cursorPos, prevScale);

    NotifyScaleUpdated(newScale);

	UpdateSliders();
	UpdateScreenPosition();

	// Position back the view area to show the "scene position" remembered under mouse cursor.
	ScrollToScenePositionAndPoint(scenePosition, cursorPos, newScale);
}

void MainWindow::UpdateScaleComboIndex(int newIndex)
{
	if (newIndex < 0 || newIndex >= (int)COUNT_OF(SCALE_PERCENTAGES))
	{
		return;
	}

	ui->scaleCombo->blockSignals(true);
	ui->scaleCombo->setCurrentIndex(newIndex);
	ui->scaleCombo->blockSignals(false);
}

void MainWindow::UpdateScaleSlider(int32 newScalePercents)
{
	// Find the nearest value for the slider.
	int32 nearestDistance = INT_MAX;
	int32 nearestIndex = 0;

	int32 scalesCount = COUNT_OF(SCALE_PERCENTAGES);
	for (int32 i = 0; i < scalesCount; i ++)
	{
		int32 curDistance = abs(SCALE_PERCENTAGES[i] - newScalePercents);
		if (curDistance < nearestDistance)
		{
			nearestDistance = curDistance;
			nearestIndex = i;
		}
	}

	ui->scaleSlider->blockSignals(true);
	ui->scaleSlider->setValue(nearestIndex);
	ui->scaleSlider->blockSignals(false);
}

void MainWindow::OnSliderMoved()
{
	UpdateScreenPosition();
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
	// Need to reposition the center of the viewport according to the new window size.
	QSize widgetSize = ui->davaGlWidget->GetPrevSize();
	QRect oldWidgetRect(QPoint(0, 0), widgetSize);
	Vector2 oldViewPortCenter = Vector2(oldWidgetRect.center().x(), oldWidgetRect.center().y());

	float32 curScale = ScreenWrapper::Instance()->GetScale();
	Vector2 viewPortSceneCenter = CalculateScenePositionForPoint(oldWidgetRect, oldViewPortCenter, curScale);

	QMainWindow::resizeEvent(event);
	UpdateSliders();

	// Widget size is now changed - reposition center of viewport.
	widgetSize = ui->davaGlWidget->size();
	Vector2 newViewPortCenter = Vector2(widgetSize.width() / 2, widgetSize.height() / 2);

    UpdateScreenPosition();
	ScrollToScenePositionAndPoint(viewPortSceneCenter, newViewPortCenter, curScale);

	// Update the rulers to take new window size int account.
	RulerController::Instance()->UpdateRulers();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	// Ask user to save the project before closing.
	if (!CloseProject())
	{
		event->ignore();
	}
}

void MainWindow::showEvent(QShowEvent * event)
{
	QMainWindow::showEvent(event);
	
	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::OnSelectedScreenChanged()
{
	screenChangeUpdate = true;
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (activeScreen)
	{
		UpdateSliders();
		UpdateScaleControls();

		float posX = activeScreen->GetPosX();
		float posY = activeScreen->GetPosY();

		if (FLOAT_EQUAL(posX, HierarchyTreeScreenNode::POSITION_UNDEFINED) &&
			FLOAT_EQUAL(posY, HierarchyTreeScreenNode::POSITION_UNDEFINED))
		{
			// The screen was just loaded and wasn't positioned yet. Need to center it in the
			// screen according to the DF-1873.
			// Since sliders were just updated, just take their centers.
			posX = (ui->horizontalScrollBar->maximum() - ui->horizontalScrollBar->minimum()) / 2;
			posY = (ui->verticalScrollBar->maximum() - ui->verticalScrollBar->minimum()) / 2;
		}

		ui->horizontalScrollBar->setValue(posX);
		ui->verticalScrollBar->setValue(posY);

		// Enable library widget for selected screen
		ui->libraryDockWidget->setEnabled(true);
        
        ui->actionEnable_Guides->setChecked(activeScreen->AreGuidesEnabled());
        ui->actionLock_Guides->setChecked(activeScreen->AreGuidesLocked());
	}
	else
	{	// Disable library widget if no screen is selected
		ui->libraryDockWidget->setEnabled(false);
        ui->actionEnable_Guides->setEnabled(false);
	}
	
	screenChangeUpdate = false;
	UpdateMenu();
	UpdateScreenPosition();
    OnUndoRedoAvailabilityChanged();
}

void MainWindow::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES& selectedNodes, HierarchyTreeController::eExpandControlType expandType)
{
    int nodesCount = selectedNodes.size();

    SetAlignEnabled(nodesCount >= 2);
    SetDistributeEnabled(nodesCount >= 3);
}

void MainWindow::UpdateSliders()
{
	QRect widgetRect = ui->davaGlWidget->rect();
	QRect viewRect = ScreenWrapper::Instance()->GetRect();

	int valueV = viewRect.height() - widgetRect.height();
	if (valueV < 0) valueV = 0;
	int valueH = viewRect.width() - widgetRect.width();
	if (valueH < 0) valueH = 0;
	if (ui->verticalScrollBar->maximum() != valueV ||
		ui->horizontalScrollBar-> maximum() != valueH)
	{
		ui->verticalScrollBar->setMinimum(0);
		ui->verticalScrollBar->setMaximum(valueV);
		ui->horizontalScrollBar->setMinimum(0);
		ui->horizontalScrollBar->setMaximum(valueH);
	}
}

void MainWindow::UpdateScreenPosition()
{
	if (screenChangeUpdate)
		return;
	
	int valueV = ui->verticalScrollBar->value();
	int valueH = ui->horizontalScrollBar->value();
	ScreenWrapper::Instance()->SetViewPos(valueH, valueV, ui->davaGlWidget->rect());

    DefaultScreen* currentScreen = ScreenWrapper::Instance()->GetActiveScreen();
    if (currentScreen)
    {
        Vector2 viewPos = -currentScreen->GetPos();
        viewPos *= ScreenWrapper::Instance()->GetScale();
        RulerController::Instance()->SetViewPos(viewPos);
        
        // Recalculate the background frame position and size.
        Rect frameRect;
        frameRect.SetPosition(-currentScreen->GetPos());
        QRect glWidgetRect = ui->davaGlWidget->rect();
        frameRect.SetSize(Vector2(glWidgetRect.width(), glWidgetRect.height()));

        ScreenWrapper::Instance()->SetBackgroundFrameRect(frameRect);
        currentScreen->SetScreenPositionChangedFlag();
    }
}

void MainWindow::OnUpdateScreenPositionRequest(const QPoint& posDelta)
{
	ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() + posDelta.x());
	ui->verticalScrollBar->setValue(ui->verticalScrollBar->value() + posDelta.y());
}

void MainWindow::OnOpenFontManager()
{
    FontManagerDialog *fontManagerDialog = new FontManagerDialog();
    if (fontManagerDialog->exec())
    {
        delete fontManagerDialog;
    }
}

void MainWindow::OnOpenLocalizationManager()
{
    LocalizationEditorDialog *localizationManagerDialog = new LocalizationEditorDialog();
    if (localizationManagerDialog->exec())
    {
        delete localizationManagerDialog;
    }
}

void MainWindow::OnShowHelp()
{
	FilePath docsPath = ResourcesManageHelper::GetDocumentationPath().toStdString() + "index.html";
	QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
	QDesktopServices::openUrl(QUrl(docsFile));
}

void MainWindow::InitMenu()
{
    SetupViewMenu();

	connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(OnNewProject()));
	connect(ui->actionSave_project, SIGNAL(triggered()), this, SLOT(OnSaveProject()));
	connect(ui->actionSave_All, SIGNAL(triggered()), this, SLOT(OnSaveProjectAll()));
    connect(ui->actionOpen_project, SIGNAL(triggered()), this, SLOT(OnOpenProject()));
	connect(ui->actionClose_project, SIGNAL(triggered()), this, SLOT(OnCloseProject()));

	connect(ui->actionNew_platform, SIGNAL(triggered()), this, SLOT(OnNewPlatform()));
	connect(ui->actionNew_screen, SIGNAL(triggered()), this, SLOT(OnNewScreen()));
	connect(ui->actionNew_aggregator, SIGNAL(triggered()), this, SLOT(OnNewAggregator()));
	connect(ui->actionImport_Platform, SIGNAL(triggered()), this, SLOT(OnImportPlatform()));

	connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(OnExitApplication()));
	
	connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuFile, SIGNAL(triggered(QAction *)), this, SLOT(FileMenuTriggered(QAction *)));

    connect(ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(OnZoomInRequested()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(OnZoomOutRequested()));
	// Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
	QList<QKeySequence> shortcuts;
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Plus));
	ui->actionZoomIn->setShortcuts(shortcuts);
#endif
	//Create empty actions for recent projects files
	for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        recentPojectActions[i] = new QAction(this);
        recentPojectActions[i]->setObjectName(QString::fromStdString(Format("recentPojectActions[%d]", i)));
    }
	
	//Help contents dialog
    connect(ui->actionHelp, SIGNAL(triggered()), this, SLOT(OnShowHelp()));
	
	// Undo/Redo.
	connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(OnUndoRequested()));
	connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(OnRedoRequested()));
	
	// Adjust controls size
	connect(ui->actionAdjustControlSize, SIGNAL(triggered()), this, SLOT(OnAdjustSize()));

	// Align.
	connect(ui->actionAlign_Left, SIGNAL(triggered()), this, SLOT(OnAlignLeft()));
	connect(ui->actionAlign_Horz_Center, SIGNAL(triggered()), this, SLOT(OnAlignHorzCenter()));
	connect(ui->actionAlign_Right, SIGNAL(triggered()),this, SLOT(OnAlignRight()));

	connect(ui->actionAlign_Top, SIGNAL(triggered()), this, SLOT(OnAlignTop()));
	connect(ui->actionAlign_Vert_Center, SIGNAL(triggered()), this, SLOT(OnAlignVertCenter()));
	connect(ui->actionAlign_Bottom, SIGNAL(triggered()), this, SLOT(OnAlignBottom()));

	// Distribute.
	connect(ui->actionEqualBetweenLeftEdges , SIGNAL(triggered()), this, SLOT(OnDistributeEqualDistanceBetweenLeftEdges ()));
	connect(ui->actionEqualBetweenXCenters, SIGNAL(triggered()), this, SLOT(OnDistributeEqualDistanceBetweenXCenters()));
	connect(ui->actionEqualBetweenRightEdges, SIGNAL(triggered()),this, SLOT(OnDistributeEqualDistanceBetweenRightEdges()));
	connect(ui->actionEqualBetweenXObjects, SIGNAL(triggered()),this, SLOT(OnDistributeEqualDistanceBetweenX()));

	connect(ui->actionEqualBetweenTopEdges , SIGNAL(triggered()), this, SLOT(OnDistributeEqualDistanceBetweenTopEdges()));
	connect(ui->actionEqualBetweenYCenters, SIGNAL(triggered()), this, SLOT(OnDistributeEqualDistanceBetweenYCenters()));
	connect(ui->actionEqualBetweenBottomEdges, SIGNAL(triggered()),this, SLOT(OnDistributeEqualDistanceBetweenBottomEdges()));
	connect(ui->actionEqualBetweenYObjects, SIGNAL(triggered()),this, SLOT(OnDistributeEqualDistanceBetweenY()));

    // Reload.
    connect(ui->actionRepack_And_Reload, SIGNAL(triggered()), this, SLOT(OnRepackAndReloadSprites()));

    // Pixelization.
    ui->actionPixelized->setChecked(EditorSettings::Instance()->IsPixelized());
    connect(ui->actionPixelized, SIGNAL(triggered()), this, SLOT(OnPixelizationStateChanged()));
    
    // Stick Mode - enable by default.
    ui->actionStickMode->setChecked(true);
    connect(ui->actionStickMode, SIGNAL(triggered()), this, SLOT(OnStickModeChanged()));
    OnStickModeChanged();

    connect(ui->actionEnable_Guides, SIGNAL(triggered()), this, SLOT(OnEnableGuidesChanged()));
    connect(ui->actionLock_Guides, SIGNAL(triggered()), this, SLOT(OnLockGuidesChanged()));
    UpdateMenu();
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    ui->menuView->addAction(ui->hierarchyDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->libraryDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->propertiesDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->previewSettingsDockWidget->toggleViewAction());

    ui->menuView->addSeparator();
    ui->menuView->addAction(ui->mainToolbar->toggleViewAction());
    ui->menuView->addAction(ui->previewToolBar->toggleViewAction());
    ui->menuView->addAction(ui->findToolBar->toggleViewAction());
    
    // Setup the Background Color menu.
    QMenu* setBackgroundColorMenu = new QMenu("Background Color");
    ui->menuView->addSeparator();
    ui->menuView->addMenu(setBackgroundColorMenu);

    static const struct
    {
        QColor color;
        QString colorName;
    } colorsMap[] =
    {
        { Qt::black, "Black" },
        { QColor(0x33, 0x33, 0x33, 0xFF), "Default" },
        { QColor(0x53, 0x53, 0x53, 0xFF), "Dark Gray" },
        { QColor(0xB8, 0xB8, 0xB8, 0xFF), "Medium Gray" },
        { QColor(0xD6, 0xD6, 0xD6, 0xFF), "Light Gray" },
    };
    
    Color curBackgroundColor = EditorSettings::Instance()->GetCurrentBackgroundFrameColor();
    int32 itemsCount = COUNT_OF(colorsMap);
    
    bool isCustomColor = true;
    for (int32 i = 0; i < itemsCount; i ++)
    {
        QAction* colorAction = new QAction(colorsMap[i].colorName, setBackgroundColorMenu);
		colorAction->setProperty(COLOR_PROPERTY_ID, colorsMap[i].color);
        
        Color curColor = ColorHelper::QTColorToDAVAColor(colorsMap[i].color);
        if (curColor == curBackgroundColor)
        {
            isCustomColor = false;
        }

        colorAction->setCheckable(true);
        colorAction->setChecked(curColor == curBackgroundColor);
        
        backgroundFramePredefinedColorActions.append(colorAction);
		setBackgroundColorMenu->addAction(colorAction);
	}
	
    backgroundFrameUseCustomColorAction = new QAction("Custom", setBackgroundColorMenu);
	backgroundFrameUseCustomColorAction->setProperty(COLOR_PROPERTY_ID, ColorHelper::DAVAColorToQTColor(curBackgroundColor));
    backgroundFrameUseCustomColorAction->setCheckable(true);
    backgroundFrameUseCustomColorAction->setChecked(isCustomColor);
    setBackgroundColorMenu->addAction(backgroundFrameUseCustomColorAction);
    
    setBackgroundColorMenu->addSeparator();
    
    backgroundFrameSelectCustomColorAction = new QAction("Select Custom Color...", setBackgroundColorMenu);
    setBackgroundColorMenu->addAction(backgroundFrameSelectCustomColorAction);
    
    connect(setBackgroundColorMenu, SIGNAL(triggered(QAction*)), this, SLOT(SetBackgroundColorMenuTriggered(QAction*)));

    // Another actions below the Set Background Color.
    ui->menuView->addAction(ui->actionZoomIn);
    ui->menuView->insertSeparator(ui->actionZoomIn);
    ui->menuView->addAction(ui->actionZoomOut);
}

void MainWindow::UpdateMenu()
{
    bool projectCreated = HierarchyTreeController::Instance()->GetTree().IsProjectCreated();
    bool projectNotEmpty = (HierarchyTreeController::Instance()->GetTree().GetPlatforms().size() > 0);

    UpdateSaveButtons();

	ui->actionClose_project->setEnabled(projectCreated);
	ui->menuProject->setEnabled(projectCreated);
	ui->actionNew_platform->setEnabled(projectCreated);
	ui->actionImport_Platform->setEnabled(projectCreated);
	
	ui->actionNew_screen->setEnabled(projectNotEmpty);
	ui->actionNew_aggregator->setEnabled(projectNotEmpty);

	ui->actionAdjustControlSize->setEnabled(projectNotEmpty);

    ui->actionFontManager->setEnabled(projectNotEmpty);
    ui->actionLocalizationManager->setEnabled(projectNotEmpty);

    // Reload.
    ui->actionRepack_And_Reload->setEnabled(projectNotEmpty);
    
    // Preview - enabling for Screens only - not for Aggregators.
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    bool enablePreview = projectNotEmpty && activeScreen && IsPointerToExactClass<HierarchyTreeScreenNode>(activeScreen);
    ui->actionPreview->setEnabled(enablePreview);
    ui->actionEditPreviewSettings->setEnabled(enablePreview);

    // Preview Dock is not visible by default - only in Preview Mode.
    bool isPreview = ui->actionPreview->isChecked();
    ui->previewSettingsDockWidget->setVisible(isPreview);
    ui->previewSettingsDockWidget->toggleViewAction()->setEnabled(isPreview);

    // Guides.
    ui->actionEnable_Guides->setEnabled(projectNotEmpty);
    ui->actionLock_Guides->setEnabled(projectNotEmpty);
    ui->actionStickMode->setEnabled(projectNotEmpty);
    
    // Screenshot.
    ui->actionSetScreenshotFolder->setEnabled(projectNotEmpty);
    ui->actionScreenshot->setEnabled(projectNotEmpty);
}

void MainWindow::OnNewProject()
{
	if (!CloseProject())
		return;
	
	QString projectDir = QFileDialog::getExistingDirectory(this, tr("Choose new project folder"),
											ResourcesManageHelper::GetDefaultDirectory());
				
	if (projectDir.isNull() || projectDir.isEmpty())
		return;
	// Convert directory path into Unix-style path
	projectDir = ResourcesManageHelper::ConvertPathToUnixStyle(projectDir);

	CommandsController::Instance()->CleanupUndoRedoStack();
	if (!HierarchyTreeController::Instance()->NewProject(projectDir))
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while creating project"));
		msgBox.exec();
	}
}

void MainWindow::OnProjectCreated()
{
	UpdateMenu();
	UpdateScaleSlider(SCALE_PERCENTAGES[DEFAULT_SCALE_PERCENTAGE_INDEX]);
	UpdateScaleComboIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);

    SetAlignEnabled(false);
    SetDistributeEnabled(false);

    DisablePreview();
    UpdatePreviewButton();

	// Release focus from Dava GL widget, so after the first click to it
	// it will lock the keyboard and will process events successfully.
	ui->hierarchyDockWidget->setFocus();
}

void MainWindow::OnProjectLoaded()
{
    OnProjectCreated();
    EditorFontManager::Instance()->OnProjectLoaded();
}

void MainWindow::OnNewPlatform()
{
	CreatePlatformDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreatePlatformCommand* cmd = new CreatePlatformCommand(dlg.GetPlatformName(), Vector2(dlg.GetWidth(), dlg.GetHeight()));
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
		UpdateMenu();
	}
}

void MainWindow::OnNewScreen()
{
	HierarchyTreeNode::HIERARCHYTREENODEID id = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetActivePlatform();
	if (node)
		id = node->GetId();

	CreateScreenDlg dlg(this);
	dlg.SetDefaultPlatform(id);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreateScreenCommand* cmd = new CreateScreenCommand(dlg.GetScreenName(), dlg.GetPlatformId());
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::OnNewAggregator()
{
	HierarchyTreeNode::HIERARCHYTREENODEID id = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetActivePlatform();
	if (node)
		id = node->GetId();
	
	CreateAggregatorDlg dlg(this);
	dlg.SetDefaultPlatform(id);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreateAggregatorCommand* cmd = new CreateAggregatorCommand(dlg.GetName(), dlg.GetPlatformId(), dlg.GetRect());
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::OnImportPlatform()
{
	QString platformsPath = ResourcesManageHelper::GetPlatformRootPath(HierarchyTreeController::Instance()->GetTree().GetRootNode()->GetProjectDir());

	QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Select platform to import"),
															platformsPath,
															QFileDialog::DontResolveSymlinks |
															QFileDialog::ReadOnly |
															QFileDialog::ShowDirsOnly);

	if (selectedDir.isEmpty())
	{
		return;
	}

	// Convert directory path into Unix-style path
	selectedDir = ResourcesManageHelper::ConvertPathToUnixStyle(selectedDir);

	if (!selectedDir.startsWith(platformsPath))
	{
		QMessageBox::critical(this, tr("Import error"),
							  tr("Only the platforms inside current project directory could be imported"));
		return;
	}

	FilePath selectedDirPath(selectedDir.toStdString());
	String platformName = selectedDirPath.GetFilename();

    selectedDirPath.MakeDirectoryPathname();
	ImportDialog importDlg(ImportDialog::IMPORT_PLATFORM, this, selectedDirPath);
	if (importDlg.exec() == QDialog::Accepted)
	{
		QSize size = importDlg.GetPlatformSize();
		Vector<ImportDialog::FileItem> files = importDlg.GetFiles();

		ImportPlatformCommand* cmd = new ImportPlatformCommand(selectedDir,
															   QString::fromStdString(platformName),
															   size, files);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
		UpdateMenu();
	}
}

void MainWindow::OnImportScreenOrAggregator()
{
	ImportDialog importDlg(ImportDialog::IMPORT_SCREEN, this);
	if (importDlg.exec() == QDialog::Accepted)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID platformId = importDlg.GetPlatformId();
		Vector<ImportDialog::FileItem> files = importDlg.GetFiles();

		ImportScreensCommand* cmd = new ImportScreensCommand(platformId, files);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::MenuFileWillShow()
{
	// Delete old list of recent project actions
	for(DAVA::int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        if(recentPojectActions[i]->parentWidget())
        {
            ui->menuFile->removeAction(recentPojectActions[i]);
        }
    }
    // Get up to date count of recent project actions
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(projectCount > 0)
    {
        QList<QAction *> recentActions;
        for(int32 i = 0; i < projectCount; ++i)
        {
            recentPojectActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            recentActions.push_back(recentPojectActions[i]);
        }
        // Insert recent project actions into file menu
        ui->menuFile->insertActions(ui->actionExit, recentActions);
        ui->menuFile->insertSeparator(ui->actionExit);
    }
}

void MainWindow::FileMenuTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
		// Check if user clicked on one of the recent project link
        if(resentScene == recentPojectActions[i])
        {
			// Close and save current project if any
			if (!CloseProject())
				return;
		
			QString projectPath = QString::fromStdString(EditorSettings::Instance()->GetLastOpenedFile(i));
			if (projectPath.isNull())
				return;

            // Check whether project file is locked.
            if (!CheckAndUnlockProject(projectPath))
            {
                return;
            }

            // Do the load.
			if (HierarchyTreeController::Instance()->Load(projectPath))
			{
				// Update project title if project was successfully loaded
				UpdateProjectSettings(projectPath);				
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText(tr("Error while loading project"));
				msgBox.exec();
			}
			// We don't need to continue circle when selected menu item action ended
			return;
        }
    }
}

bool MainWindow::CheckAndUnlockProject(const QString& projectPath)
{
    if (!FileSystem::Instance()->IsFile(projectPath.toStdString()))
    {
        QMessageBox msgBox;
        msgBox.setText(QString(tr("The project file %1 does not exist").arg(projectPath)));
        msgBox.addButton(tr("OK"), QMessageBox::YesRole);
        msgBox.exec();
        return false;
    }
    
    if (!FileSystem::Instance()->IsFileLocked(projectPath.toStdString()))
    {
        // Nothing to unlock.
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText(QString(tr("The project file %1 is locked by other user. Do you want to unlock it?").arg(projectPath)));
    QAbstractButton* unlockButton = msgBox.addButton(tr("Unlock"), QMessageBox::YesRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    msgBox.exec();

    if (msgBox.clickedButton() != unlockButton)
    {
        return false;
    }

    // Check whether it is possible to unlock project file.
    if (!FileSystem::Instance()->LockFile(projectPath.toStdString(), false))
    {
        QMessageBox errorBox;
        errorBox.setText(QString(tr("Unable to unlock project file %1. Please check whether the project is opened in another UIEditor and close it, if yes.").arg(projectPath)));
        errorBox.exec();
        
        return false;
    }

    return true;
}

void MainWindow::DoSaveProject(bool changesOnly)
{
	QString projectPath = HierarchyTreeController::Instance()->GetTree().GetActiveProjectPath();

	if (projectPath.isNull() || projectPath.isEmpty())
		return;

	HierarchyTreeController* controller = HierarchyTreeController::Instance();
	bool saveSucceeded = changesOnly ? controller->SaveOnlyChangedScreens(projectPath) :
		controller->SaveAll(projectPath);

	if (saveSucceeded)
	{
		// If project was successfully saved - we should save new project file path
		// and add this project to recent files list
		UpdateProjectSettings(ResourcesManageHelper::GetProjectFilePath(projectPath));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while saving project"));
		msgBox.exec();
	}
}

void MainWindow::OnOpenProject()
{
	QString projectPath = QFileDialog::getOpenFileName(this, tr("Select a project file"),
														ResourcesManageHelper::GetDefaultDirectory(),
														tr( "Project (*.uieditor)"));
    if (projectPath.isNull() || projectPath.isEmpty())
        return;

	// Close and save current project if any
	if (!CloseProject())
		return;

	// Convert file path into Unix-style path
	projectPath = ResourcesManageHelper::ConvertPathToUnixStyle(projectPath);
        
	CommandsController::Instance()->CleanupUndoRedoStack();
	if (HierarchyTreeController::Instance()->Load(projectPath))
	{
		// If project was successfully loaded - we should save project path
		// and add this project to recent files list
		UpdateProjectSettings(projectPath);
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while loading project"));
		msgBox.exec();
	}
}

void MainWindow::OnSaveProject()
{
	DoSaveProject(true);
}

void MainWindow::OnSaveProjectAll()
{
	DoSaveProject(false);
}

void MainWindow::OnCloseProject()
{
	CloseProject();
}

void MainWindow::OnExitApplication()
{
	if (CloseProject())
	{
		QCoreApplication::exit();
	}
}

bool MainWindow::CloseProject()
{
	bool hasUnsavedChanged = (HierarchyTreeController::Instance()->HasUnsavedChanges() ||
                             PreviewController::Instance()->HasUnsavedChanges());
    
	if (hasUnsavedChanged)
	{
		int ret = QMessageBox::warning(this, qApp->applicationName(),
									   tr("The project has been modified.\n"
										  "Do you want to save your changes?"),
									   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
									   QMessageBox::Save);
		if (ret == QMessageBox::Cancel)
			return false;
		else if (ret == QMessageBox::Save)
			OnSaveProject();
	}
	
	CopyPasteController::Instance()->Clear();
	HierarchyTreeController::Instance()->CloseProject();
	// Update project title
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());
	
	return true;
}

void MainWindow::UpdateProjectSettings(const QString& projectPath)
{
	// Add file to recent project files list
	EditorSettings::Instance()->AddLastOpenedFile(projectPath.toStdString());
	
	// Save to settings default project directory
	QFileInfo fileInfo(projectPath);
	QString projectDir = fileInfo.absoluteDir().absolutePath();
	EditorSettings::Instance()->SetProjectPath(projectDir.toStdString());

	// Update window title
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle(projectPath));
    
    // Apply the pixelization value.
    Texture::SetPixelization(EditorSettings::Instance()->IsPixelized());
}

void MainWindow::OnUndoRequested()
{
	CommandsController::Instance()->Undo();
}

void MainWindow::OnRedoRequested()
{
	CommandsController::Instance()->Redo();
}

void MainWindow::OnZoomInRequested()
{
	OnUpdateScaleRequest(1.0f);
}

void MainWindow::OnZoomOutRequested()
{
	OnUpdateScaleRequest(-1.0f);
}

void MainWindow::OnUndoRedoAvailabilityChanged()
{
	this->ui->actionUndo->setEnabled(CommandsController::Instance()->IsUndoAvailable());
	this->ui->actionRedo->setEnabled(CommandsController::Instance()->IsRedoAvailable());
}

void MainWindow::OnChangePropertySucceeded()
{
	OnSelectedScreenChanged();
}

void MainWindow::OnUnsavedChangesNumberChanged()
{
	QString projectTitle = ResourcesManageHelper::GetProjectTitle();
	if (HierarchyTreeController::Instance()->HasUnsavedChanges())
	{
		projectTitle += " *";
	}
    
    UpdateSaveButtons();

	setWindowTitle(projectTitle);
}

Vector2 MainWindow::CalculateScenePositionForPoint(const QRect& widgetRect, const Vector2& point, float curScale)
{
	// Correctly handle scales less then 100%.
	QRect viewRect = ScreenWrapper::Instance()->GetRect();
	
	// If horz/vert offset is less than zero, it means no scroll bars visible, and the view size
	// less than widget size. This situation should be handled separately.
	float hOffset = (viewRect.width() - widgetRect.width()) / 2;
	if (hOffset > 0)
	{
		hOffset = ui->horizontalScrollBar->value();
	}
	
	float vOffset = (viewRect.height() - widgetRect.height()) / 2;
	if (vOffset > 0)
	{
		vOffset = ui->verticalScrollBar->value();
	}
	
	Vector2 resultPosition;
	resultPosition.x = (hOffset + point.x) / curScale;
	resultPosition.y = (vOffset + point.y) / curScale;
	
	return resultPosition;
}

void MainWindow::ScrollToScenePositionAndPoint(const Vector2& scenePosition, const Vector2& point,
											   float newScale)
{
	float newHScrollValue = (scenePosition.x * newScale) - point.x;
	if (newHScrollValue < ui->horizontalScrollBar->minimum())
	{
		newHScrollValue = ui->horizontalScrollBar->minimum();
	}
	if (newHScrollValue > ui->horizontalScrollBar->maximum())
	{
		newHScrollValue = ui->horizontalScrollBar->maximum();
	}
	
	float newVScrollValue = (scenePosition.y * newScale) - point.y;
	if (newVScrollValue < ui->verticalScrollBar->minimum())
	{
		newVScrollValue = ui->verticalScrollBar->minimum();
	}
	if (newVScrollValue > ui->verticalScrollBar->maximum())
	{
		newVScrollValue = ui->verticalScrollBar->maximum();
	}
	
	ui->horizontalScrollBar->setValue(newHScrollValue);
	ui->verticalScrollBar->setValue(newVScrollValue);
}

void MainWindow::OnAdjustSize()
{
	HierarchyTreeController::Instance()->AdjustSelectedControlsSize();
}

void MainWindow::OnAlignLeft()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_LEFT);
}

void MainWindow::OnAlignHorzCenter()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_HORZ_CENTER);
}

void MainWindow::OnAlignRight()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_RIGHT);
}

void MainWindow::OnAlignTop()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_TOP);
}

void MainWindow::OnAlignVertCenter()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_VERT_CENTER);
}

void MainWindow::OnAlignBottom()
{
	HierarchyTreeController::Instance()->AlignSelectedControls(ALIGN_CONTROLS_BOTTOM);
}

void MainWindow::OnDistributeEqualDistanceBetweenLeftEdges()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_LEFT_EDGES);
}

void MainWindow::OnDistributeEqualDistanceBetweenXCenters()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_X_CENTERS);
}

void MainWindow::OnDistributeEqualDistanceBetweenRightEdges()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_RIGHT_EDGES);
}

void MainWindow::OnDistributeEqualDistanceBetweenX()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_X);
}

void MainWindow::OnDistributeEqualDistanceBetweenTopEdges()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_TOP_EDGES);
}

void MainWindow::OnDistributeEqualDistanceBetweenYCenters()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_Y_CENTERS);
}

void MainWindow::OnDistributeEqualDistanceBetweenBottomEdges()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_BOTTOM_EDGES);
}

void MainWindow::OnDistributeEqualDistanceBetweenY()
{
	HierarchyTreeController::Instance()->DistributeSelectedControls(DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_Y);
}

void MainWindow::OnRepackAndReloadSprites()
{
    RepackAndReloadSprites();
}

void MainWindow::NotifyScaleUpdated(float32 newScale)
{
    ScreenWrapper::Instance()->SetScale(newScale);
    GridVisualizer::Instance()->SetScale(newScale);

    RulerController::Instance()->SetScale(newScale);
    
    DefaultScreen* currentScreen = ScreenWrapper::Instance()->GetActiveScreen();
    if (currentScreen)
    {
        currentScreen->SetScreenPositionChangedFlag();
    }
}

void MainWindow::OnPixelizationStateChanged()
{
    bool isPixelized = ui->actionPixelized->isChecked();
    EditorSettings::Instance()->SetPixelized(isPixelized);

    ScreenWrapper::Instance()->SetApplicationCursor(Qt::WaitCursor);
    Texture::SetPixelization(isPixelized);
    ScreenWrapper::Instance()->RestoreApplicationCursor();
}

void MainWindow::RepackAndReloadSprites()
{
    ScreenWrapper::Instance()->SetApplicationCursor(Qt::WaitCursor);
    const Set<String>& errorsSet = HierarchyTreeController::Instance()->RepackAndReloadSprites();
    ScreenWrapper::Instance()->RestoreApplicationCursor();

    if (!errorsSet.empty())
	{
		ErrorsListDialog errorsDialog;
		errorsDialog.InitializeErrorsList(errorsSet);
		errorsDialog.exec();
	}
}

void MainWindow::SetBackgroundColorMenuTriggered(QAction* action)
{
    Color newColor;

    if (action == backgroundFrameSelectCustomColorAction)
    {
        // Need to select new Background Frame color.
        QColor curColor = ColorHelper::DAVAColorToQTColor(EditorSettings::Instance()->GetCustomBackgroundFrameColor());
        QColor color = QColorDialog::getColor(curColor, this, "Select color", QColorDialog::ShowAlphaChannel);
        if (color.isValid() == false)
        {
            return;
        }

        newColor = ColorHelper::QTColorToDAVAColor(color);
        EditorSettings::Instance()->SetCustomBackgroundFrameColor(newColor);
    }
    else if (action == backgroundFrameUseCustomColorAction)
    {
        // Need to use custom Background Frame Color set up earlier.
        newColor = EditorSettings::Instance()->GetCustomBackgroundFrameColor();
    }
    else
    {
        // Need to use predefined Background Frame Color.
        newColor = ColorHelper::QTColorToDAVAColor(action->property(COLOR_PROPERTY_ID).value<QColor>());
    }

    EditorSettings::Instance()->SetCurrentBackgroundFrameColor(newColor);
    ScreenWrapper::Instance()->SetBackgroundFrameColor(newColor);
    
    // Update the check marks.
    bool colorFound = false;
    foreach (QAction* colorAction, backgroundFramePredefinedColorActions)
    {
        Color color = ColorHelper::QTColorToDAVAColor(colorAction->property(COLOR_PROPERTY_ID).value<QColor>());
        if (color == newColor)
        {
            colorAction->setChecked(true);
            colorFound = true;
        }
        else
        {
            colorAction->setChecked(false);
        }
    }

    // In case we don't found current color in predefined ones - select "Custom" menu item.
    backgroundFrameUseCustomColorAction->setChecked(!colorFound);
}

void MainWindow::SetAlignEnabled(bool value)
{
	ui->actionAlign_Left->setEnabled(value);
	ui->actionAlign_Horz_Center->setEnabled(value);
	ui->actionAlign_Right->setEnabled(value);

	ui->actionAlign_Top->setEnabled(value);
    ui->actionAlign_Vert_Center->setEnabled(value);
	ui->actionAlign_Bottom->setEnabled(value);
}

void MainWindow::SetDistributeEnabled(bool value)
{
	ui->actionEqualBetweenLeftEdges->setEnabled(value);
	ui->actionEqualBetweenXCenters->setEnabled(value);
    ui->actionEqualBetweenRightEdges->setEnabled(value);
    ui->actionEqualBetweenXObjects->setEnabled(value);

	ui->actionEqualBetweenTopEdges->setEnabled(value);
    ui->actionEqualBetweenYCenters->setEnabled(value);
    ui->actionEqualBetweenBottomEdges->setEnabled(value);
    ui->actionEqualBetweenYObjects->setEnabled(value);
}

void MainWindow::OnPreviewTriggered()
{
    bool isPreview = ui->actionPreview->isChecked();
    if (isPreview)
    {
        PreviewSettingsDialog* dialog = new PreviewSettingsDialog(true);
        if (dialog->exec() != QDialog::Accepted)
        {
            ui->actionPreview->setChecked(false);
            delete dialog;
            return;
        }

        EnablePreview(dialog->GetSelectedData(), dialog->GetApplyScale());
        delete dialog;
    }
    else
    {
        DisablePreview();
    }

    UpdatePreviewButton();
}

void MainWindow::OnPreviewModeChanged(int previewSettingsID)
{
    const PreviewSettingsData& previewData = PreviewController::Instance()->GetPreviewSettingsData(previewSettingsID);
    SetPreviewMode(previewData);
}

void MainWindow::OnGLWidgetResized()
{
    UpdateSliders();
    UpdateScreenPosition();
}

void MainWindow::EnablePreview(const PreviewSettingsData& data, bool applyScale)
{
    HierarchyTreeController::Instance()->EnablePreview(data, applyScale);
    UpdatePreviewScale();
    EnableEditing(false);
}

void MainWindow::SetPreviewMode(const PreviewSettingsData& data)
{
    HierarchyTreeController::Instance()->SetPreviewMode(data);
    UpdatePreviewScale();
    UpdateScreenPosition();
}

void MainWindow::DisablePreview()
{
    EnableEditing(true);
    HierarchyTreeController::Instance()->DisablePreview();

    ScreenWrapper::Instance()->SetScale(1.0f);
    UpdateScaleControls();
    UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::EnableEditing(bool value)
{
    ui->hierarchyDockWidget->setVisible(value);
    ui->libraryDockWidget->setVisible(value);
    ui->propertiesDockWidget->setVisible(value);
    ui->previewSettingsDockWidget->setVisible(!value);

    ui->scaleCombo->setVisible(value);
    ui->scaleSlider->setVisible(value);

    ui->mainToolbar->setEnabled(value);

    ui->actionZoomIn->setEnabled(value);
    ui->actionZoomOut->setEnabled(value);

    ui->horizontalRuler->setVisible(value);
    ui->verticalRuler->setVisible(value);

    ui->hierarchyDockWidget->toggleViewAction()->setEnabled(value);
    ui->libraryDockWidget->toggleViewAction()->setEnabled(value);
    ui->propertiesDockWidget->toggleViewAction()->setEnabled(value);
    ui->previewSettingsDockWidget->toggleViewAction()->setEnabled(!value);

    if (value)
    {
        ui->actionUndo->setEnabled(CommandsController::Instance()->IsUndoAvailable());
        ui->actionRedo->setEnabled(CommandsController::Instance()->IsRedoAvailable());
    }
    else
    {
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
    }
}

void MainWindow::UpdatePreviewButton()
{
    bool isPreview = ui->actionPreview->isChecked();
    if (isPreview)
    {
        ui->actionPreview->setText("Preview Mode");
    }
    else
    {
        ui->actionPreview->setText("Editing Mode");
    }
}

void MainWindow::UpdatePreviewScale()
{
    const PreviewTransformData& transformData = PreviewController::Instance()->GetTransformData();
    ScreenWrapper::Instance()->SetScale(transformData.zoomLevel);
    UpdateScaleControls();
}

void MainWindow::OnEditPreviewSettings()
{
    // The dialog should be opened in Edit mode, nos Select.
    PreviewSettingsDialog* dialog = new PreviewSettingsDialog(false);
    dialog->exec();
    delete dialog;
}

void MainWindow::OnGuideDropped(Qt::DropAction dropAction)
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (!activeScreen)
    {
        return;
    }
    
    if (dropAction == Qt::IgnoreAction)
    {
        activeScreen->CancelNewGuide();
        return;
    }
    
    if (!activeScreen->CanAcceptNewGuide())
    {
        // New guide is on the same position as existing one - no need to add.
        activeScreen->CancelNewGuide();
        return;
    }
    
    // Create the appropriate command.
    AddNewGuideCommand* command = new AddNewGuideCommand(activeScreen);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void MainWindow::OnStickModeChanged()
{
    int32 stickMode = ui->actionStickMode->isChecked() ? StickToSides | StickToCenters : StickDisabled;
    HierarchyTreeController::Instance()->SetStickMode(stickMode);
}

void MainWindow::OnEnableGuidesChanged()
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (activeScreen)
    {
        activeScreen->SetGuidesEnabled(ui->actionEnable_Guides->isChecked());
    }
}

void MainWindow::OnSetScreenshotFolder()
{
    SetScreenshotFolder();
}

void MainWindow::SetScreenshotFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Screenshots Directory"),
                                                    screenShotFolder,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
    {
        screenShotFolder = dir;
    }
}

void MainWindow::OnScreenshot()
{
    DefaultScreen* currentScreen = ScreenWrapper::Instance()->GetActiveScreen();
    if (!currentScreen || !currentScreen->GetScreenControl())
    {
        return;
    }

    static const float32 maxScreenshotScale = 4.0f;
    if (currentScreen->GetScale().x > maxScreenshotScale || currentScreen->GetScale().y > maxScreenshotScale)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Current zoom level is too high for making screenshots. Reduce it to less than %1%.").arg((int)(maxScreenshotScale * 100)));
        msgBox.exec();

        return;
    }

    if (screenShotFolder.isEmpty())
    {
        SetScreenshotFolder();
    }

    QString deviceModel = "NormalView";
    if (PreviewController::Instance()->IsPreviewEnabled())
    {
        deviceModel = QString::fromStdString(PreviewController::Instance()->GetActivePreviewSettingsData().deviceName);
        deviceModel.replace(QRegExp("[^a-zA-Z0-9]"),QString("_"));
    }

    QString screenShotFileName = QString("UIEditor_Screenshot_%1_%2").arg(deviceModel).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd_HH.mm.ss.z.png"));
    QString fullPath = QDir().cleanPath(screenShotFolder + QDir::separator() + screenShotFileName);

    ScreenWrapper::Instance()->SetApplicationCursor(Qt::WaitCursor);
    PreviewController::Instance()->MakeScreenshot(fullPath.toStdString(), currentScreen);
    ScreenWrapper::Instance()->RestoreApplicationCursor();
}

void MainWindow::OnLockGuidesChanged()
{
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    if (activeScreen)
    {
        activeScreen->LockGuides(ui->actionLock_Guides->isChecked());
    }
}

void MainWindow::UpdateSaveButtons()
{
    bool hasUnsavedChanges = HierarchyTreeController::Instance()->HasUnsavedChanges();
    
    ui->actionSave_project->setEnabled(hasUnsavedChanges);
    ui->actionSave_All->setEnabled(hasUnsavedChanges);
}


void MainWindow::OnSearchPressed()
{
    QString partOfName = findField->text();
    if (partOfName.isEmpty())
    	return;
    
    QList<HierarchyTreeControlNode*> foundNodes;
    QList<HierarchyTreeScreenNode*> foundScreens;
    HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    
    HierarchyTreeController::Instance()->ResetSelectedControl();
    
    if (NULL == activeScreen)
    {
        HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
        if (activePlatform)
        {
            foundScreens = SearchScreenByName(activePlatform->GetChildNodes(),partOfName,ui->actionIgnoreCase->isChecked());
        }
        
        if (!foundScreens.empty())
        {
        	// Select first found screen/aggregator
        	this->ui->hierarchyDockWidgetContents->ScrollTo(foundScreens.at(0));
            if (foundScreens.size() > 1)
            {	// and highlight other found screens/aggregators
            	this->ui->hierarchyDockWidgetContents->HighlightScreenNodes(foundScreens);
            }
        }
    }
    else
    {
        SearchControlsByName(foundNodes,activeScreen->GetChildNodes(),partOfName,ui->actionIgnoreCase->isChecked());
        if (!foundNodes.empty())
        {
        	// Multiple selection, or control selected
        	HierarchyTreeController::Instance()->SynchronizeSelection(foundNodes);
            if (foundNodes.size() == 1)
            {
        		// Scroll to first one in the list
        		this->ui->hierarchyDockWidgetContents->ScrollTo(foundNodes.at(0));
            }
        }
    }
}

void MainWindow::SearchControlsByName(QList<HierarchyTreeControlNode*>& foundNodes,const HierarchyTreeNode::HIERARCHYTREENODESLIST nodes, const  QString partOfName,bool ignoreCase) const
{
    HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it = nodes.begin();
    for (; it!=nodes.end(); ++it)
    {
        HierarchyTreeControlNode * controlNode = static_cast<HierarchyTreeControlNode *>(*it);
        const QString name = QString::fromStdString(controlNode->GetUIObject()->GetName());
        Qt::CaseSensitivity cs = ignoreCase?Qt::CaseInsensitive:Qt::CaseSensitive;
        if (name.contains(partOfName,cs))
        {
            foundNodes.push_back(controlNode);
        }
        SearchControlsByName(foundNodes,(*it)->GetChildNodes(),partOfName,ignoreCase);
    }
}

QList<HierarchyTreeScreenNode*> MainWindow::SearchScreenByName(const HierarchyTreeNode::HIERARCHYTREENODESLIST nodes, const  QString partOfName,bool ignoreCase) const
{
    QList<HierarchyTreeScreenNode*> foundNodes;
    HierarchyTreeNode::HIERARCHYTREENODESCONSTITER it = nodes.begin();
    for (; it!=nodes.end(); ++it)
    {
        HierarchyTreeScreenNode * screenNode = static_cast<HierarchyTreeScreenNode *>(*it);
        QString name = screenNode->GetName();
        Qt::CaseSensitivity cs = ignoreCase?Qt::CaseInsensitive:Qt::CaseSensitive;
        if (name.contains(partOfName,cs))
        {
            foundNodes.push_back(screenNode);
        }
    }
    return foundNodes;
}
