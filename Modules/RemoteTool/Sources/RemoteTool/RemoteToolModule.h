#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class FieldBinder;
}
}

class DeviceListController;
class DeviceListWidget;

/**
    Module allows to inspect remote applications, retrieve logs and memory profiling data from them.

    Module is a TArc-based. Being added into application (which is also has to be tarc-based),
    it adds main menu option "Remote" right before "Help" option and one action "Remote Inspect" inside of it.
    Activating "Remote Inspect" invokes non-modal window, in which you can browse all available
    remote applications and connect to any of them to retrieve logs or memory profiling data.
    
    To use module in your application you only should add module
    in CMakeLists.txt of your application project:

    \code
        find_package( RemoteTool REQUIRED )
    \endcode
*/
class RemoteToolModule : public DAVA::TArc::ClientModule
{
public:
    ~RemoteToolModule();

private:
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void Show();

private:
    DAVA::TArc::QtConnections connections;
    QPointer<DeviceListController> deviceListController;
    QPointer<DeviceListWidget> deviceListWidget;

    DAVA_VIRTUAL_REFLECTION(RemoteToolModule, DAVA::TArc::ClientModule);
};
