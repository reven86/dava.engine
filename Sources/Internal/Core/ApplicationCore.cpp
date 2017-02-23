#if !defined(__DAVAENGINE_COREV2__)
#include "Core/ApplicationCore.h"
#include "Animation/AnimationManager.h"
#include "UI/UIControlSystem.h"
#include "Sound/SoundSystem.h"
#include "Time/SystemTimer.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Notification/LocalNotificationController.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/StringFormat.h"
#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif
#include "Platform/Steam.h"

namespace DAVA
{
ApplicationCore::ApplicationCore()
    : BaseObject()
#if defined(__DAVAENGINE_ANDROID__)
    , backgroundTicker(NULL)
    , backgroundTickerFinishing(false)
    , backgroundTickTimeMs(250)
#endif
{
}

ApplicationCore::~ApplicationCore()
{
}

void ApplicationCore::Update(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_UPDATE)

#ifdef __DAVAENGINE_AUTOTESTING__
    float32 realFrameDelta = SystemTimer::GetRealFrameDelta();
    AutotestingSystem::Instance()->Update(realFrameDelta);
#endif

    SoundSystem::Instance()->Update(timeElapsed);
    AnimationManager::Instance()->Update(timeElapsed);
    UIControlSystem::Instance()->Update();
    
#if defined(__DAVAENGINE_STEAM__)
    Steam::Update();
#endif
}

void ApplicationCore::OnEnterFullscreen()
{
}

void ApplicationCore::OnExitFullscreen()
{
}

void ApplicationCore::Draw()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_DRAW)

    Renderer::GetRenderStats().Reset();

    RenderSystem2D::Instance()->BeginFrame();

    UIControlSystem::Instance()->Draw();
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Draw();
#endif
    RenderSystem2D::Instance()->EndFrame();
}

void ApplicationCore::BeginFrame()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_BEGIN_FRAME)

    Renderer::BeginFrame();
}

void ApplicationCore::EndFrame()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_END_FRAME)

    Renderer::EndFrame();
}

void ApplicationCore::OnRenderingIsNotPossible(rhi::RenderingError error)
{
    String info = Format("Rendering is not possible and no handler found. Application will likely crash or hang now. Error: 0x%08x",
                         static_cast<DAVA::uint32>(error));

    DVASSERT(0, info.c_str());
    Logger::Error("%s", info.c_str());
    abort();
}

void ApplicationCore::OnSuspend()
{
    SoundSystem::Instance()->Suspend();
    Core::Instance()->SetIsActive(false);

#if defined(__DAVAENGINE_ANDROID__)
    StartBackgroundTicker();
#endif
}

void ApplicationCore::OnResume()
{
#if defined(__DAVAENGINE_ANDROID__)
    StopBackgroundTicker();
#endif

    Core::Instance()->SetIsActive(true);
    SoundSystem::Instance()->Resume();
}

#if defined(__DAVAENGINE_ANDROID__)

void ApplicationCore::StartBackgroundTicker(uint32 tickPeriod)
{
    if (NULL == backgroundTicker)
    {
        Logger::Debug("[ApplicationCore: OnSuspend] Background tick Thread Create Start");
        backgroundTicker = Thread::Create(Message(this, &ApplicationCore::BackgroundTickerHandler));
        backgroundTickerFinishing = false;
        if (backgroundTicker)
        {
            backgroundTickTimeMs = tickPeriod;
            backgroundTicker->Start();
        }
        Logger::Debug("[ApplicationCore: OnSuspend] Background tick  Thread Create End");
    }
}

void ApplicationCore::StopBackgroundTicker()
{
    if (NULL != backgroundTicker)
    {
        Logger::Debug("[ApplicationCore: OnResume] Background tick Thread Finish start");
        backgroundTickerFinishing = true;
        backgroundTicker->Join();
        SafeRelease(backgroundTicker);
        Logger::Debug("[ApplicationCore: OnResume] Background tick Thread Finish end");
    }
}

void ApplicationCore::BackgroundTickerHandler(BaseObject* caller, void* callerData, void* userData)
{
    while (!backgroundTickerFinishing)
    {
        Thread::Sleep(backgroundTickTimeMs);
        OnBackgroundTick();
    }
}
#endif

void ApplicationCore::OnBackgroundTick()
{
    DownloadManager::Instance()->Update();
    LocalNotificationController::Instance()->Update();
}

bool ApplicationCore::OnQuit()
{
    return false;
}

void ApplicationCore::OnAppFinished()
{
#if defined(__DAVAENGINE_ANDROID__)
    StopBackgroundTicker();
#endif
}

void ApplicationCore::OnBackground()
{
    // Default implementation is empty.
}

void ApplicationCore::OnForeground()
{
    // Default implementation is empty.
}

void ApplicationCore::OnDeviceLocked()
{
    // Default implementation is empty.
}

void ApplicationCore::OnFocusLost()
{
    // Default implementation is empty.
}

void ApplicationCore::OnFocusReceived()
{
    // Default implementation is empty.
}
};

#endif // !__DAVAENGINE_COREV2__
