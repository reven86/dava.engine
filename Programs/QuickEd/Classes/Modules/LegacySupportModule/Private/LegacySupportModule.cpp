#include "Modules/LegacySupportModule/LegacySupportModule.h"
#include "Modules/DocumentsModule/Document.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/ProjectModule/Project.h"

#include "UI/mainwindow.h"
#include "UI/ProjectView.h"
#include "UI/DocumentGroupView.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/ContextAccessor.h>

DAVA_VIRTUAL_REFLECTION_IMPL(LegacySupportModule)
{
    DAVA::ReflectionRegistrator<LegacySupportModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void LegacySupportModule::PostInit()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();

    projectDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);
}

void LegacySupportModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA::TArc;
    if (wrapper.HasData() && fields.empty() == false)
    {
        return;
    }
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    MainWindow* mainWindow = globalContext->GetData<MainWindow>();
    MainWindow::ProjectView* projectView = mainWindow->GetProjectView();
    MainWindow::DocumentGroupView* documentGroupView = projectView->GetDocumentGroupView();
    if (wrapper.HasData())
    {
        project.reset(new Project(projectView, accessor));
    }
    else
    {
        project.release();
    }
    projectView->OnProjectChanged(project.get());
    documentGroupView->SetProject(project.get());
}

void LegacySupportModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    std::unique_ptr<Document> document(new Document(accessor, context->GetID()));
    context->CreateData(std::move(document));
}

void LegacySupportModule::OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    MainWindow* mainWindow = globalContext->GetData<MainWindow>();
    MainWindow::ProjectView* projectView = mainWindow->GetProjectView();
    MainWindow::DocumentGroupView* documentGroupView = projectView->GetDocumentGroupView();

    Document* document = current->GetData<Document>();
    documentGroupView->OnDocumentChanged(document);
}

DECL_GUI_MODULE(LegacySupportModule);
