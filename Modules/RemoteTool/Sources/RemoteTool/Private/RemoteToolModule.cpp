#include "RemoteTool/RemoteToolModule.h"
#include "RemoteTool/Private/DeviceListController.h"
#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Functional/Function.h>
#include <Reflection/ReflectionRegistrator.h>

RemoteToolModule::~RemoteToolModule()
{
    DVASSERT(deviceListWidget == nullptr);
    if (deviceListWidget != nullptr)
    {
        delete deviceListWidget;
    }
}

void RemoteToolModule::PostInit()
{
    using namespace DAVA::TArc;

    // create menu bar action "Remote", insert before "Help"
    QAction* menuRemoteAction = new QAction(QStringLiteral("Remote"), nullptr);
    ActionPlacementInfo menuRemotePlacement(CreateMenuPoint("", { DAVA::TArc::InsertionParams::eInsertionMethod::BeforeItem, MenuItems::menuHelp }));
    GetUI()->AddAction(mainWindowKey, menuRemotePlacement, menuRemoteAction);

    QtAction* toolAction = new QtAction(GetAccessor(), QString("Remote Inspect"));

    // create "Remote Inspect" action inside of "Remote"
    ActionPlacementInfo toolPlacement(CreateMenuPoint("Remote"));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, toolPlacement, toolAction);

    connections.AddConnection(toolAction, &QAction::triggered, DAVA::MakeFunction(this, &RemoteToolModule::Show));
}

void RemoteToolModule::Show()
{
    if (!deviceListController)
    {
        deviceListWidget = new DeviceListWidget();
        deviceListWidget->setAttribute(Qt::WA_DeleteOnClose);

        deviceListController = new DeviceListController(deviceListWidget);
        deviceListController->SetView(deviceListWidget);
    }
    deviceListController->ShowView();
}

void RemoteToolModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    if (key == DAVA::TArc::mainWindowKey && deviceListWidget != nullptr)
    {
        delete deviceListWidget;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoteToolModule)
{
    DAVA::ReflectionRegistrator<RemoteToolModule>::Begin()
    .ConstructorByPointer()
    .End();
}
