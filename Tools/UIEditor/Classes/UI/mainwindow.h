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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "ScreenWrapper.h"
#include "EditorSettings.h"

#include "PreviewController.h"

namespace Ui {
class MainWindow;
}

class QFileDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void showEvent(QShowEvent * event);
	virtual void closeEvent(QCloseEvent * event);

private slots:
	// Zoom.
	void OnScaleSliderValueChanged(int value);
	void OnScaleComboIndexChanged(int value);
	void OnScaleComboTextEditingFinished();

	// Scroll.
	void OnSliderMoved();

    void OnOpenFontManager();
    void OnOpenLocalizationManager();
    void OnShowHelp();
	
	void OnNewProject();
	void OnSaveProject();
	void OnSaveProjectAll();
    void OnOpenProject();
	void OnCloseProject();
	void OnExitApplication();
	
	void OnNewPlatform();
	void OnNewScreen();
	void OnNewAggregator();

	void OnImportPlatform();
	void OnImportScreenOrAggregator();

	void OnProjectCreated();
	void OnSelectedScreenChanged();
	
	void OnUpdateScaleRequest(float scaleDelta);
	void OnUpdateScreenPositionRequest(const QPoint& posDelta);
	
	void FileMenuTriggered(QAction *resentScene);
	void MenuFileWillShow();

    void SetBackgroundColorMenuTriggered(QAction* action);

	void OnUndoRequested();
	void OnRedoRequested();
	
	void OnZoomInRequested();
	void OnZoomOutRequested();
	
	void OnUndoRedoAvailabilityChanged();
	void OnChangePropertySucceeded();

	void OnUnsavedChangesNumberChanged();

    void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &);

	// Adjust size
	void OnAdjustSize();
	
	// Align block.
	void OnAlignLeft();
	void OnAlignHorzCenter();
	void OnAlignRight();
	void OnAlignTop();
	void OnAlignVertCenter();
	void OnAlignBottom();

	// Distribute blocks.
	void OnDistributeEqualDistanceBetweenLeftEdges();
	void OnDistributeEqualDistanceBetweenXCenters();
	void OnDistributeEqualDistanceBetweenRightEdges();
	void OnDistributeEqualDistanceBetweenX();

	void OnDistributeEqualDistanceBetweenTopEdges();
	void OnDistributeEqualDistanceBetweenYCenters();
	void OnDistributeEqualDistanceBetweenBottomEdges();
	void OnDistributeEqualDistanceBetweenY();

    // Repack and Reload.
    void OnRepackAndReloadSprites();
    
    // Pixelization.
    void OnPixelizationStateChanged();

    // Editing mode (Edit/Preview)
    void OnPreviewTriggered();
    
    // Edit Preview Settings..
    void OnEditPreviewSettings();
    
    // Notification from GL widget its resize is done.
    void OnGLWidgetResized();

private:
	bool CloseProject();

	void UpdateScaleControls();
	void UpdateSliders();
	void UpdateScreenPosition();
	
	void InitMenu();
    void SetupViewMenu();
	void UpdateMenu();
	void UpdateProjectSettings(const QString& filename);

	// Save/restore positions of DockWidgets and main window geometry
	void SaveMainWindowState();
	void RestoreMainWindowState();
	// Create toolbar for HierarchyTreeDockWidget
	void CreateHierarchyDockWidgetToolbar();

	// Save the full project or changes only.
	void DoSaveProject(bool changesOnly);

	// Handle the scale change.
	void UpdateScaleAndScaleSliderByIndex(int32 index);

	void UpdateScale(int32 newScalePercents);
	void UpdateScaleComboIndex(int newIndex);
	void UpdateScaleSlider(int32 newScalePercents);

	// "Smart zoom" functionality - retain the scene under cursor when zoom is changed.
	Vector2 CalculateScenePositionForPoint(const QRect& widgetRect, const Vector2& point, float curScale);
	void ScrollToScenePositionAndPoint(const Vector2& scenePosition, const Vector2& point,
									   float newScale);

    // Notify external systems that the scale is updated.
    void NotifyScaleUpdated(float32 newScale);

    // Repack and reload sprites.
    void RepackAndReloadSprites();

    // Control Align/Distribute actions.
    void SetAlignEnabled(bool value);
    void SetDistributeEnabled(bool value);

    // Preview handling.
    void EnablePreview(const PreviewSettingsData& data);
    void DisablePreview();
    void UpdatePreviewButton();

    // Enable/disable editing controls for Preview mode.
    void EnableEditing(bool value);

private:
    Ui::MainWindow *ui;
	QAction *recentPojectActions[EditorSettings::RECENT_FILES_COUNT];

    // Background Frame Color menu actions.
    QList<QAction*> backgroundFramePredefinedColorActions;
    QAction* backgroundFrameUseCustomColorAction;
    QAction* backgroundFrameSelectCustomColorAction;

	bool screenChangeUpdate;
};

#endif // MAINWINDOW_H
