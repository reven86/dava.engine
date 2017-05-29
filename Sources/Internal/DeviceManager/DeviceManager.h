#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "DeviceManager/DeviceManagerTypes.h"
#include "Functional/Signal.h"

#include "Engine/Private/EnginePrivateFwd.h"

/**
    \defgroup device_manager Device Manager
*/

namespace DAVA
{
namespace Private
{
struct DeviceManagerImpl;
}

/**
    \ingroup device_manager

    Class which keeps current device configuration, listens for device addition, removal or devices' properties changes.
    Application can subscribe to appropriate signals to receive notification about configuration changes.

    \todo For now `DeviceManager` observes only display devices and cpu stats, further add other devices (input, storage, maybe network).
*/
class DeviceManager final
{
private:
    DeviceManager(Private::EngineBackend* engineBackend);
    ~DeviceManager();

public:
    /** Get primary display as reported by system */
    const DisplayInfo& GetPrimaryDisplay() const;

    /** Get displays which are available now */
    const Vector<DisplayInfo>& GetDisplays() const;

    /** Get total display count */
    size_t GetDisplayCount() const;

    Signal<> displayConfigChanged; //<! Emited when display has been added/removed or properties of any display has changed

    /**
        Get CPU temperature in celsius.
        \note Only supported on Android for now.
    */
    float32 GetCpuTemperature() const;

private:
    void UpdateDisplayConfig();
    void HandleEvent(const Private::MainDispatcherEvent& e);

    Vector<DisplayInfo> displays;
    std::unique_ptr<Private::DeviceManagerImpl> impl;

    friend class Private::EngineBackend;
    friend struct Private::DeviceManagerImpl;
};

inline const DisplayInfo& DeviceManager::GetPrimaryDisplay() const
{
    // DeviceManagerImpl always places primary display as first element
    return displays[0];
}

inline const Vector<DisplayInfo>& DeviceManager::GetDisplays() const
{
    return displays;
}

inline size_t DeviceManager::GetDisplayCount() const
{
    return displays.size();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
