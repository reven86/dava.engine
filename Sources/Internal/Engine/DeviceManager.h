#pragma once

#include "Base/BaseTypes.h"
#include "Engine/DeviceManagerTypes.h"
#include "Functional/Signal.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
struct DeviceManagerImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup device_manager Device Manager
*/
class DeviceManager final
{
public:
    DeviceManager(Private::EngineBackend* engineBackend);
    ~DeviceManager();

    const DisplayInfo& GetPrimaryDisplay() const;
    const Vector<DisplayInfo>& GetDisplays() const;
    size_t GetDisplayCount() const;

    Signal<> displayConfigChanged;

private:
    void HandleEvent(const Private::MainDispatcherEvent& e);

    Vector<DisplayInfo> displays;
    std::unique_ptr<Private::DeviceManagerImpl> impl;

    friend struct Private::DeviceManagerImpl;
};

inline const DisplayInfo& DeviceManager::GetPrimaryDisplay() const
{
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
