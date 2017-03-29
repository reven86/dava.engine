#include "RemoteTool/RemoteToolModule.h"
#include "RemoteTool/Private/DeviceListController.h"
#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Functional/Function.h>
#include <Reflection/ReflectionRegistrator.h>

RemoteToolModule::RemoteToolModule(DAVA::TArc::WindowKey windowKey, DAVA::TArc::InsertionParams insertionParams /* = DAVA::TArc::InsertionParams()*/)
    : windowKey(windowKey)
    , insertionParams(insertionParams)
{
}

void RemoteToolModule::PostInit()
{
    using namespace DAVA::TArc;

    // create menu bar action "Remote"
    QAction* menuRemoteAction = new QAction(QStringLiteral("Remote"), nullptr);
    ActionPlacementInfo menuRemotePlacement(CreateMenuPoint("", insertionParams));
    GetUI()->AddAction(windowKey, menuRemotePlacement, menuRemoteAction);

    QtAction* toolAction = new QtAction(GetAccessor(), QString("Remote Inspect"));

    // create "Remote Inspect" action inside of "Remote"
    ActionPlacementInfo toolPlacement(CreateMenuPoint("Remote"));
    GetUI()->AddAction(windowKey, toolPlacement, toolAction);

    connections.AddConnection(toolAction, &QAction::triggered, DAVA::MakeFunction(this, &RemoteToolModule::Show));
}

void RemoteToolModule::Show()
{
    if (!deviceListController)
    {
        DeviceListWidget* w = new DeviceListWidget();
        w->setAttribute(Qt::WA_DeleteOnClose);

        deviceListController = new DeviceListController(w);
        deviceListController->SetView(w);
    }
    deviceListController->ShowView();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoteToolModule)
{
    DAVA::ReflectionRegistrator<RemoteToolModule>::Begin()
    .ConstructorByPointer<DAVA::TArc::WindowKey, DAVA::TArc::InsertionParams>()
    .End();
}
