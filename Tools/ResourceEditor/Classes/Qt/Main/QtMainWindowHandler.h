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



#ifndef __QT_MAIN_WINDOW_HANDLER_H__
#define __QT_MAIN_WINDOW_HANDLER_H__

#if 0

#include <QObject>
#include <QPoint>
#include <QVector>
#include <QAbstractButton>
#include <QRadioButton.h>

#include "DAVAEngine.h"
#include "../Constants.h"
#include "Classes/SceneEditor/EditorSettings.h"

#include "TextureBrowser/TextureBrowser.h"
#include "MaterialBrowser/MaterialBrowser.h"
#include "Classes/Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"
#include "Classes/Commands/CommandList.h"

#include "../Scene/System/HeightmapEditorSystem.h"

class Command;
class QMenu;
class QAction;
class QTreeView;
class QStatusBar;
class QPushButton;
class QSlider;
class QComboBox;
class ModificationWidget;
class QSpinBox;
class QCheckBox;
class QDoubleSpinBox;
class SceneEditor2;
class AddSwitchEntityDialog;

class QtMainWindowHandler: public QObject, public DAVA::Singleton<QtMainWindowHandler>
{
    Q_OBJECT
    
public:
    QtMainWindowHandler(QObject *parent = 0);
    virtual ~QtMainWindowHandler();

    void RegisterNodeActions(DAVA::int32 count, ...);
    void RegisterViewportActions(DAVA::int32 count, ...);
    void RegisterDockActions(DAVA::int32 count, ...);
    void RegisterTextureGPUActions(DAVA::int32 count, ...);
	void RegisterModificationActions(DAVA::int32 count, ...);
	void RegisterEditActions(DAVA::int32 count, ...);

    void SetResentMenu(QMenu *menu);

    //MENU FILE
    void UpdateRecentScenesList();

	void SetDefaultFocusWidget(QWidget *widget);
	void RestoreDefaultFocus();
    
    void RegisterStatusBar(QStatusBar *registeredSatusBar);
    void ShowStatusBarMessage(const DAVA::String &message, DAVA::int32 displayTime = 0);
    
    void SetWaitingCursorEnabled(bool enabled);

	//heightmap editor
	void RegisterHeightmapEditorWidgets(QPushButton*, QSlider*, QComboBox*, QRadioButton*,
										QRadioButton*, QRadioButton*, QSlider*, QSlider*,
										QLabel*, QRadioButton*, QRadioButton*, QCheckBox*,
										QCheckBox*);
	void SetHeightmapEditorWidgetsState(bool state);

	//tilemask editor
	void RegisterTilemaskEditorWidgets(QPushButton*, QSlider*, QComboBox*, QSlider*, QComboBox*);
	void SetTilemaskEditorWidgetsState(bool state);

	//custom colors
	void RegisterCustomColorsWidgets(QPushButton*, QPushButton*, QSlider*, QComboBox*, QPushButton*);
    void SetCustomColorsWidgetsState(bool state);

	//custom colors new
	void RegisterCustomColorsEditorWidgets(QPushButton*, QPushButton*, QSlider*, QComboBox*, QPushButton*);
    void SetCustomColorsEditorWidgetsState(bool state);

	//set switch index
	void RegisterSetSwitchIndexWidgets(QSpinBox*, QRadioButton*, QRadioButton*, QPushButton*);
    void SetSwitchIndexWidgetsState(bool state);

	//material view options
	void RegisterMaterialViewOptionsWidgets(QComboBox*);
	void SetMaterialViewOptionsWidgetsState(bool state);
	void SelectMaterialViewOption(Material::eViewOptions value);

	//hanging objects
	void RegisterHangingObjectsWidgets(QCheckBox*, QDoubleSpinBox*, QPushButton*);
    void SetHangingObjectsWidgetsState(bool state);

	//visibility check tool
	void RegisterWidgetsVisibilityTool(QPushButton*, QPushButton*, QPushButton*, QPushButton*, QSlider*);
	void SetWidgetsStateVisibilityTool(bool state);
	void SetPointButtonStateVisibilityTool(bool state);
	void SetAreaButtonStateVisibilityTool(bool state);

	//visibility tool new
	void RegisterVisibilityToolWidgets(QPushButton*, QPushButton*, QPushButton*, QPushButton*, QSlider*);
	void SetVisibilityToolWidgetsState(bool state);

	void UpdateUndoActionsState();
    
    bool SaveScene(Scene *scene);
	bool SaveScene(Scene *scene, const FilePath &pathname);


	void EnableSkyboxMenuItem(bool isEnabled);

public slots:
    void CreateNodeTriggered(QAction *nodeAction);
    void ViewportTriggered(QAction *viewportAction);
    void FileMenuTriggered(QAction *resentScene);

    //File
    void NewScene();
    void OpenScene();
    void OpenProject();
    void OpenResentScene(DAVA::int32 index);

	bool SaveScene();
    void ExportMenuTriggered(QAction *exportAsAction);

	void SaveToFolderWithChilds();

	//Edit
	void UndoAction();
	void RedoAction();
	void ConvertToShadow();

    //View
    void RestoreViews();

    //tools
    void Materials();
    void ConvertTextures();
    void HeightmapEditor();
    void TilemapEditor();
    void RulerTool();
    void ShowSettings();
    void Beast();
    void SquareTextures();
	void CubemapEditor();
    void ReplaceZeroMipmaps();

    //ViewOptions
    void MenuViewOptionsWillShow();
    void ToggleNotPassableTerrain();
    void ReloadMenuTriggered(QAction *reloadAsAction);
    
    //Help
    void OpenHelp();

    //scene graph
    void RefreshSceneGraph();
    
	//set switch index
	void ToggleSetSwitchIndex(DAVA::uint32  value, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX state);

	//material view options
	void MaterialViewOptionChanged(int index);

	//hanging objects
	void ToggleHangingObjects(float value, bool isEnabled);

	//heightmap editor
	void ToggleHeightmapEditor();
	void SetHeightmapEditorBrushSize(int brushSize);
	void SetHeightmapEditorToolImage(int imageIndex);
	void SetRelativeHeightmapDrawing();
	void SetAverageHeightmapDrawing();
	void SetAbsoluteHeightmapDrawing();
	void SetHeightmapEditorStrength(int strength);
	void SetHeightmapEditorAverageStrength(int averageStrength);
	void SetHeightmapDropperHeight(SceneEditor2* scene, double height);
	void SetHeightmapDropper();
	void SetHeightmapCopyPaste();
	void SetHeightmapCopyPasteHeightmap(int );
	void SetHeightmapCopyPasteTilemask(int);

	//tilemask editor
	void ToggleTilemaskEditor();
	void SetTilemaskEditorBrushSize(int brushSize);
	void SetTilemaskEditorToolImage(int imageIndex);
	void SetTilemaskEditorStrength(int strength);
	void SetTilemaskDrawTexture(int textureIndex);

    //custom colors
    void ToggleCustomColors();
    void SaveTextureCustomColors();
    void ChangeBrushSizeCustomColors(int newSize);
    void ChangeColorCustomColors(int newColorIndex);
	void LoadTextureCustomColors();

	//custom colors new
	void ToggleCustomColorsEditor();
	void SaveCustomColorsTexture();
	void LoadCustomColorsTexture();
	void SetCustomColorsBrushSize(int brushSize);
	void SetCustomColorsColor(int colorIndex);
	void NeedSaveCustomColorsTexture(SceneEditor2* scene);

	//visibility check tool
	void ToggleVisibilityTool();
	void SaveTextureVisibilityTool();
	void ChangleAreaSizeVisibilityTool(int newSize);
	void SetVisibilityPointVisibilityTool();
	void SetVisibilityAreaVisibilityTool();

	//visibility check tool new
	void ToggleVisibilityToolEditor();
	void SaveVisibilityToolTexture();
	void SetVisibilityToolAreaSize(int size);
	void SetVisibilityPoint();
	void SetVisibilityArea();
	void SetVisibilityToolButtonsState(SceneEditor2* scene);

    //
    void RepackAndReloadTextures();

	//particles editor
	void CreateParticleEmitterNode();
	
	//modification options
	void ModificationSelect();
	void ModificationMove();
	void ModificationRotate();
	void ModificationScale();
	void ModificationPlaceOnLand();
	void ModificationSnapToLand();
	void OnApplyModification(double x, double y, double z);
	void OnResetModification();
	void SetModificationMode(ResourceEditor::eModificationActions mode);

	void OnSceneActivated(SceneData *scene);
	void OnSceneReleased(SceneData *scene);
	void OnSceneCreated(SceneData *scene);

	void ReloadSceneTextures();

	void OnEntityModified(DAVA::Scene* scene, CommandList::eCommandId id, const DAVA::Set<DAVA::Entity*>& affectedEntities);

    void CameraLightTrigerred();

    void AddSwitchEntity();
	void AddActionComponent();
	void RemoveActionComponent();
    
	void SetShadowColor();
	void SetShadowBlendAlpha();
	void SetShadowBlendMultiply();

signals:
	void ProjectChanged();

private:
    //create node
    void CreateNode(ResourceEditor::eNodeType type);
    //viewport
    void SetViewport(ResourceEditor::eViewportType type);

    void RegisterActions(QAction **actions, DAVA::int32 count, va_list &vl);
    
    void ClearActions(int32 count, QAction **actions);

	void UpdateModificationActions();
    
	void SaveParticleEmitterNodes(Scene* scene);
	void SaveParticleEmitterNodeRecursive(Entity* parentNode);


	// This method is called after each action is executed and responsible
	// for enabling/disabling appropriate menu items depending on actions.
	void HandleMenuItemsState(CommandList::eCommandId id, const DAVA::Set<DAVA::Entity*>& affectedEntities);

	// "SkyBox menu" - specific checks.
	void CheckNeedEnableSkyboxMenu(const DAVA::Set<DAVA::Entity*>& affectedEntities,
								   bool isEnabled);
	
	void UpdateSkyboxMenuItemAfterSceneLoaded(SceneData* sceneData);
	void SetHeightmapDrawingType(HeightmapEditorSystem::eHeightmapDrawType type);

	void UpdateTilemaskTileTextures();

private:
	//set switch index
	QPushButton*	setSwitchIndexToggleButton;
	QSpinBox*		editSwitchIndexValue;
	QRadioButton*	rBtnSelection;
	QRadioButton*	rBtnScene;

	//material view options
	QComboBox* comboMaterialViewOption;

	//hanging objects
	QPushButton*	hangingObjectsToggleButton;
	QDoubleSpinBox*	editHangingObjectsValue;
	QCheckBox *		checkBoxHangingObjects;

	//custom colors
	QPushButton* customColorsToggleButton;
	QPushButton* customColorsSaveTextureButton;
	QSlider* customColorsBrushSizeSlider;
	QComboBox* customColorsColorComboBox;
	QPushButton* customColorsLoadTextureButton;

	//custom colors new
	QPushButton* customColorsEditorToggleButton;
	QPushButton* customColorsSaveTexture;
	QSlider* customColorsBrushSize;
	QComboBox* customColorsColor;
	QPushButton* customColorsLoadTexture;

	//visibility check tool
	QPushButton* visibilityToolToggleButton;
	QPushButton* visibilityToolSaveTextureButton;
	QPushButton* visibilityToolSetPointButton;
	QPushButton* visibilityToolSetAreaButton;
	QSlider* visibilityToolAreaSizeSlider;

	//visibility check tool new
	QPushButton* visibilityToolEditorToggleButton;
	QPushButton* visibilityToolSaveTexture;
	QPushButton* visibilityToolSetPoint;
	QPushButton* visibilityToolSetArea;
	QSlider* visibilityToolAreaSize;

	//heightmap editor
	QPushButton* heightmapToggleButton;
	QSlider* heightmapBrushSize;
	QComboBox* heightmapToolImage;
	QRadioButton* heightmapDrawingRelative;
	QRadioButton* heightmapDrawingAverage;
	QRadioButton* heightmapDrawingAbsolute;
	QSlider* heightmapStrength;
	QSlider* heightmapAverageStrength;
	QLabel* heightmapDropperHeight;
	QRadioButton* heightmapDropper;
	QRadioButton* heightmapCopyPaste;
	QCheckBox* heightmapCopyHeightmap;
	QCheckBox* heightmapCopyTilemask;

	//tilemassk editor
	QPushButton* tilemaskToggleButton;
	QSlider* tilemaskBrushSize;
	QComboBox* tilemaskToolImage;
	QSlider* tilemaskStrength;
	QComboBox* tilemaskDrawTexture;

    QAction *resentSceneActions[EditorSettings::RESENT_FILES_COUNT];
    QAction *nodeActions[ResourceEditor::NODE_COUNT];
    QAction *viewportActions[ResourceEditor::VIEWPORT_COUNT];
    QAction *hidablewidgetActions[ResourceEditor::HIDABLEWIDGET_COUNT];
    QAction *textureForGPUActions[DAVA::GPU_FAMILY_COUNT + 1];
	QAction *modificationActions[ResourceEditor::MODIFY_COUNT];
	QAction *editActions[ResourceEditor::EDIT_COUNT];

    QMenu *menuResentScenes;
	QWidget *defaultFocusWidget;
    
    QStatusBar *statusBar;

};

#endif

#endif // __QT_MAIN_WINDOW_HANDLER_H__
