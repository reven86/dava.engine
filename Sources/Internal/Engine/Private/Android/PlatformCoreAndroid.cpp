#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Base/Exception.h"
#include "Engine/Window.h"
#include "Engine/Android/JNIBridge.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/Android/AndroidBridge.h"
#include "Engine/Private/Android/Window/WindowBackendAndroid.h"

#include "Debug/Backtrace.h"
#include "Input/InputSystem.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

extern int DAVAMain(DAVA::Vector<DAVA::String> cmdline);
extern DAVA::Private::AndroidBridge* androidBridge;

extern "C"
{

JNIEXPORT void JNICALL Java_com_dava_engine_DavaGamepadManager_nativeOnGamepadAdded(JNIEnv* env, jclass jclazz, jint deviceId, jstring name, jboolean hasTriggerButtons)
{
    using namespace DAVA;
    String deviceName = JNI::JavaStringToString(name, env);
    androidBridge->core->OnGamepadAdded(deviceId, deviceName, hasTriggerButtons == JNI_TRUE);
}

JNIEXPORT void JNICALL Java_com_dava_engine_DavaGamepadManager_nativeOnGamepadRemoved(JNIEnv* env, jclass jclazz, jint deviceId)
{
    androidBridge->core->OnGamepadRemoved(deviceId);
}

} // extern "C"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(engineBackend)
    , mainDispatcher(engineBackend->GetDispatcher())
{
    AndroidBridge::AttachPlatformCore(this);
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    engineBackend->OnGameLoopStarted();

    while (!quitGameThread)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        int32 fps = engineBackend->OnFrame();

        uint64 frameEndTime = SystemTimer::Instance()->AbsoluteMS();
        uint32 frameDuration = static_cast<uint32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        Thread::Sleep(sleep);
    }

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void PlatformCore::PrepareToQuit()
{
    AndroidBridge::PostQuitToActivity();
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

WindowBackend* PlatformCore::ActivityOnCreate()
{
    Window* primaryWindow = engineBackend->InitializePrimaryWindow();
    WindowBackend* primaryWindowBackend = primaryWindow->GetBackend();
    return primaryWindowBackend;
}

void PlatformCore::ActivityOnResume()
{
    mainDispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
}

void PlatformCore::ActivityOnPause()
{
    // Blocking call !!!
    mainDispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED));
}

void PlatformCore::ActivityOnDestroy()
{
    // Dispatch application termination request initiated by system, i.e. android activity is finishing
    // Do nonblocking call as Java part will wait until native thread is finished
    engineBackend->PostAppTerminate(true);
}

void PlatformCore::GameThread()
{
    try
    {
        DAVAMain(std::move(androidBridge->cmdargs));
    }
    catch (const Exception& e)
    {
        StringStream ss;
        ss << "!!! Unhandled DAVA::Exception at `" << e.file << "`: " << e.line << std::endl;
        ss << Debug::GetBacktraceString(e.callstack) << std::endl;
        Logger::PlatformLog(Logger::LEVEL_ERROR, ss.str().c_str());
        throw;
    }
    catch (const std::exception& e)
    {
        StringStream ss;
        ss << "!!! Unhandled std::exception in DAVAMain: " << e.what() << std::endl;
        Logger::PlatformLog(Logger::LEVEL_ERROR, ss.str().c_str());
        throw;
    }
}

void PlatformCore::OnGamepadAdded(int32 deviceId, const String& name, bool hasTriggerButtons)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadAddedEvent(deviceId));
}

void PlatformCore::OnGamepadRemoved(int32 deviceId)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateGamepadRemovedEvent(deviceId));
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
