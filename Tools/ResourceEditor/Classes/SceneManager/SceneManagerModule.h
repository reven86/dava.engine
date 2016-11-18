#pragma once

#include "Base/BaseTypes.h"

#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/Utils/QtConnections.h"

#include "Classes/SceneManager/Private/SceneRenderWidget.h"

namespace DAVA
{
class FilePath;
}

class SceneEditor2;

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
    void OnContextWillChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne) override;
    void OnContextDidChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne) override;
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
    void SaveSceneAs();
    void SaveSceneToFolder(bool compressedTextures);
    void CloseAllScenes();

    /// Fields value handlers
    void OnActiveTabChanged(const DAVA::Any& contextID);
    void OnScenePathChanged(const DAVA::Any& scenePath);

    /// IWidgetDelegate
    bool CloseSceneRequest(DAVA::uint64 id) override;

    /// Helpers
    void UpdateTabTitle(DAVA::uint64 contextID);
    bool CanCloseScene(SceneEditor2* scene);
    bool IsSavingAllowed(SceneEditor2* scene);
    SceneEditor2* OpenSceneImpl(const DAVA::FilePath& scenePath);

    /// This method try to scene at "scenePath" place.
    /// If "scenePath" is empty, method try to save scene at current scene file.
    /// If current scene path is empty (for example this is completely new scene), method will call FileSaveDialog
    /// return true if scene was saved
    /// Preconditions:
    ///     "scenePath" - should be a file
    bool SaveSceneImpl(SceneEditor2* scene, const DAVA::FilePath& scenePath = DAVA::FilePath());
    DAVA::FilePath GetSceneSavePath(const SceneEditor2* scene);

    /// scene->SaveEmitters would call this function if emitter to save didn't have path
    DAVA::FilePath SaveEmitterFallback(const DAVA::String& entityName, const DAVA::String& emitterName);

private:
    DAVA::TArc::QtConnections connections;
    DAVA::uint32 newSceneCounter = 0;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
};