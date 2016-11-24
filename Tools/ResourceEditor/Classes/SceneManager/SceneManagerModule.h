#pragma once

#include "Base/BaseTypes.h"

#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/Utils/QtConnections.h"

#include "Classes/SceneManager/Private/SceneRenderWidget.h"
#include "Classes/Qt/Main/RecentMenuItems.h"

namespace DAVA
{
class FilePath;
}

class SceneEditor2;
class SceneData;

class SceneManagerModule : public DAVA::TArc::ControllerModule, private SceneRenderWidget::IWidgetDelegate
{
protected:
    void OnRenderSystemInitialized(DAVA::Window* w) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key) override;
    bool ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;

    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne) override;
    void OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne) override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void PostInit() override;

private:
    void CreateModuleControls(DAVA::TArc::UI* ui);
    void CreateModuleActions(DAVA::TArc::UI* ui);
    void RegisterOperations();

    /// Action and operation handlers
    void CreateNewScene();
    void OpenScene();
    void OpenSceneQuckly();
    void OpenSceneByPath(const DAVA::FilePath& scenePath);
    void SaveScene();
    void SaveScene(bool saveAs);
    void SaveSceneToFolder(bool compressedTextures);
    void ExportScene();
    void CloseAllScenes(bool needSavingReqiest);
    void ReloadTextures(DAVA::eGPUFamily gpu);

    void ShowPreview(const DAVA::FilePath& scenePath);
    void HidePreview();

    /// Fields value handlers
    void OnActiveTabChanged(const DAVA::Any& contextID);
    void OnScenePathChanged(const DAVA::Any& scenePath);

    /// IWidgetDelegate
    bool OnCloseSceneRequest(DAVA::uint64 id) override;
    void OnDragEnter(QObject* target, QDragEnterEvent* event) override;
    void OnDragMove(QObject* target, QDragMoveEvent* event) override;
    void OnDrop(QObject* target, QDropEvent* event) override;

    /// Helpers
    void UpdateTabTitle(DAVA::uint64 contextID);
    bool CanCloseScene(SceneData* data);
    DAVA::RefPtr<SceneEditor2> OpenSceneImpl(const DAVA::FilePath& scenePath);

    /// This method try to scene at "scenePath" place.
    /// If "scenePath" is empty, method try to save scene at current scene file.
    /// If current scene path is empty (for example this is completely new scene), method will call FileSaveDialog
    /// return true if scene was saved
    /// Preconditions:
    ///     "scenePath" - should be a file
    bool SaveSceneImpl(DAVA::RefPtr<SceneEditor2> scene, const DAVA::FilePath& scenePath = DAVA::FilePath());
    DAVA::FilePath GetSceneSavePath(const DAVA::RefPtr<SceneEditor2>& scene);

    /// scene->SaveEmitters() would call this function if emitter to save didn't have path
    DAVA::FilePath SaveEmitterFallback(const DAVA::String& entityName, const DAVA::String& emitterName);
    bool IsSceneCompatible(const DAVA::FilePath& scenePath);

    bool SaveTileMaskInAllScenes();
    bool SaveTileMaskInScene(DAVA::RefPtr<SceneEditor2> scene);

    bool CloseSceneImpl(DAVA::uint64 id, bool needSavingRequest);
    void RestartParticles();
    bool IsSavingAllowed(SceneData* sceneData);
    void DefaultDragHandler(QObject* target, QDropEvent* event);
    bool IsValidMimeData(QDropEvent* event);
    void DeleteSelection();
    void MoveToSelection();

private:
    DAVA::TArc::QtConnections connections;
    DAVA::uint32 newSceneCounter = 0;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    std::unique_ptr<RecentMenuItems> recentItems;

    QPointer<SceneRenderWidget> renderWidget;
};
