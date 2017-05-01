#include "Modules/IssueNavigatorModule/IssueNavigatorModule.h"

#include "Modules/IssueNavigatorModule/IssueNavigatorWidget.h"
#include "Modules/IssueNavigatorModule/LayoutIssuesHandler.h"

#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/QtBoxLayouts.h>

DAVA_VIRTUAL_REFLECTION_IMPL(IssueNavigatorModule)
{
    DAVA::ReflectionRegistrator<IssueNavigatorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void IssueNavigatorModule::PostInit()
{
    InitUI();
    RegisterOperations();
}

void IssueNavigatorModule::InitUI()
{
    using namespace DAVA::TArc;
    const char* title = "Issue Navigator";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    PanelKey key(title, panelInfo);

    widget = new IssueNavigatorWidget(GetAccessor());
    GetUI()->AddView(DAVA::TArc::mainWindowKey, key, widget);

    DAVA::int32 sectionId = 0;
    layoutIssuesHandler.reset(new LayoutIssuesHandler(sectionId++, widget));
}

void IssueNavigatorModule::RegisterOperations()
{
    //    RegisterOperation(int operationID, Cls *object, Ret (Cls::*fn)(Args...) const)
}

DECL_GUI_MODULE(IssueNavigatorModule);
