#pragma once

#include "ui_mainwindow.h"

#include "Classes/Qt/Main/ModificationWidget.h"
#include "Classes/Qt/Tools/QtWaitDialog/QtWaitDialog.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/RecentMenuItems.h"
#include "Classes/Qt/GlobalOperations.h"
#include "Classes/Beast/BeastProxy.h"

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"

#include "QtTools/Utils/ShortcutChecker.h"

#include "DAVAEngine.h"
#include "Base/Platform.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QPointer>

class RECommandNotificationObject;
class AddSwitchEntityDialog;
class Request;
class QtLabelWithActions;
class HangingObjectsHeight;
class DeveloperTools;
class VersionInfoWidget;
#if defined(NEW_PROPERTY_PANEL)
class PropertyPanel;
#endif
class DeviceListController;
class SpritesPackerModule;
class ErrorDialogOutput;

namespace DAVA
{
class RenderWidget;
}

class QtMainWindow : public QMainWindow, public GlobalOperations, private DAVA::TArc::DataListener
{
    Q_OBJECT

    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;

signals:
    void GlobalInvalidateTimeout();

    void TexturesReloaded();

public:
    explicit QtMainWindow(DAVA::TArc::UI* tarcUI, QWidget* parent = 0);
    ~QtMainWindow();

    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);
    void OnRenderingInitialized();
    SceneEditor2* GetCurrentScene();

    bool OpenScene(const QString& path);
    bool SaveScene(SceneEditor2* scene);
    bool SaveSceneAs(SceneEditor2* scene);

    void SetGPUFormat(DAVA::eGPUFamily gpu);

    void WaitStart(const QString& title, const QString& message, int min = 0, int max = 100);
    void WaitSetMessage(const QString& messsage);
    void WaitSetValue(int value);
    bool IsWaitDialogOnScreen() const;
    void WaitStop();

    void BeastWaitSetMessage(const QString& messsage);
    bool BeastWaitCanceled();

    void EnableGlobalTimeout(bool enable);

    bool CanBeClosed();

    bool ParticlesArePacking() const;

    void CallAction(ID id, DAVA::Any&& args) override;
    QWidget* GetGlobalParentWidget() const override;
    void ShowWaitDialog(const DAVA::String& tittle, const DAVA::String& message, DAVA::uint32 min = 0, DAVA::uint32 max = 100) override;
    bool IsWaitDialogVisible() const override;
    void HideWaitDialog() override;
    void ForEachScene(const DAVA::Function<void(SceneEditor2*)>& functor) override;

    void CloseAllScenes();

    // qt actions slots
public slots:
    void OnSceneNew();
    void OnSceneOpen();
    void OnSceneOpenQuickly();
    void OnSceneSave();
    void OnSceneSaveAs();
    void OnSceneSaveToFolder();
    void OnSceneSaveToFolderCompressed();
    void OnRecentFilesTriggered(QAction* recentAction);
    void ExportTriggered();
    void OnImportSpeedTreeXML();
    void RemoveSelection();

    void OnUndo();
    void OnRedo();

    void OnEditorGizmoToggle(bool show);
    void OnViewLightmapCanvas(bool show);
    void OnAllowOnSceneSelectionToggle(bool allow);
    void OnShowStaticOcclusionToggle(bool show);
    void OnEnableVisibilitySystemToggle(bool enabled);
    void OnRefreshVisibilitySystem();
    void OnFixVisibilityFrame();
    void OnReleaseVisibilityFrame();

    void OnEnableDisableShadows(bool enable);

    void EnableSounds(bool enable);

    void OnReloadTextures();
    void OnReloadTexturesTriggered(QAction* reloadAction);

    void OnSelectMode();
    void OnMoveMode();
    void OnRotateMode();
    void OnScaleMode();
    void OnPivotCenterMode();
    void OnPivotCommonMode();
    void OnManualModifMode();
    void OnPlaceOnLandscape();
    void OnSnapToLandscape();
    void OnResetTransform();
    void OnLockTransform();
    void OnUnlockTransform();

    void OnCenterPivotPoint();
    void OnZeroPivotPoint();

    void OnMaterialEditor(DAVA::NMaterial* material = nullptr);
    void OnTextureBrowser();
    void OnSceneLightMode();

    void OnCubemapEditor();
    void OnImageSplitter();

    void OnAddLandscape();
    void OnAddVegetation();
    void OnLightDialog();
    void OnCameraDialog();
    void OnEmptyEntity();
    void OnAddWindEntity();
    void OnAddPathEntity();

    void OnUserNodeDialog();
    void OnSwitchEntityDialog();
    void OnParticleEffectDialog();
    void On2DCameraDialog();
    void On2DSpriteDialog();
    void OnAddEntityFromSceneTree();

    void OnShowSettings();
    void OnOpenHelp();

    void OnSaveHeightmapToImage();
    void OnSaveTiledTexture();
    void OnTiledTextureRetreived(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture);

    void OnConvertModifiedTextures();

    void OnCloseTabRequest(int tabIndex, Request* closeRequest);

    void OnBeastAndSave();

    void OnBuildStaticOcclusion();
    void OnInavalidateStaticOcclusion();

    void OnLandscapeEditorToggled(SceneEditor2* scene);
    void OnForceFirstLod(bool);
    void OnCustomColorsEditor();
    void OnHeightmapEditor();
    void OnRulerTool();
    void OnTilemaskEditor();
    void OnNotPassableTerrain();
    void OnWayEditor();

    void OnObjectsTypeChanged(QAction* action);
    void OnObjectsTypeChanged(int type);

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);

    void OnMaterialLightViewChanged(bool);
    void OnCustomQuality();

    void OnReloadShaders();

    void OnSwitchWithDifferentLODs(bool checked);

    void OnGenerateHeightDelta();

    void OnBatchProcessScene();

    void OnSnapCameraToLandscape(bool);

    void SetupTitle(const DAVA::String& projectPath);

    void RestartParticleEffects();
    bool SetVisibilityToolEnabledIfPossible(bool);
    void UpdateLandscapeRenderMode();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void SetupWidget();
    void SetupMainMenu();
    void SetupThemeActions();
    void SetupToolBars();
    void SetupStatusBar();
    void SetupDocks();
    void SetupActions();
    void SetupShortCuts();

    void StartGlobalInvalidateTimer();

    void RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode);

    bool IsAnySceneChanged();

    void DiableUIForFutureUsing();
    void SynchronizeStateWithUI();

    bool SelectCustomColorsTexturePath();

    static void SetActionCheckedSilently(QAction* action, bool checked);

    void OnSceneSaveAsInternal(bool saveWithCompressed);

    void SaveAllSceneEmitters(SceneEditor2* scene) const;

private slots:
    void SceneCommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification);
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

    void OnGlobalInvalidateTimeout();
    void EditorLightEnabled(bool enabled);
    void OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);
    void UnmodalDialogFinished(int);

    void DebugVersionInfo();
    void DebugColorPicker();
    void DebugDeviceList();
    void OnConsoleItemClicked(const QString& data);

    void UpdateUndoActionText(const DAVA::String& text);
    void UpdateRedoActionText(const DAVA::String& text);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QtWaitDialog* beastWaitDialog;
    QPointer<QDockWidget> dockActionEvent;
    QPointer<QDockWidget> dockConsole;

    bool globalInvalidate;

    ModificationWidget* modificationWidget;

    QComboBox* objectTypesWidget;

    AddSwitchEntityDialog* addSwitchEntityDialog;
    HangingObjectsHeight* hangingObjectsWidget;

    void EnableSceneActions(bool enable);
    void EnableProjectActions(bool enable);
    void UpdateConflictingActionsState(bool enable);
    void UpdateModificationActionsState();
    void UpdateWayEditor(const RECommandNotificationObject& commandNotification);

    void LoadViewState(SceneEditor2* scene);
    void LoadModificationState(SceneEditor2* scene);
    void LoadEditorLightState(SceneEditor2* scene);
    void LoadGPUFormat();
    void LoadLandscapeEditorState(SceneEditor2* scene);
    void LoadObjectTypes(SceneEditor2* scene);
    void LoadHangingObjects(SceneEditor2* scene);
    void LoadMaterialLightViewMode();

    bool SaveTilemask(bool forAllTabs = true);

    // Landscape editor specific
    // TODO: remove later -->
    bool LoadAppropriateTextureFormat();
    bool IsSavingAllowed();
    // <--

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

    //Need for any debug functionality
    QPointer<DeveloperTools> developerTools;
    QPointer<VersionInfoWidget> versionInfoWidget;

    QPointer<DeviceListController> deviceListController;

    RecentMenuItems recentFiles;

#if defined(NEW_PROPERTY_PANEL)
    wgt::IComponentContext& ngtContext;
    std::unique_ptr<PropertyPanel> propertyPanel;
#endif
    std::shared_ptr<GlobalOperations> globalOperations;
    ErrorDialogOutput* errorLoggerOutput = nullptr;

#if defined(__DAVAENGINE_MACOS__)
    ShortcutChecker shortcutChecker;
#endif

    DAVA::TArc::UI* tarcUI = nullptr;
    std::unique_ptr<DAVA::TArc::WaitHandle> waitDialog;
    DAVA::TArc::DataWrapper projectDataWrapper;

private:
    struct EmitterDescriptor
    {
        EmitterDescriptor(DAVA::ParticleEmitter* _emitter, DAVA::ParticleLayer* layer, DAVA::FilePath path, DAVA::String name)
            : emitter(_emitter)
            , ownerLayer(layer)
            , yamlPath(path)
            , entityName(name)
        {
        }

        DAVA::ParticleEmitter* emitter = nullptr;
        DAVA::ParticleLayer* ownerLayer = nullptr;
        DAVA::FilePath yamlPath;
        DAVA::String entityName;
    };

    void CollectEmittersForSave(DAVA::ParticleEmitter* topLevelEmitter, DAVA::List<EmitterDescriptor>& emitters, const DAVA::String& entityName) const;
};
