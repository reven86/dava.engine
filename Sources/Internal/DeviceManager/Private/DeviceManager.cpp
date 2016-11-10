#include "Engine/DeviceManager.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

#if defined(__DAVAENGINE_QT__)
#include "Engine/Private/Qt/DeviceManagerImplQt.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Engine/Private/Win32/DeviceManagerImplWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Engine/Private/UWP/DeviceManagerImplWin10.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Engine/Private/OsX/DeviceManagerImplMac.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Engine/Private/iOS/DeviceManagerImplIos.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Engine/Private/Android/DeviceManagerImplAndroid.h"
#else
#error "DeviceManager: unknown platform"
#endif

namespace DAVA
{
DeviceManager::DeviceManager(Private::EngineBackend* engineBackend)
    : impl(new Private::DeviceManagerImpl(this, engineBackend->GetDispatcher()))
{
}

DeviceManager::~DeviceManager() = default;

void DeviceManager::UpdateDisplayConfig()
{
    impl->UpdateDisplayConfig();
}

void DeviceManager::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;
    if (e.type == MainDispatcherEvent::DISPLAY_CONFIG_CHANGED)
    {
        size_t count = e.displayConfigEvent.count;
        DisplayInfo* displayInfo = e.displayConfigEvent.displayInfo;

        displays.resize(count);
        std::move(displayInfo, displayInfo + count, begin(displays));

        delete[] displayInfo;

        displayConfigChanged.Emit();
    }
}

} // namespace DAVA
