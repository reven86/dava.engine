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
    void CloseAllScenes();

    /// Fields value handlers
    void OnActiveTabChanged(const DAVA::Any& contextID);
    void OnSceneModificationFlagChanged(const DAVA::Any& isSceneModified);
    void OnScenePathChanged(const DAVA::Any& scenePath);

    /// IWidgetDelegate
    void CloseSceneRequest(DAVA::uint64 id) override;

    /// Helpers
    void UpdateTabTitle(DAVA::uint64 contextID);
    bool CanCloseScene(SceneEditor2* scene);
    bool IsSavingAllowed(SceneEditor2* scene);
    SceneEditor2* OpenSceneImpl(const DAVA::FilePath& scenePath);

private:
    DAVA::TArc::QtConnections connections;
    DAVA::uint32 newSceneCounter = 0;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
};