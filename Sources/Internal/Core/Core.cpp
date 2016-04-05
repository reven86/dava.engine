/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DAVAClassRegistrator.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Core/ApplicationCore.h"
#include "Core/Core.h"
#include "Core/PerformanceSettings.h"
#include "Platform/SystemTimer.h"
#include "UI/UIScreenManager.h"
#include "UI/UIControlSystem.h"
#include "Input/InputSystem.h"
#include "Debug/DVAssert.h"
#include "Render/2D/TextBlock.h"
#include "Debug/Replay.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Base/AllocatorFactory.h"
#include "Render/2D/FTFont.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Image/ImageSystem.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "DLC/Downloader/DownloadManager.h"
#include "DLC/Downloader/CurlDownloader.h"
#include "Render/OcclusionQuery.h"
#include "Notification/LocalNotificationController.h"
#include "Platform/DeviceInfo.h"
#include "Render/Renderer.h"
#include "UI/UIControlSystem.h"

#include "Network/NetCore.h"
#include "MemoryManager/MemoryProfiler.h"

#include "Job/JobManager.h"

#if defined(__DAVAENGINE_STEAM__)
#include "Platform/Steam.h"
#endif

#if defined(__DAVAENGINE_ANDROID__)
#include <cfenv>
#pragma STDC FENV_ACCESS on
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#endif

#if defined(__DAVAENGINE_IPHONE__)
#include <cfenv>
#pragma STDC FENV_ACCESS on
#elif defined(__DAVAENGINE_ANDROID__)
#include "Input/AccelerometerAndroid.h"
#endif //PLATFORMS

#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
#include <EGL/eglext.h>
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif

#include "Concurrency/Thread.h"
#include "Debug/Profiler.h"

#include "Core.h"
#define PROF__FRAME 0
#define PROF__FRAME_UPDATE 1
#define PROF__FRAME_DRAW 2
#define PROF__FRAME_ENDFRAME 3

namespace DAVA
{
static ApplicationCore* core = nullptr;

Core::Core()
{
    globalFrameIndex = 1;
    isActive = false;
    firstRun = true;
    isConsoleMode = false;
    options = new KeyedArchive();
    float32 defaultUserScale = 1.f;
    if (nullptr != options)
    {
        defaultUserScale = options->GetFloat("userScreenScaleFactor", 1.0f);
    }
    screenMetrics.userScale = defaultUserScale;
}

Core::~Core()
{
    SafeRelease(options);
    SafeRelease(core);
}

namespace fpu_exceptions
{
#ifdef DAVA_ENGINE_DEBUG_FPU_EXCEPTIONS
#ifdef __DAVAENGINE_WINDOWS__
void (*SEFuncPtr)(unsigned int, PEXCEPTION_POINTERS) = nullptr;

void SEHandler(unsigned int exceptionCode, PEXCEPTION_POINTERS pExpInfo)
{
    switch (exceptionCode)
    {
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    // https://social.msdn.microsoft.com/Forums/en-US/48f63378-19be-413f-88a5-0f24aa72d3c8/the-exceptions-statusfloatmultipletraps-and-statusfloatmultiplefaults-is-needed-in-more
    case STATUS_FLOAT_MULTIPLE_TRAPS:
    case STATUS_FLOAT_MULTIPLE_FAULTS:
    {
        _clearfp();
        StringStream ss;
        ss << "floating-point structured exception: 0x" << std::hex << exceptionCode
           << " at 0x" << pExpInfo->ExceptionRecord->ExceptionAddress;
        throw std::runtime_error(ss.str());
    }
    default:
        if (SEFuncPtr != nullptr)
        {
            SEFuncPtr(exceptionCode, pExpInfo);
        }
        else
        {
            StringStream ss;
            ss << "structured exception: 0x" << std::hex << exceptionCode
               << " at 0x" << pExpInfo->ExceptionRecord->ExceptionAddress;
            throw std::runtime_error(ss.str());
        }
    }
};

void EnableFloatingPointExceptions()
{
    // https://msdn.microsoft.com/en-us/library/5z4bw5h5.aspx
    fpu_exceptions::SEFuncPtr = _set_se_translator(&fpu_exceptions::SEHandler);

    unsigned int feValue = ~(_EM_INVALID | /*_EM_DENORMAL |*/ _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW /* | _EM_INEXACT*/);
    unsigned int mask = _MCW_EM;
    unsigned int currentWord = 0;
    errno_t err = _controlfp_s(&currentWord, 0, 0);
    DVASSERT(err == 0);
    // https://msdn.microsoft.com/en-us/library/c9676k6h.aspx
    err = _controlfp_s(&currentWord, feValue, mask);
    DVASSERT(err == 0);
    Logger::FrameworkDebug("FPU exceptions enabled");
}
#else // __DAVAENGINE_WINDOWS__
void EnableFloatingPointExceptions()
{
// https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
// http://en.cppreference.com/w/cpp/numeric/fenv
// on iOS better in debug add flag -fsanitize=undefined
#ifdef __DAVAENGINE_ANDROID__
#ifndef FE_NOMASK_ENV
    Logger::FrameworkDebug("FPU exceptions not supported");
    // still try
    int result = feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW /* | FE_INEXACT */);
    DVASSERT(result != -1);
#else
    int result = feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW /* | FE_INEXACT */);
    DVASSERT(result != -1);
    Logger::FrameworkDebug("FPU exceptions enabled");
#endif
#endif // __DAVAENGINE_ANDROID__
}
#endif // non __DAVAENGINE_WINDOWS__
#else // __DAVAENGINE_DEBUG__
#ifdef __DAVAENGINE_WINDOWS__
void DisableFloatingPointExceptions()
{
    // do nothing on windows fpu exceptions disabled by default
}
#else // non __DAVAENGINE_WINDOWS__
void DisableFloatingPointExceptions()
{
    Logger::FrameworkDebug("disable FPU exceptions");
#ifdef __DAVAENGINE_ANDROID__
    int result = fedisableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW /* | FE_INEXACT */);
    DVASSERT(result != -1);
#else
// on iOS fpu exceptions disables by default
#endif
}
#endif 
#endif // not DAVA_ENGINE_DEBUG_FPU_EXCEPTIONS
} // end namespace debug_details

void Core::CreateSingletons()
{
    new Logger();

#ifdef DAVA_ENGINE_DEBUG_FPU_EXCEPTIONS
    fpu_exceptions::EnableFloatingPointExceptions();
#else
    fpu_exceptions::DisableFloatingPointExceptions();
#endif

    new AllocatorFactory();
    new JobManager();
    new FileSystem();
    FilePath::InitializeBundleName();

    FileSystem::Instance()->SetDefaultDocumentsDirectory();
    FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);

    Logger::Info("SoundSystem init start");
    try
    {
        new SoundSystem();
    }
    catch (std::exception& ex)
    {
        Logger::Info("%s", ex.what());
    }
    Logger::Info("SoundSystem init finish");

    if (isConsoleMode)
    {
        /*
            Disable all debug initialization messages in console mode
         */
        Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
    }

    DeviceInfo::InitializeScreenInfo();

    new LocalizationSystem();

    new SystemTimer();
    new Random();
    new AnimationManager();
    new FontManager();
    new UIControlSystem();
    new InputSystem();
    new PerformanceSettings();
    new VersionInfo();
    new ImageSystem();
    new FrameOcclusionQueryManager();

    new VirtualCoordinatesSystem();
    new RenderSystem2D();

#if defined(__DAVAENGINE_ANDROID__)
    new AssetsManager();
#endif
    
#if defined __DAVAENGINE_IPHONE__
// not used
#elif defined(__DAVAENGINE_ANDROID__)
    new AccelerometerAndroidImpl();
#endif //#if defined __DAVAENGINE_IPHONE__

    new UIScreenManager();

    Thread::InitMainThread();

    new DownloadManager();
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());

    new LocalNotificationController();

    RegisterDAVAClasses();

    new Net::NetCore();

#if defined(__DAVAENGINE_STEAM__) && (defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__))
    Steam::Init();
#endif

#ifdef __DAVAENGINE_AUTOTESTING__
    new AutotestingSystem();
#endif
}

// We do not create RenderManager until we know which version of render manager we want to create
void Core::CreateRenderer()
{
    DVASSERT(options->IsKeyExists("renderer"));
    rhi::Api renderer = static_cast<rhi::Api>(options->GetInt32("renderer"));

    if (options->IsKeyExists("rhi_threaded_frame_count"))
    {
        rendererParams.threadedRenderEnabled = true;
        rendererParams.threadedRenderFrameCount = options->GetInt32("rhi_threaded_frame_count");
    }

    rendererParams.maxIndexBufferCount = options->GetInt32("max_index_buffer_count");
    rendererParams.maxVertexBufferCount = options->GetInt32("max_vertex_buffer_count");
    rendererParams.maxConstBufferCount = options->GetInt32("max_const_buffer_count");
    rendererParams.maxTextureCount = options->GetInt32("max_texture_count");

    rendererParams.maxTextureSetCount = options->GetInt32("max_texture_set_count");
    rendererParams.maxSamplerStateCount = options->GetInt32("max_sampler_state_count");
    rendererParams.maxPipelineStateCount = options->GetInt32("max_pipeline_state_count");
    rendererParams.maxDepthStencilStateCount = options->GetInt32("max_depthstencil_state_count");
    rendererParams.maxRenderPassCount = options->GetInt32("max_render_pass_count");
    rendererParams.maxCommandBuffer = options->GetInt32("max_command_buffer_count");
    rendererParams.maxPacketListCount = options->GetInt32("max_packet_list_count");

    rendererParams.shaderConstRingBufferSize = options->GetInt32("shader_const_buffer_size");

    Renderer::Initialize(renderer, rendererParams);
}

void Core::ReleaseRenderer()
{
    Renderer::Uninitialize();
}

void Core::ReleaseSingletons()
{
#if defined(__DAVAENGINE_STEAM__) && (defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__))
    Steam::Deinit();
#endif
    // Finish network infrastructure
    // As I/O event loop runs in main thread so NetCore should run out loop to make graceful shutdown
    Net::NetCore::Instance()->Finish(true);
    Net::NetCore::Instance()->Release();

#ifdef __DAVAENGINE_AUTOTESTING__
    AutotestingSystem::Instance()->Release();
#endif

    LocalNotificationController::Instance()->Release();
    DownloadManager::Instance()->Release();
    PerformanceSettings::Instance()->Release();
    UIScreenManager::Instance()->Release();
    UIControlSystem::Instance()->Release();
    FontManager::Instance()->Release();
    AnimationManager::Instance()->Release();
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    Accelerometer::Instance()->Release();
//SoundSystem::Instance()->Release();
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    LocalizationSystem::Instance()->Release();
    //  Logger::FrameworkDebug("[Core::Release] successfull");
    FileSystem::Instance()->Release();
    SoundSystem::Instance()->Release();
    Random::Instance()->Release();
    FrameOcclusionQueryManager::Instance()->Release();
    VirtualCoordinatesSystem::Instance()->Release();
    RenderSystem2D::Instance()->Release();

    InputSystem::Instance()->Release();
    JobManager::Instance()->Release();
    VersionInfo::Instance()->Release();
    AllocatorFactory::Instance()->Release();
    Logger::Instance()->Release();
    ImageSystem::Instance()->Release();

#if defined(__DAVAENGINE_ANDROID__)
    AssetsManager::Instance()->Release();
#endif

    SystemTimer::Instance()->Release();
}

void Core::SetOptions(KeyedArchive* archiveOfOptions)
{
    SafeRelease(options);

    options = SafeRetain(archiveOfOptions);
    
#if defined(__DAVAENGINE_WIN_UAP__)
    screenOrientation = static_cast<eScreenOrientation>(options->GetInt32("orientation", SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE));
#elif !defined(__DAVAENGINE_ANDROID__) // defined(__DAVAENGINE_WIN_UAP__)
    //YZ android platform always use SCREEN_ORIENTATION_PORTRAIT and rotate system view and don't rotate GL view
    screenOrientation = static_cast<eScreenOrientation>(options->GetInt32("orientation", SCREEN_ORIENTATION_PORTRAIT));
#endif
}

KeyedArchive* Core::GetOptions()
{
    return options;
}

Core::eScreenOrientation Core::GetScreenOrientation()
{
    return screenOrientation;
}

Core::eScreenMode Core::GetScreenMode()
{
    return eScreenMode::FULLSCREEN;
}

bool Core::SetScreenMode(eScreenMode screenMode)
{
    return screenMode == eScreenMode::FULLSCREEN;
}

void Core::GetAvailableDisplayModes(List<DisplayMode>& availableModes)
{
}

DisplayMode Core::FindBestMode(const DisplayMode& requestedMode)
{
    List<DisplayMode> availableDisplayModes;
    GetAvailableDisplayModes(availableDisplayModes);

    DisplayMode bestMatchMode;

    bestMatchMode.refreshRate = -1;
    for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
    {
        DisplayMode& availableMode = *it;
        if ((availableMode.width == requestedMode.width) && (availableMode.height == requestedMode.height))
        {
            // if first mode found replace
            if (bestMatchMode.refreshRate == -1)
                bestMatchMode = availableMode;

            if (availableMode.bpp > bestMatchMode.bpp) // find best match with highest bits per pixel
            {
                bestMatchMode = availableMode;
            }
        }
    }

    if (bestMatchMode.refreshRate == -1) // haven't found any mode
    {
        int32 minDiffWidth = 0;
        int32 minDiffHeight = 0;
        float32 requestedAspect = (requestedMode.height > 0 ? float32(requestedMode.width) / float32(requestedMode.height) : 1.0f);
        float32 minDiffAspect = 0;

        for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
        {
            DisplayMode& availableMode = *it;

            int32 diffWidth = abs(availableMode.width - requestedMode.width);
            int32 diffHeight = abs(availableMode.height - requestedMode.height);

            float32 availableAspect = (availableMode.height > 0 ? float32(availableMode.width) / float32(availableMode.height) : 1.0f);
            float32 diffAspect = fabsf(availableAspect - requestedAspect);

            //          if (diffWidth >= 0 && diffHeight >= 0)
            {
                // if first mode found replace
                if (bestMatchMode.refreshRate == -1)
                {
                    minDiffWidth = diffWidth;
                    minDiffHeight = diffHeight;
                    minDiffAspect = diffAspect;
                }

                if (diffAspect <= (minDiffAspect + 0.01f))
                {
                    if ((diffAspect + 0.01f) < minDiffAspect)
                    {
                        // aspect changed, clear min diff
                        minDiffWidth = diffWidth;
                        minDiffHeight = diffHeight;
                    }

                    minDiffAspect = diffAspect;

                    //int32 curDiffWidth = availableMode.width - bestMatchMode.width;
                    //int32 curDiffHeight = availableMode.height - bestMatchMode.height;

                    //if (diffWidth + diffHeight <= curDiffWidth + curDiffHeight)
                    if (diffWidth + diffHeight <= minDiffWidth + minDiffHeight)
                    {
                        minDiffWidth = diffWidth;
                        minDiffHeight = diffHeight;

                        if (availableMode.bpp >= bestMatchMode.bpp) // find best match with highest bits per pixel
                        {
                            bestMatchMode = availableMode;
                        }
                    }
                }
            }
        }
    }

    if (bestMatchMode.refreshRate == -1) // haven't found any mode
    {
        int maxRes = 0;
        for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
        {
            DisplayMode& availableMode = *it;

            //int32 diffWidth = availableMode.width ;
            //int32 diffHeight = availableMode.height - requestedMode.height;
            if (availableMode.width + availableMode.height + availableMode.bpp > maxRes)
            {
                maxRes = availableMode.width + availableMode.height + availableMode.bpp;
                bestMatchMode = availableMode;
            }
        }
    }
    return bestMatchMode;
}

Vector2 Core::GetWindowSize()
{
    return Vector2(screenMetrics.width, screenMetrics.height);
}

void Core::Quit()
{
    Logger::FrameworkDebug("[Core::Quit] is not supported by platform implementation of core");
    exit(0);
}

void Core::SetApplicationCore(ApplicationCore* _core)
{
    core = _core;
}

ApplicationCore* Core::GetApplicationCore()
{
    return core;
}

void Core::SystemAppStarted()
{
    Logger::Info("Core::SystemAppStarted in");
    #if PROFILER_ENABLED
    profiler::EnsureInited();
    NAME_COUNTER(PROF__FRAME, "frame");
    NAME_COUNTER(PROF__FRAME_UPDATE, "frame-update");
    NAME_COUNTER(PROF__FRAME_DRAW, "frame-draw");
    NAME_COUNTER(PROF__FRAME_ENDFRAME, "frame-endframe");
    #endif

    if (VirtualCoordinatesSystem::Instance()->WasScreenSizeChanged())
    {
        VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
        /*  Question to Hottych: Does it really necessary here?
            RenderManager::Instance()->SetRenderOrientation(Core::Instance()->GetScreenOrientation());
         */
    }

    if (core != nullptr)
    {
        rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
        Core::Instance()->CreateRenderer();
        RenderSystem2D::Instance()->Init();
        core->OnAppStarted();
    }

    Logger::Info("Core::SystemAppStarted out");
}

void Core::SystemAppFinished()
{
    Logger::Info("Core::SystemAppFinished in");

    if (core != nullptr)
    {
        #if TRACER_ENABLED
        //        profiler::DumpEvents();
        profiler::SaveEvents("trace.json");
        #endif
        core->OnAppFinished();
        Core::Instance()->ReleaseRenderer();
    }

    Logger::Info("Core::SystemAppFinished out");
}

void Core::SystemProcessFrame()
{
    #if PROFILER_ENABLED
    profiler::EnsureInited();
    profiler::Start();
    START_TIMING(PROF__FRAME);
    #endif

    TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "Core::SystemProcessFrame");
    SCOPE_EXIT
    {
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "Core::SystemProcessFrame");
    };

#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
    static bool isInit = false;
    static EGLuint64NV frequency;
    static PFNEGLGETSYSTEMTIMENVPROC eglGetSystemTimeNV;
    static PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC eglGetSystemTimeFrequencyNV;
    if (!isInit)
    {
        eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC)eglGetProcAddress("eglGetSystemTimeNV");
        eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC)eglGetProcAddress("eglGetSystemTimeFrequencyNV");
        if (!eglGetSystemTimeNV || !eglGetSystemTimeFrequencyNV)
        {
            DVASSERT(!"Error export eglGetSystemTimeNV, eglGetSystemTimeFrequencyNV");
            exit(0);
        }
        frequency = eglGetSystemTimeFrequencyNV();
    }
    EGLuint64NV start = eglGetSystemTimeNV() / frequency;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__
    Stats::Instance()->BeginFrame();
    TIME_PROFILE("Core::SystemProcessFrame");
    
#if !defined(DAVA_NETWORK_DISABLE)
    // Poll for network I/O events here, not depending on Core active flag
    Net::NetCore::Instance()->Poll();
#endif
    // Give memory profiler chance to notify its subscribers about new frame
    DAVA_MEMORY_PROFILER_UPDATE();

    if (!core)
    {
        #if PROFILER_ENABLED
        profiler::Stop();
        #endif
        return;
    }

    if (!isActive)
    {
        LCP;
        #if PROFILER_ENABLED
        profiler::Stop();
        #endif
        return;
    }

    SystemTimer::Instance()->Start();

    /**
        Check if device not in lost state first / after that be
    */
    //  if (!Renderer::IsDeviceLost())
    {
        // #ifdef __DAVAENGINE_DIRECTX9__
        //      if(firstRun)
        //      {
        //          core->BeginFrame();
        //          firstRun = false;
        //      }
        // #else
        InputSystem::Instance()->OnBeforeUpdate();

        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "Core::BeginFrame")
        core->BeginFrame();
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "Core::BeginFrame")

//#endif

#if !defined(__DAVAENGINE_WINDOWS__) && !defined(__DAVAENGINE_WIN_UAP__) && !defined(__DAVAENGINE_MACOS__)
        // recalc frame inside begin / end frame
        VirtualCoordinatesSystem* vsc = VirtualCoordinatesSystem::Instance();
        if (vsc->WasScreenSizeChanged())
        {
            vsc->ScreenSizeChanged();
        }
#endif

        float32 frameDelta = SystemTimer::Instance()->FrameDelta();
        SystemTimer::Instance()->UpdateGlobalTime(frameDelta);

        if (Replay::IsRecord())
        {
            Replay::Instance()->RecordFrame(frameDelta);
        }
        if (Replay::IsPlayback())
        {
            UIControlSystem::Instance()->ReplayEvents();
            frameDelta = Replay::Instance()->PlayFrameTime();
            if (Replay::IsPlayback()) //can be unset in previous string
            {
                SystemTimer::Instance()->SetFrameDelta(frameDelta);
            }
        }

        START_TIMING(PROF__FRAME_UPDATE);

        LocalNotificationController::Instance()->Update();
        DownloadManager::Instance()->Update();

        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "JobManager::Update")
        JobManager::Instance()->Update();
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "JobManager::Update")

        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "Core::Update")
        core->Update(frameDelta);
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "Core::Update")

        InputSystem::Instance()->OnAfterUpdate();
        STOP_TIMING(PROF__FRAME_UPDATE);

        START_TIMING(PROF__FRAME_DRAW);

        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "Core::Draw")
        core->Draw();
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "Core::Draw")
        STOP_TIMING(PROF__FRAME_DRAW);

        START_TIMING(PROF__FRAME_ENDFRAME);
        TRACE_BEGIN_EVENT((uint32)Thread::GetCurrentId(), "", "Core::EndFrame")
        core->EndFrame();
        TRACE_END_EVENT((uint32)Thread::GetCurrentId(), "", "Core::EndFrame")

        STOP_TIMING(PROF__FRAME_ENDFRAME);
    }
    Stats::Instance()->EndFrame();
    globalFrameIndex++;
    
#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
    EGLuint64NV end = eglGetSystemTimeNV() / frequency;
    EGLuint64NV interval = end - start;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__

    #if PROFILER_ENABLED
    STOP_TIMING(PROF__FRAME);
    profiler::Stop();
    //profiler::Dump();
    profiler::DumpAverage();
    #endif

#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_WIN_UAP__) || defined(__DAVAENGINE_MACOS__)
    if (screenMetrics.initialized && screenMetrics.screenMetricsModified)
    {
        ApplyWindowSize();
    }
#endif // defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_WIN_UAP__)
}

void Core::GoBackground(bool isLock)
{
    if (core)
    {
        rhi::ShaderSourceCache::Save("~doc:/ShaderSource.bin");
        if (isLock)
        {
            core->OnDeviceLocked();
        }
        else
        {
            core->OnBackground();
        }
    }
}

void Core::GoForeground()
{
    if (core)
    {
        core->OnForeground();
    }
    Net::NetCore::Instance()->RestartAllControllers();
}

void Core::FocusLost()
{
    if (core)
    {
        core->OnFocusLost();
    }
}

void Core::FocusReceived()
{
    if (core)
    {
        core->OnFocusReceived();
    }
}

uint32 Core::GetGlobalFrameIndex()
{
    return globalFrameIndex;
}

void Core::SetCommandLine(int argc, char* argv[])
{
    commandLine.assign(argv, argv + argc);
}

void Core::SetCommandLine(Vector<String>&& args)
{
    commandLine = std::move(args);
}

void Core::SetCommandLine(const DAVA::String& cmdLine)
{
    commandLine.clear();
    bool inQuote = false;
    String currentParam;
    for (auto ch : cmdLine)
    {
        if (ch == '"')
        {
            inQuote = !inQuote;
        }
        else if (!inQuote && ch == ' ')
        {
            if (!currentParam.empty())
            {
                commandLine.push_back(currentParam);
            }
            currentParam.clear();
        }
        else
        {
            currentParam += ch;
        }
    }
    if (!currentParam.empty())
    {
        commandLine.push_back(currentParam);
    }
}

const Vector<String>& Core::GetCommandLine()
{
    return commandLine;
}

bool Core::IsConsoleMode()
{
    return isConsoleMode;
}

void* Core::GetNativeView() const
{
    return screenMetrics.nativeView;
}

void Core::SetNativeView(void* newNativeView)
{
    DVASSERT(nullptr != newNativeView);
    if (screenMetrics.nativeView != newNativeView)
    {
        screenMetrics.nativeView = newNativeView;
    }
}

void Core::EnableConsoleMode()
{
    isConsoleMode = true;
}

void Core::InitWindowSize(void* nativeView, float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    DVASSERT(nullptr != nativeView);
    DVASSERT(scaleX * scaleY);
    DVASSERT(!screenMetrics.initialized);
    screenMetrics.nativeView = nativeView;
    screenMetrics.width = width;
    screenMetrics.height = height;
    screenMetrics.scaleX = scaleX;
    screenMetrics.scaleY = scaleY;
    screenMetrics.screenMetricsModified = false;
    screenMetrics.initialized = true;

    rendererParams.window = screenMetrics.nativeView;
    rendererParams.width = static_cast<int32>(screenMetrics.width * screenMetrics.scaleX * screenMetrics.userScale);
    rendererParams.height = static_cast<int32>(screenMetrics.height * screenMetrics.scaleY * screenMetrics.userScale);

    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(static_cast<int32>(screenMetrics.width), static_cast<int32>(screenMetrics.height));
    virtSystem->SetPhysicalScreenSize(static_cast<int32>(rendererParams.width), static_cast<int32>(rendererParams.height));
    virtSystem->EnableReloadResourceOnResize(true);
}

void Core::WindowSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    if ((width == 0.f) || (height == 0.f) || (scaleX == 0.f) || (scaleY == 0.f))
    {
        return;
    }
    bool equal = true;
    equal &= FLOAT_EQUAL(width, screenMetrics.width);
    equal &= FLOAT_EQUAL(height, screenMetrics.height);
    equal &= FLOAT_EQUAL(scaleX, screenMetrics.scaleX);
    equal &= FLOAT_EQUAL(scaleY, screenMetrics.scaleY);

    if (!equal)
    {
        screenMetrics.width = width;
        screenMetrics.height = height;
        screenMetrics.scaleX = scaleX;
        screenMetrics.scaleY = scaleY;
        // if changedMetricsScreen == true, then on the next call SystemProcessFrame() update all sizes and systems, after set it false
        screenMetrics.screenMetricsModified = true;
    }
}

void Core::ApplyWindowSize()
{
    screenMetrics.screenMetricsModified = false;
    DVASSERT(Renderer::IsInitialized());
    int32 physicalWidth = static_cast<int32>(screenMetrics.width * screenMetrics.scaleX * screenMetrics.userScale);
    int32 physicalHeight = static_cast<int32>(screenMetrics.height * screenMetrics.scaleY * screenMetrics.userScale);

    // render reset
    rhi::ResetParam params;
    params.width = physicalWidth;
    params.height = physicalHeight;
    params.window = screenMetrics.nativeView;
    Renderer::Reset(params);

    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(static_cast<int32>(screenMetrics.width), static_cast<int32>(screenMetrics.height));
    virtSystem->SetPhysicalScreenSize(physicalWidth, physicalHeight);
    virtSystem->ScreenSizeChanged();
}

void Core::SetIsActive(bool _isActive)
{
    isActive = _isActive;
    Logger::Info("Core::SetIsActive %s", (_isActive) ? "TRUE" : "FALSE");
}

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__)
Core::eDeviceFamily Core::GetDeviceFamily()
{
    return DEVICE_DESKTOP;
}
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WINDOWS__)

uint32 Core::GetScreenDPI()
{
    return DPIHelper::GetScreenDPI();
}

void Core::SetIcon(int32 /*iconId*/){};

float32 Core::GetScreenScaleMultiplier() const
{
    return screenMetrics.userScale;
}

void Core::SetScreenScaleMultiplier(float32 multiplier)
{
    DVASSERT(multiplier > 0.f);
    if (!FLOAT_EQUAL(screenMetrics.userScale, multiplier))
    {
        screenMetrics.userScale = multiplier;
        screenMetrics.screenMetricsModified = true;
        options->SetFloat("userScreenScaleFactor", multiplier);
    }
}

float32 Core::GetScreenScaleFactor() const
{
    return (DeviceInfo::GetScreenInfo().scale * GetScreenScaleMultiplier());
}

void Core::SetWindowMinimumSize(float32 /*width*/, float32 /*height*/)
{
}

Vector2 Core::GetWindowMinimumSize() const
{
    return Vector2();
}

} // namespace DAVA
