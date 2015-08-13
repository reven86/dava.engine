/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "corecon.h"

// Force all versions of CoreCon in scope
#ifdef CCAPI_VERSIONED_H
#undef CCAPI_VERSIONED_H
#endif
#ifdef CORECON_VER
#undef CORECON_VER
#endif
#define CORECON_VER 11
#include "ccapi.h"

#ifdef CCAPI_VERSIONED_H
#undef CCAPI_VERSIONED_H
#endif
#ifdef CORECON_VER
#undef CORECON_VER
#endif
#define CORECON_VER 12
#include "ccapi.h"

#include <comdef.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"
#include "Debug/DVAssert.h"
#include "Utils/UTF8Utils.h"

template <typename ObjectContainerType, typename ContainerType, typename CollectionType>
static inline HRESULT collectionFor(const ComPtr<ContainerType> &container, 
                                    ComPtr<CollectionType> &collection)
{
    ComPtr<ObjectContainerType> objectContainer;
    HRESULT hr = container.As(&objectContainer);
    if (FAILED(hr))
        return hr;
    hr = objectContainer->EnumerateObjects(&collection);
    return hr;
}

void LogError(const DAVA::String& message)
{
    DAVA::String msg = "CoreCon. " + message;
    DVASSERT_MSG(false, msg);
}

class CoreConDevicePrivate
{
public:
    DAVA::String name;
    DAVA::String id;
    bool isEmulator;
    int version;

protected:
    CoreConDevicePrivate(int version) : version(version) { }
};

template <typename DeviceType>
class CoreConDevicePrivateVersioned : public CoreConDevicePrivate
{
public:
    CoreConDevicePrivateVersioned(int version) : CoreConDevicePrivate(version) { }
    ComPtr<DeviceType> handle;
};

CoreConDevice::CoreConDevice(int version)
{
    if (version == 11)
        d_ptr.reset(new CoreConDevicePrivateVersioned<ICcDevice_11>(version));
    else if (version == 12)
        d_ptr.reset(new CoreConDevicePrivateVersioned<ICcDevice_12>(version));
    else
        LogError("Invalid CoreCon version specified : " + std::to_string(version));
}

CoreConDevice::~CoreConDevice()
{
}

DAVA::String CoreConDevice::name() const
{
    return d_ptr->name;
}

DAVA::String CoreConDevice::id() const
{
    return d_ptr->id;
}

bool CoreConDevice::isEmulator() const
{
    return d_ptr->isEmulator;
}

HANDLE CoreConDevice::handle() const
{
    CoreConDevicePrivate* d = d_ptr.get();
    if (d_ptr->version == 11)
        return static_cast<const CoreConDevicePrivateVersioned<ICcDevice_11> *>(d)->handle.Get();
    if (d_ptr->version == 12)
        return static_cast<const CoreConDevicePrivateVersioned<ICcDevice_12> *>(d)->handle.Get();
    return 0;
}

class ComInitializer
{
protected:
    ComInitializer()
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
        {
            LogError("Failed to initialize COM.");
        }
    }
    virtual ~ComInitializer()
    {
        if (SUCCEEDED(hr))
            CoUninitialize();
    }
    HRESULT hr;
};

class CoreConServerPrivate : private ComInitializer
{
public:
    CoreConServerPrivate(int version) : version(version), langModule(0)
    {
    }
    ~CoreConServerPrivate()
    {
        for (auto x : devices) { delete x; }
        devices.clear();
    }

    virtual bool initialize() = 0;
    DAVA::String formatError(HRESULT hr) const;


    int version;
    DAVA::List<CoreConDevice *> devices;
    HMODULE langModule;

    template <typename T>
    static CoreConDevicePrivateVersioned<T> *deviceHandle(CoreConDevice *device)
    {
        return static_cast<CoreConDevicePrivateVersioned<T> *>(device->d_ptr.get());
    }
};

template <typename ServerType, typename DataStoreType, typename PlatformType,
          typename PlatformContainerType, typename CollectionType, typename DeviceType, typename DeviceContainerType,
          typename ObjectType, typename ObjectContainerType, typename PropertyType, typename PropertyContainerType>
class CoreConServerPrivateVersioned : public CoreConServerPrivate
{
public:
    CoreConServerPrivateVersioned(CoreConServer *server, int version)
        : CoreConServerPrivate(version)
    {
        HRESULT hr = E_FAIL;
        if (version == 11)
            hr = CoCreateInstance(CLSID_ConMan_11, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&handle));
        else if (version == 12)
            hr = CoCreateInstance(CLSID_ConMan_12, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&handle));
        else
            LogError("Invalid CoreCon version specified: " + std::to_string(version));

        if (FAILED(hr))
            LogError("Failed to initialize connection server. " + formatError(hr));

        // The language module is available as long as the above succeeded
        langModule = GetModuleHandle(L"conmanui");
    }

    bool initialize()
    {
        ComPtr<DataStoreType> dataStore;
        HRESULT hr = handle->GetDatastore(GetUserDefaultLCID(), &dataStore);
        if (FAILED(hr)) {
            LogError("Failed to obtain the data store. " + formatError(hr));
            return false;
        }

        ComPtr<PlatformContainerType> platformContainer;
        hr = dataStore->get_PlatformContainer(&platformContainer);
        if (FAILED(hr)) {
            LogError("Failed to obtain the platform container. " + formatError(hr));
            return false;
        }

        ComPtr<CollectionType> platformCollection;
        hr = collectionFor<ObjectContainerType>(platformContainer, platformCollection);
        if (FAILED(hr)) {
            LogError("Failed to obtain the platform collection. " + formatError(hr));
            return false;
        }

        long platformCount;
        hr = platformCollection->get_Count(&platformCount);
        if (FAILED(hr)) {
            LogError("Failed to obtain the platform object count. " + formatError(hr));
            return false;
        }
        for (long platformIndex = 0; platformIndex < platformCount; ++platformIndex) {
            ComPtr<ObjectType> platformObject;
            hr = platformCollection->get_Item(platformIndex, &platformObject);
            if (FAILED(hr)) {
                LogError("\1. " + std::to_string(platformIndex));
                continue;
            }

            ComPtr<PlatformType> platform;
            hr = platformObject.As(&platform);
            if (FAILED(hr)) {
                LogError("\1. " + std::to_string(platformIndex));
                continue;
            }

            ComPtr<DeviceContainerType> deviceContainer;
            hr = platform->get_DeviceContainer(&deviceContainer);
            if (FAILED(hr)) {
                LogError("Failed to obtain the device container. " + formatError(hr));
                continue;
            }

            ComPtr<CollectionType> deviceCollection;
            hr = collectionFor<ObjectContainerType>(deviceContainer, deviceCollection);
            if (FAILED(hr)) {
                LogError("Failed to obtain the device object collection. " + formatError(hr));
                continue;
            }

            long deviceCount;
            hr = deviceCollection->get_Count(&deviceCount);
            if (FAILED(hr)) {
                LogError("Failed to obtain the device object count. " + formatError(hr));
                continue;
            }
            for (long deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
                std::unique_ptr<CoreConDevice> device(new CoreConDevice(version));

                ComPtr<ObjectType> deviceObject;
                hr = deviceCollection->get_Item(deviceIndex, &deviceObject);
                if (FAILED(hr)) {
                    LogError("Failed to obtain the device object at index: " + std::to_string(platformIndex));
                    continue;
                }

                hr = deviceObject.As(&deviceHandle<DeviceType>(device.get())->handle);
                if (FAILED(hr)) {
                    LogError("Failed to confirm a device from the object at index: " + std::to_string(deviceIndex));
                    continue;
                }

                _bstr_t deviceId;
                hr = deviceObject->get_ID(deviceId.GetAddress());
                if (FAILED(hr)) {
                    LogError("Failed to obtain device id at index: " + std::to_string(deviceIndex));
                    continue;
                }
                deviceHandle<DeviceType>(device.get())->id = deviceId;
                _bstr_t deviceName;
                hr = deviceObject->get_Name(deviceName.GetAddress());
                if (FAILED(hr)) {
                    LogError("Failed to obtain device name at index: " + std::to_string(deviceIndex));
                    continue;
                }
                deviceHandle<DeviceType>(device.get())->name = deviceName;

                ComPtr<PropertyContainerType> propertyContainer;
                hr = deviceObject->get_PropertyContainer(&propertyContainer);
                if (FAILED(hr)) {
                    LogError("Failed to obtain a property container at index: " + std::to_string(deviceIndex));
                    continue;
                }

                ComPtr<CollectionType> propertyCollection;
                hr = collectionFor<ObjectContainerType>(propertyContainer, propertyCollection);
                if (FAILED(hr)) {
                    LogError("Failed to obtain property collection of device at index: " + std::to_string(deviceIndex));
                    continue;
                }

                bool isPseudoDevice = false;
                long propertyCount;
                hr = propertyCollection->get_Count(&propertyCount);
                if (FAILED(hr)) {
                    LogError("Failed to obtain property count of device at index: " + std::to_string(deviceIndex));
                    continue;
                }

                for (long propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
                    ComPtr<ObjectType> propertyObject;
                    hr = propertyCollection->get_Item(propertyIndex, &propertyObject);
                    if (FAILED(hr)) {
                        LogError("Failed to obtain property at index: " + std::to_string(propertyIndex));
                        continue;
                    }

                    _bstr_t id;
                    hr = propertyObject->get_ID(id.GetAddress());
                    if (FAILED(hr)) {
                        LogError("Failed to obtain property id at index: " + std::to_string(propertyIndex));
                        continue;
                    }

                    ComPtr<PropertyType> property;
                    hr = propertyObject.As(&property);
                    if (FAILED(hr)) {
                        LogError("Failed to cast the property object at index: " + std::to_string(propertyIndex));
                        continue;
                    }

                    if (id == _bstr_t(L"IsPseudoDevice")) {
                        _bstr_t value;
                        hr = property->get_Value(value.GetAddress());
                        if (FAILED(hr)) {
                            LogError("Failed to cast the property value at index: " + std::to_string(propertyIndex));
                            continue;
                        }
                        if (value == _bstr_t(L"true")) {
                            isPseudoDevice = true;
                            break; // No need to look at this device further
                        }
                    }

                    if (id == _bstr_t(L"Emulator")) {
                        _bstr_t value;
                        hr = property->get_Value(value.GetAddress());
                        if (FAILED(hr)) {
                            LogError("Failed to cast the property value at index: " + std::to_string(propertyIndex));
                            continue;
                        }
                        deviceHandle<DeviceType>(device.get())->isEmulator = value == _bstr_t(L"true");
                    }
                }

                if (!isPseudoDevice)
                    devices.push_back(device.release());
            }
        }
        return true;
    }

    ComPtr<ServerType> handle;
};

typedef CoreConServerPrivateVersioned<ICcServer_11, ICcDatastore_11, ICcPlatform_11,
            ICcPlatformContainer_11, ICcCollection_11, ICcDevice_11, ICcDeviceContainer_11,
            ICcObject_11, ICcObjectContainer_11,
            ICcProperty_11, ICcPropertyContainer_11> CoreConServerPrivate_11;

typedef CoreConServerPrivateVersioned<ICcServer_12, ICcDatastore_12, ICcPlatform_12,
            ICcPlatformContainer_12, ICcCollection_12, ICcDevice_12, ICcDeviceContainer_12,
            ICcObject_12, ICcObjectContainer_12,
            ICcProperty_12, ICcPropertyContainer_12> CoreConServerPrivate_12;

CoreConServer::CoreConServer(int version)
{
    if (version == 11)
        d_ptr.reset(new CoreConServerPrivate_11(this, version));
    else if (version == 12)
        d_ptr.reset(new CoreConServerPrivate_12(this, version));
    else
    {
        DAVA::String msg =
            "CoreCon. Invalid CoreCon version specified: " + std::to_string(version);
        DVASSERT_MSG(false, msg);
    }

    initialize();
}

CoreConServer::~CoreConServer()
{
}

HANDLE CoreConServer::handle() const
{
    const CoreConServerPrivate* d = d_ptr.get();

    if (d->version == 11)
        return static_cast<const CoreConServerPrivate_11 *>(d)->handle.Get();
    if (d->version == 12)
        return static_cast<const CoreConServerPrivate_12 *>(d)->handle.Get();
    return 0;
}

DAVA::List<CoreConDevice *> CoreConServer::devices() const
{
    const CoreConServerPrivate* d = d_ptr.get();
    return d->devices;
}

bool CoreConServer::initialize()
{
    CoreConServerPrivate* d = d_ptr.get();

    if (!d || !handle())
        return false;

    if (!d->devices.empty())
        return true;

    return d->initialize();
}

DAVA::String CoreConServer::formatError(HRESULT hr) const
{
    const CoreConServerPrivate* d = d_ptr.get();
    return d->formatError(hr);
}

DAVA::String CoreConServerPrivate::formatError(HRESULT hr) const
{
    wchar_t error[1024];
    HMODULE module = 0;
    DWORD origin = HRESULT_FACILITY(hr);
    if (origin == 0x973 || origin == 0x974 || origin == 0x103)
        module = langModule;
    if (module) {
        int length = LoadString(module, HRESULT_CODE(hr), error, sizeof(error)/sizeof(wchar_t));
        if (length)
            return DAVA::UTF8Utils::EncodeToUTF8(DAVA::WideString(error, length));
    }
    return "Error code: " + std::to_string(hr);
}
