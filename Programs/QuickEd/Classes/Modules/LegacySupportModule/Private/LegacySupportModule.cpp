#include "Modules/LegacySupportModule/LegacySupportModule.h"
#include "Modules/LegacySupportModule/Private/Document.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/LegacySupportModule/Private/Project.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include "Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/ProjectView.h"
#include "UI/Package/PackageWidget.h"
#include "UI/DocumentGroupView.h"
#include "UI/Find/FindFilter.h"

#include <TArc/Core/ContextAccessor.h>

#include <QtTools/Utils/Themes/Themes.h>

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
    using namespace DAVA;
    using namespace TArc;

    Themes::InitFromQApplication();

    ContextAccessor* accessor = GetAccessor();

    projectDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);

    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    documentDataWrapper.SetListener(this);

    InitMainWindow();
    RegisterOperations();
}

void LegacySupportModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalData = accessor->GetGlobalContext();
    projectDataWrapper.SetListener(nullptr);

    //this code is writed to support legacy work with Project
    //when we removing ProjectData inside OnWindowClose we dont receive OnDataChanged
    project = nullptr;
}

void LegacySupportModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    QWidget* window = GetUI()->GetWindow(QEGlobal::windowKey);
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window);
    DVASSERT(mainWindow != nullptr);
    MainWindow::ProjectView* projectView = mainWindow->GetProjectView();

    if (wrapper == projectDataWrapper)
    {
        project = nullptr;
        if (std::find(fields.begin(), fields.end(), String(ProjectData::projectPathPropertyName)) != fields.end()
            || wrapper.HasData())
        {
            project.reset(new Project(projectView, GetAccessor()));
        }
    }
    else if (wrapper == documentDataWrapper)
    {
        MainWindow::DocumentGroupView* documentGroupView = projectView->GetDocumentGroupView();
        DataContext* activeContext = accessor->GetActiveContext();
        Document* document = nullptr;
        DataContext::ContextID contextID = activeContext->GetID();
        if (activeContext != nullptr)
        {
            auto iter = documents.find(contextID);
            DVASSERT(iter != documents.end());
            document = iter->second.get();
        }
        PackageWidget* packageWidget = mainWindow->GetPackageWidget();
        if (wrapper.HasData() == false)
        {
            packageWidget->OnSelectionChanged(Any());
            DVASSERT(document == nullptr);
            documentGroupView->SetDocument(document);
        }

        else if (wrapper.HasData() && fields.empty())
        {
            DVASSERT(document != nullptr);
            documentGroupView->SetDocument(document);
            Any selectionValue = wrapper.GetFieldValue(DocumentData::selectionPropertyName);
            packageWidget->OnSelectionChanged(selectionValue);
        }
        else
        {
            //event-based code require selectionChange first, packageChange second and than another selecitonChanged
            DVASSERT(document != nullptr);
            bool selectionWasChanged = std::find(fields.begin(), fields.end(), String(DocumentData::selectionPropertyName)) != fields.end();
            bool packageWasChanged = std::find(fields.begin(), fields.end(), String(DocumentData::packagePropertyName)) != fields.end();
            if (selectionWasChanged || packageWasChanged)
            {
                bool documentWasChanged = false;
                if (packageWasChanged)
                {
                    const DocumentData* documentData = activeContext->GetData<DocumentData>();
                    const PackageNode* package = documentData->GetPackageNode();
                    if (package != document->GetPackage())
                    {
                        document = new Document(accessor, contextID);
                        documentWasChanged = true;
                    }

                    packageWidget->OnSelectionChanged(Any());
                    documentGroupView->SetDocument(document);
                    if (documentWasChanged)
                    {
                        documents[contextID].reset(document);
                    }
                }

                Any selectionValue = wrapper.GetFieldValue(DocumentData::selectionPropertyName);
                packageWidget->OnSelectionChanged(selectionValue);
            }
        }
    }
}

void LegacySupportModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext::ContextID contextID = context->GetID();
    documents[contextID] = std::make_unique<Document>(accessor, contextID);
}

void LegacySupportModule::InitMainWindow()
{
    using namespace DAVA;
    using namespace TArc;

    MainWindow* mainWindow = new MainWindow();
    MainWindow::ProjectView* projectView = mainWindow->GetProjectView();

    connections.AddConnection(projectView, &MainWindow::ProjectView::JumpToControl, MakeFunction(this, &LegacySupportModule::JumpToControl));
    connections.AddConnection(projectView, &MainWindow::ProjectView::JumpToPackage, MakeFunction(this, &LegacySupportModule::JumpToPackage));
    connections.AddConnection(projectView, &MainWindow::ProjectView::JumpToPrototype, MakeFunction(this, &LegacySupportModule::OnJumpToPrototype));
    connections.AddConnection(projectView, &MainWindow::ProjectView::FindPrototypeInstances, MakeFunction(this, &LegacySupportModule::OnFindPrototypeInstances));

    MainWindow::DocumentGroupView* documentGroupView = projectView->GetDocumentGroupView();
    connections.AddConnection(documentGroupView, &MainWindow::DocumentGroupView::OpenPackageFile, [this](const QString& path) {
        InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
    });

    connections.AddConnection(mainWindow->GetPackageWidget(), &PackageWidget::SelectedNodesChanged, MakeFunction(this, &LegacySupportModule::OnSelectionInPackageChanged));

    const char* editorTitle = "DAVA Framework - QuickEd | %1-%2 [%3 bit]";
    uint32 bit = static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8);
    QString title = QString(editorTitle).arg(DAVAENGINE_VERSION).arg(APPLICATION_BUILD_VERSION).arg(bit);
    mainWindow->SetEditorTitle(title);

    GetUI()->InjectWindow(QEGlobal::windowKey, mainWindow);
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
}

void LegacySupportModule::RegisterOperations()
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    QWidget* window = GetUI()->GetWindow(QEGlobal::windowKey);
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window);
    DVASSERT(nullptr != mainWindow);
    MainWindow::ProjectView* view = mainWindow->GetProjectView();
    RegisterOperation(QEGlobal::SelectFile.ID, view, &MainWindow::ProjectView::SelectFile);
}

void LegacySupportModule::OnFindPrototypeInstances()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    //TODO: bind this action to current document when Package will be moved to TArc
    if (activeContext == nullptr)
    {
        return;
    }

    const DocumentData* documentData = activeContext->GetData<DocumentData>();
    const SelectedNodes& nodes = documentData->GetSelectedNodes();

    if (nodes.size() == 1)
    {
        auto it = nodes.begin();
        PackageBaseNode* node = *it;

        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            FilePath path = controlNode->GetPackage()->GetPath();
            String name = controlNode->GetName();

            DataContext* globalContext = accessor->GetGlobalContext();
            QWidget* window = GetUI()->GetWindow(QEGlobal::windowKey);
            MainWindow* mainWindow = qobject_cast<MainWindow*>(window);
            MainWindow::ProjectView* view = mainWindow->GetProjectView();

            view->FindControls(std::make_unique<PrototypeUsagesFilter>(path.GetFrameworkPath(), FastName(name)));
        }
    }
}

void LegacySupportModule::OnSelectionInPackageChanged(const SelectedNodes& selection)
{
    using namespace DAVA;
    using namespace TArc;
    documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
}

void LegacySupportModule::OnJumpToPrototype()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    //TODO: bind this action to current document when Package will be moved to TArc
    if (activeContext == nullptr)
    {
        return;
    }

    const DocumentData* documentData = activeContext->GetData<DocumentData>();
    const SelectedNodes& nodes = documentData->GetSelectedNodes();
    if (nodes.size() == 1)
    {
        auto it = nodes.begin();
        PackageBaseNode* node = *it;

        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr && controlNode->GetPrototype() != nullptr)
        {
            ControlNode* prototypeNode = controlNode->GetPrototype();
            FilePath path = prototypeNode->GetPackage()->GetPath();
            String name = prototypeNode->GetName();
            JumpToControl(path, name);
        }
    }
}

void LegacySupportModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    using namespace DAVA;
    using namespace TArc;

    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    QString name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void LegacySupportModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}
