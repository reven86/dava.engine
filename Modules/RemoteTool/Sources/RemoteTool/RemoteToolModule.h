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

/**
    Module allows to inspect remote applications, retrieve logs and memory profiling data from them.

    Module is a TArc-based. Being added into application (which is also has to be tarc-based),
    it adds main menu option "Remote" and one action "Remote Inspect" inside of it.
    Activating "Remote Inspect" invokes non-modal window, in which you can browse all available
    remote applications and connect to any of them to retrieve logs or memory profiling data.
    
    To use module in your application, follow steps below:

        1. add module in CMakeLists.txt of your application project
        \code
            find_package( RemoteTool REQUIRED )
        \endcode

        2. add module in your tarc-based application inside of CreateModules() function.
        e.g.:
        \code
            void MyApplication::CreateModules(DAVA::TArc::Core* tarcCore) const
            {
                .....
                // say you want to add "Remote" menu item right after "Tools" menu item
                DAVA::TArc::InsertionParams insertionParams(DAVA::TArc::InsertionParams::eInsertionMethod::AfterItem, QStringLiteral("Tools"));
                tarcCore->CreateModule<RemoteToolModule>(MyMainWindowKey, insertionParams);
            }
        \endcode
*/
class RemoteToolModule : public DAVA::TArc::ClientModule
{
public:
    /** constructs module
    `windowKey` is token of TArc window in which module GUI will be placed
    `params` specifies where in main menu to insert new "Remote" item
    */
    RemoteToolModule(DAVA::TArc::WindowKey windowKey, DAVA::TArc::InsertionParams params = DAVA::TArc::InsertionParams());

protected:
    void PostInit() override;

private:
    void Show();

private:
    DAVA::TArc::WindowKey windowKey;
    DAVA::TArc::InsertionParams insertionParams;
    DAVA::TArc::QtConnections connections;
    QPointer<DeviceListController> deviceListController;

    DAVA_VIRTUAL_REFLECTION(RemoteToolModule, DAVA::TArc::ClientModule);
};
