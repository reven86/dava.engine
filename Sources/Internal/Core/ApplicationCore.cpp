#include "Core/ApplicationCore.h"
#include "Animation/AnimationManager.h"
#include "UI/UIControlSystem.h"
#include "Render/OcclusionQuery.h"
#include "Sound/SoundSystem.h"
#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Notification/LocalNotificationController.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Debug/Profiler.h"
#include "Concurrency/Thread.h"
#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif

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
    TIME_PROFILE("ApplicationCore::Update");
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Update(timeElapsed);
#endif
    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "SoundSystem::Update")
    SoundSystem::Instance()->Update(timeElapsed);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "SoundSystem::Update")

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "AnimationManager::Update")
    AnimationManager::Instance()->Update(timeElapsed);
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "AnimationManager::Update")

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Update")
    UIControlSystem::Instance()->Update();
    TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "UIControlSystem::Update")
}

void ApplicationCore::OnEnterFullscreen()
{
}

void ApplicationCore::OnExitFullscreen()
{
}

void ApplicationCore::Draw()
{
    TIME_PROFILE("ApplicationCore::Draw");
    RenderSystem2D::Instance()->BeginFrame();

    FrameOcclusionQueryManager::Instance()->ResetFrameStats();

    UIControlSystem::Instance()->Draw();
#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Draw();
#endif
    FrameOcclusionQueryManager::Instance()->ProccesRenderedFrame();
    RenderSystem2D::Instance()->EndFrame();
}

void ApplicationCore::BeginFrame()
{
    Renderer::BeginFrame();
}

void ApplicationCore::EndFrame()
{
    Renderer::EndFrame();
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
