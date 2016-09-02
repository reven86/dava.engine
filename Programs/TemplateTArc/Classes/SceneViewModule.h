#pragma once

#include "UI/UI3DView.h"
#include "UI/UIScreen.h"

#include "TArcCore/ControllerModule.h"
#include "WindowSubSystem/UI.h"
#include "DataProcessing/DataContext.h"
#include "TArcUtils/QtConnections.h"

#include "Base/BaseTypes.h"

class SceneViewModule : public DAVA::TArc::ControllerModule
{
public:
    SceneViewModule();

protected:
    void OnRenderSystemInitialized(DAVA::Window& w) override;
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;

private:
    void SetupRenderWidget();
    void SetupActions();
    void OnWindowResized(DAVA::Window& w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY);

    void OpenScene();
    void OpenScene(const DAVA::String& scenePath);

private:
    DAVA::ScopedPtr<DAVA::UI3DView> ui3dView = DAVA::ScopedPtr<DAVA::UI3DView>(nullptr);
    DAVA::ScopedPtr<DAVA::UIScreen> uiScreen = DAVA::ScopedPtr<DAVA::UIScreen>(nullptr);
    DAVA::TArc::QtConnections connections;

    DAVA::TArc::WindowKey windowKey;
};