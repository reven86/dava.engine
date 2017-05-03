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
    using namespace DAVA::TArc;
    const char* title = "Issue Navigator";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    PanelKey key(title, panelInfo);

    widget = new IssueNavigatorWidget(GetAccessor());
    connections.AddConnection(widget, &IssueNavigatorWidget::JumpToControl, MakeFunction(this, &IssueNavigatorModule::JumpToControl));
    connections.AddConnection(widget, &IssueNavigatorWidget::JumpToPackage, MakeFunction(this, &IssueNavigatorModule::JumpToPackage));

    GetUI()->AddView(DAVA::TArc::mainWindowKey, key, widget);

    DAVA::int32 sectionId = 0;
    layoutIssuesHandler.reset(new LayoutIssuesHandler(GetAccessor(), sectionId++, widget));
}

void IssueNavigatorModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    const QString& name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void IssueNavigatorModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

DECL_GUI_MODULE(IssueNavigatorModule);
