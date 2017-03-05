#include "Modules/FindResultsModule/FindResultsModule.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(FindResultsModule)
{
    ReflectionRegistrator<FindResultsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindResultsModule::PostInit()
{
    findResultsWidget = new FindResultsWidget();

    const char* title = "Find Results";
    TArc::DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    TArc::PanelKey panelKey(title, panelInfo);
    GetUI()->AddView(QEGlobal::windowKey, panelKey, findResultsWidget);

    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToControl, MakeFunction(this, &FindResultsModule::JumpToControl));
    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToPackage, MakeFunction(this, &FindResultsModule::JumpToPackage));

    RegisterOperations();

    TArc::ContextAccessor* accessor = GetAccessor();
    projectDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);
}

void FindResultsModule::JumpToControl(const FilePath& packagePath, const String& controlName)
{
    using namespace DAVA;
    using namespace TArc;

    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    const QString& name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void FindResultsModule::JumpToPackage(const FilePath& packagePath)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

void FindResultsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::FindInProject.ID, this, &FindResultsModule::FindInProject);
    RegisterOperation(QEGlobal::FindInDocument.ID, this, &FindResultsModule::FindInDocument);
}

void FindResultsModule::FindInProject(std::shared_ptr<FindFilter> filter)
{
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    ProjectData* projectData = globalContext->GetData<ProjectData>();
    FileSystemCacheData* fileSystemCacheData = globalContext->GetData<FileSystemCacheData>();

    if (projectData != nullptr)
    {
        findResultsWidget->Find(filter, projectData, fileSystemCacheData->GetFiles("yaml"));
    }
}

void FindResultsModule::FindInDocument(std::shared_ptr<FindFilter> filter)
{
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    if (activeContext != nullptr)
    {
        ProjectData* projectData = activeContext->GetData<ProjectData>();
        DocumentData* documentData = activeContext->GetData<DocumentData>();

        findResultsWidget->Find(filter, projectData, documentData);
    }
}

void FindResultsModule::OnDataChanged(const TArc::DataWrapper& wrapper, const Vector<Any>& fields)
{
    findResultsWidget->StopFind();
    findResultsWidget->ClearResults();
}

DECL_GUI_MODULE(FindResultsModule);
