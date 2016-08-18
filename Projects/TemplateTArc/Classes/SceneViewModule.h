#pragma once

#include "UI/UI3DView.h"
#include "UI/UIScreen.h"

#include "TArcCore/ControllerModule.h"
#include "DataProcessing/DataContext.h"

#include "Base/BaseTypes.h"

class SceneViewModule : public tarc::ControllerModule
{
public:
    SceneViewModule() = default;

protected:
    void OnRenderSystemInitialized(DAVA::Window& w) override;
    void OnContextCreated(tarc::DataContext& context) override;
    void OnContextDeleted(tarc::DataContext& context) override;
    void PostInit() override;

private:
    void OnWindowResized(DAVA::Window& w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY);

private:
    DAVA::ScopedPtr<DAVA::UI3DView> ui3dView = DAVA::ScopedPtr<DAVA::UI3DView>(nullptr);
    DAVA::ScopedPtr<DAVA::UIScreen> uiScreen = DAVA::ScopedPtr<DAVA::UIScreen>(nullptr);
};