#include "Platform/Qt5/QtLayer.h"

#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Sound/SoundSystem.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "UI/UIControlSystem.h"

extern void FrameworkWillTerminate();
extern void FrameworkDidLaunched();

namespace DAVA
{
QtLayer::QtLayer()
    : delegate(nullptr)
    , isDAVAEngineEnabled(true)
{
}

QtLayer::~QtLayer()
{
    AppFinished();
}

void QtLayer::Quit()
{
    if (delegate)
    {
        delegate->Quit();
    }
}

void QtLayer::SetDelegate(QtLayerDelegate* delegate)
{
    this->delegate = delegate;
}

void QtLayer::AppStarted()
{
    FrameworkDidLaunched();
    Core::Instance()->SystemAppStarted();
}

void QtLayer::AppFinished()
{
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();
    Core::Instance()->Release();
}

void QtLayer::OnSuspend()
{
    SoundSystem::Instance()->Suspend();
    //    Core::Instance()->SetIsActive(false);
}

void QtLayer::OnResume()
{
    SoundSystem::Instance()->Resume();
    Core::Instance()->SetIsActive(true);
}

void QtLayer::ProcessFrame()
{
    if (Core::Instance()->IsConsoleMode() == false)
    { // avoid calling of system process frame for console mode. Right not it is called from DAVA GL Widget. Will be refactored in future

        rhi::InvalidateCache(); //as QT itself can break gl states
        Core::Instance()->SystemProcessFrame();
    }
}

void QtLayer::Resize(int32 width, int32 height, float64 dpr)
{
    int32 realWidth = static_cast<int32>(width * dpr);
    int32 realHeight = static_cast<int32>(height * dpr);
    rhi::ResetParam resetParams;
    resetParams.width = realWidth;
    resetParams.height = realHeight;
    Renderer::Reset(resetParams);

    VirtualCoordinatesSystem* vcs = VirtualCoordinatesSystem::Instance();
    DVASSERT(nullptr != vcs)

    vcs->SetInputScreenAreaSize(realWidth, realHeight);

    vcs->UnregisterAllAvailableResourceSizes();
    vcs->RegisterAvailableResourceSize(width, height, "Gfx");
    vcs->RegisterAvailableResourceSize(width, height, "Gfx2");

    vcs->SetPhysicalScreenSize(realWidth, realHeight);
    vcs->SetVirtualScreenSize(width, height);
    vcs->ScreenSizeChanged();
}

void QtLayer::KeyPressed(Key key, uint64 timestamp)
{
    UIEvent ev;
    ev.phase = UIEvent::Phase::KEY_DOWN;
    ev.timestamp = static_cast<float64>(timestamp);
    ev.device = UIEvent::Device::KEYBOARD;
    ev.key = key;

    UIControlSystem::Instance()->OnInput(&ev);

    InputSystem::Instance()->GetKeyboard().OnKeyPressed(key);
}

void QtLayer::KeyReleased(Key key, uint64 timestamp)
{
    UIEvent ev;
    ev.phase = UIEvent::Phase::KEY_UP;
    ev.device = UIEvent::Device::KEYBOARD;
    ev.timestamp = static_cast<float64>(timestamp);
    ev.key = key;

    UIControlSystem::Instance()->OnInput(&ev);

    InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(key);
}

void QtLayer::MouseEvent(const UIEvent& event)
{
    UIEvent evCopy(event);
    UIControlSystem::Instance()->OnInput(&evCopy);
}

    
#if defined(__DAVAENGINE_WIN32__)

void* QtLayer::CreateAutoreleasePool()
{
    return nullptr;
}

void QtLayer::ReleaseAutoreleasePool(void* pool)
{
    (void)pool;
}
    
#endif
};
