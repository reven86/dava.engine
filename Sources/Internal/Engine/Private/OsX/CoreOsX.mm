#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/CoreOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/OsX/CoreOsXObjcBridge.h"

namespace DAVA
{
namespace Private
{
CoreOsX::CoreOsX(EngineBackend* e)
    : engineBackend(e)
    , objcBridge(new CoreOsXObjcBridge(this))
{
}

CoreOsX::~CoreOsX()
{
    delete objcBridge;
}

Vector<String> CoreOsX::GetCommandLine(int argc, char* argv[])
{
    return Vector<String>();
}

void CoreOsX::Init()
{
}

void CoreOsX::Run()
{
    objcBridge->InitNSApplication();
    NSApplicationMain(0, nullptr);
}

void CoreOsX::Quit()
{
    objcBridge->Quit();
}

WindowOsX* CoreOsX::CreateNativeWindow(WindowBackend* w, float32 width, float32 height)
{
    WindowOsX* nativeWindow = new WindowOsX(engineBackend, w);
    if (!nativeWindow->CreateWindow(width, height))
    {
        delete nativeWindow;
        nativeWindow = nullptr;
    }
    return nativeWindow;
}

void CoreOsX::DestroyNativeWindow(WindowBackend* w)
{
    w->GetNativeWindow()->DestroyWindow();
}

int32 CoreOsX::OnFrame()
{
    return engineBackend->OnFrame();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
