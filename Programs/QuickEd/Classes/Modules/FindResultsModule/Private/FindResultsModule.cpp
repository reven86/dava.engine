#include "Modules/FindResultsModule/FindResultsModule.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FindResultsModule)
{
    DAVA::ReflectionRegistrator<FindResultsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindResultsModule::PostInit()
{
    findResultsWidget = new FindResultsWidget();

    const char* title = "Find Results";
    DAVA::TArc::DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    DAVA::TArc::PanelKey panelKey(title, panelInfo);
    GetUI()->AddView(QEGlobal::windowKey, panelKey, findResultsWidget);

    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToControl, DAVA::MakeFunction(this, &FindResultsModule::JumpToControl));
    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToPackage, DAVA::MakeFunction(this, &FindResultsModule::JumpToPackage));

    RegisterOperations();

    DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    projectDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);
}

void FindResultsModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    using namespace DAVA;
    using namespace TArc;

    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    QString name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void FindResultsModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

void FindResultsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::FindInProject.ID, this, &FindResultsModule::FindInProject);
}

void FindResultsModule::FindInProject(std::shared_ptr<FindFilter> filter)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    if (activeContext == nullptr)
    {
        return;
    }

    ProjectData* projectData = activeContext->GetData<ProjectData>();
    FileSystemCacheData* fileSystemCacheData = activeContext->GetData<FileSystemCacheData>();

    findResultsWidget->Find(filter, projectData, fileSystemCacheData->GetFiles("yaml"));
}

void FindResultsModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    findResultsWidget->StopFind();
    findResultsWidget->ClearResults();
}

DECL_GUI_MODULE(FindResultsModule);
