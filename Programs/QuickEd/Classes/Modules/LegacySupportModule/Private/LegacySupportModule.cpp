#include "Modules/LegacySupportModule/LegacySupportModule.h"
#include "Modules/DocumentsModule/Document.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/ProjectModule/Project.h"

#include "Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/ProjectView.h"
#include "UI/DocumentGroupView.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/Private/UIManager.h>

#include <Tools/version.h>
#include <DAVAVersion.h>

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

    InitMainWindow();
}

void LegacySupportModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalData = accessor->GetGlobalContext();
    globalData->DeleteData<MainWindow>();

    projectDataWrapper.SetListener(nullptr);
}

void LegacySupportModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA::TArc;
    if (fields.empty() == false)
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

void LegacySupportModule::InitMainWindow()
{
    using namespace DAVA;
    using namespace TArc;

    std::unique_ptr<MainWindow> mainWindow(new MainWindow());
    MainWindow* mainWindowPtr = mainWindow.get();
    const char* editorTitle = "DAVA Framework - QuickEd | %1-%2 [%3 bit]";
    uint32 bit = static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8);
    QString title = QString(editorTitle).arg(DAVAENGINE_VERSION).arg(APPLICATION_BUILD_VERSION).arg(bit);
    mainWindowPtr->SetEditorTitle(title);

    UIManager* ui = static_cast<UIManager*>(GetUI());
    ui->InjectWindow(QEGlobal::windowKey, mainWindowPtr);
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::move(mainWindow));
}
