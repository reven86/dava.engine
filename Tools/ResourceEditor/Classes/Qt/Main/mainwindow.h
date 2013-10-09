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
#include "ui_mainwindow.h"
#include "ModificationWidget.h"
#include "Tools/QtWaitDialog/QtWaitDialog.h"

#include "Base/Singleton.h"

#include "Scene/SceneEditor2.h"
#include "Tools/QtPosSaver/QtPosSaver.h"

// TODO: remove old screen -->
#include "Classes/SceneEditor/MaterialEditor.h"
// <---
class AddSwitchEntityDialog;

class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
	Q_OBJECT

protected:
    
    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;
    
public:
	explicit QtMainWindow(bool enableGlobalTimeout, QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
	SceneTabWidget* GetSceneWidget();
	SceneEditor2* GetCurrentScene();

	bool SaveSceneAs(SceneEditor2 *scene);

	void SetGPUFormat(DAVA::eGPUFamily gpu);
	DAVA::eGPUFamily GetGPUFormat();

	void WaitStart(const QString &title, const QString &message, int min = 0, int max = 100);
	void WaitSetMessage(const QString &messsage);
	void WaitSetValue(int value);
	void WaitStop();

    void EnableGlobalTimeout(bool enable);
    
signals:
    void GlobalInvalidateTimeout();

    
// qt actions slots
public slots:
	void OnProjectOpen();
	void OnProjectClose();
	void OnSceneNew();
	void OnSceneOpen();
	void OnSceneSave();
	void OnSceneSaveAs();
	void OnSceneSaveToFolder();
	void OnRecentTriggered(QAction *recentAction);
	void ExportMenuTriggered(QAction *exportAsAction);

	void OnUndo();
	void OnRedo();

	void OnReloadTextures();
	void OnReloadTexturesTriggered(QAction *reloadAction);

    void OnReloadSprites();
    
	void OnSelectMode();
	void OnMoveMode();
	void OnRotateMode();
	void OnScaleMode();
	void OnPivotCenterMode();
	void OnPivotCommonMode();
	void OnManualModifMode();
	void OnPlaceOnLandscape();
	void OnSnapToLandscape();

	void OnMaterialEditor();
	void OnTextureBrowser();
	void OnSceneLightMode();

	void OnCubemapEditor();
		
	void OnLandscapeDialog();
	void OnLightDialog();
	void OnServiceNodeDialog();
	void OnCameraDialog();
	void OnImposterDialog();

	void OnUserNodeDialog();
	void OnSwitchEntityDialog();
	void OnParticleEffectDialog();
	void OnUniteEntitiesWithLODs();
	void OnAddEntityMenuAboutToShow();
	
	void OnNotPassableTerrain();
	void OnRulerTool();
	
	void OnSetSkyboxNode();
	
	void OnShowSettings();

	void OnOpenHelp();

	void OnSetShadowColor();
	void OnShadowBlendModeMenu();
	void OnShadowBlendModeAlpha();
	void OnShadowBlendModeMultiply();
    
    void OnSaveHeightmapToPNG();
    void OnSaveTiledTexture();
    
    void OnConvertToShadow();
	
	void OnAddActionComponent();
	void OnRemoveActionComponent();

protected:
	virtual bool eventFilter(QObject *object, QEvent *event);

	void SetupMainMenu();
	void SetupToolBars();
	void SetupDocks();
	void SetupActions();
	void SetupTitle();

	void InitRecent();
	void AddRecent(const QString &path);
    
    void CreateMaterialEditorIfNeed();
    
    void UpdateStatusBar();
    void StartGlobalInvalidateTimer();

protected slots:
	void ProjectOpened(const QString &path);
	void ProjectClosed();

	void SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
	void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);
	void EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity);
	void EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity);
    
	void AddSwitchDialogFinished(int result);

	void UpdateRulerToolLength(SceneEditor2* scene, double length, double previewLength);

    void OnGlobalInvalidateTimeout();

	void NotPassableToggled(SceneEditor2* scene);

	void EditorLightEnabled(bool enabled);

private:
	Ui::MainWindow *ui;
	QtWaitDialog *waitDialog;
	QtPosSaver posSaver;

	QList<QAction *> recentScenes;
	ModificationWidget *modificationWidget;
	AddSwitchEntityDialog* addSwitchEntityDialog;

	// TODO: remove this old screen -->
	MaterialEditor *materialEditor;
	// <--

	void LoadUndoRedoState(SceneEditor2 *scene);
	void LoadModificationState(SceneEditor2 *scene);
	void LoadEditorLightState(SceneEditor2 *scene);
	void LoadNotPassableState(SceneEditor2* scene);
	void LoadRulerToolState(SceneEditor2* scene);
	void LoadGPUFormat();
	void CreateAndDisplayAddEntityDialog(Entity* sceneNode);
    
    bool globalInvalidateTimeoutEnabled;
};


#endif // MAINWINDOW_H
