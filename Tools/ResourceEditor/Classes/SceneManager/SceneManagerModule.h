#pragma once

#include "Base/BaseTypes.h"

#include "TArc/Core/ControllerModule.h"
#include "TArc/Utils/QtConnections.h"

namespace DAVA
{
class FilePath;
}

class SceneEditor2;

class SceneManagerModule : public DAVA::TArc::ControllerModule
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

    /// Action and operation handlers
    void CreateNewScene();

    SceneEditor2* OpenSceneImpl(const DAVA::FilePath& scenePath);

private:
    DAVA::TArc::QtConnections connections;
    DAVA::uint32 newSceneCounter = 0;
};