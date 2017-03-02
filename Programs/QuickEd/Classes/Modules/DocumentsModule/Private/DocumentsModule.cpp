#include "Modules/DocumentsModule/DocumentsModule.h"
#include "Modules/LegacySupportModule/Private/Project.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/Private/DocumentsWatcherData.h"
#include "Modules/DocumentsModule/Private/EditorCanvasData.h"
#include "Modules/LegacySupportModule/Private/Document.h"

#include "QECommands/ChangePropertyValueCommand.h"

#include "UI/Package/PackageModel.h"

#include "UI/QtModelPackageCommandExecutor.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include "EditorSystems/EditorSystemsManager.h"

#include "Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/DocumentGroupView.h"
#include "UI/ProjectView.h"
#include "UI/Preview/PreviewWidget.h"
#include "UI/Package/PackageWidget.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/QuickEdPackageBuilder.h"
#include "Model/YamlPackageSerializer.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Models/SceneTabsModel.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>

#include <QtTools/InputDialogs/MultilineTextInputDialog.h>

#include <Command/CommandStack.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIStaticText.h>
#include <Render/Renderer.h>
#include <Render/DynamicBufferAllocator.h>
#include <Particles/ParticleEmitter.h>
#include <Engine/PlatformApi.h>

#include <QAction>
#include <QApplication>
#include <QFileSystemWatcher>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentsModule)
{
    DAVA::ReflectionRegistrator<DocumentsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DocumentsModule::DocumentsModule() = default;
DocumentsModule::~DocumentsModule() = default;

void DocumentsModule::OnRenderSystemInitialized(DAVA::Window* window)
{
    using namespace DAVA;

    Renderer::SetDesiredFPS(60);
    DynamicBufferAllocator::SetPageSize(DynamicBufferAllocator::DEFAULT_PAGE_SIZE);
}

bool DocumentsModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText)
{
    using namespace DAVA;
    using namespace TArc;
    DVASSERT(QEGlobal::windowKey == key);
    QString windowText = QObject::tr("Save changes to the following items?\n");
    QStringList unsavedDocuments;
    ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([&unsavedDocuments](DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->CanSave())
        {
            unsavedDocuments << data->GetName();
        }
    });

    const int maxDisplayableDocumentsCount = 15;
    if (unsavedDocuments.size() > maxDisplayableDocumentsCount)
    {
        unsavedDocuments = unsavedDocuments.mid(0, maxDisplayableDocumentsCount);
        unsavedDocuments << "...";
    }
    windowText.append(unsavedDocuments.join("\n"));
    requestWindowText = windowText.toStdString();

    return HasUnsavedDocuments() == false;
}

void DocumentsModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([this](DataContext& context) {
        SaveDocument(context.GetID());
    });
}

void DocumentsModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
    //do nothing
}

void DocumentsModule::PostInit()
{
    using namespace DAVA;
    using namespace TArc;

    fieldBinder.reset(new FieldBinder(GetAccessor()));

    InitWatcher();
    InitEditorSystems();
    InitCentralWidget();

    RegisterOperations();
    CreateDocumentsActions();
    CreateUndoRedoActions();
    CreateViewActions();

    //bind canSave to draw "*" in tabBar
    {
        FieldDescriptor descriptor;
        descriptor.type = ReflectedTypeDB::Get<DocumentData>();
        descriptor.fieldName = FastName(DocumentData::canSavePropertyName);
        fieldBinder->BindField(descriptor, MakeFunction(this, &DocumentsModule::OnCanSaveChanged));
    }
    //bind active tab to change context
    {
        FieldDescriptor descriptor;
        descriptor.type = ReflectedTypeDB::Get<SceneTabsModel>();
        descriptor.fieldName = FastName(SceneTabbar::activeTabPropertyName);
        fieldBinder->BindField(descriptor, MakeFunction(this, &DocumentsModule::OnActiveTabChanged));
    }
}

void DocumentsModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA;
    using namespace TArc;

    DeleteAllDocuments();

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();
    context->DeleteData<DocumentsWatcherData>();
}

void DocumentsModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(nullptr != data);

    TabDescriptor descriptor;
    descriptor.tabTitle = data->GetName().toStdString();
    descriptor.tabTooltip = data->GetPackageAbsolutePath().toStdString();

    auto emplaceResult = tabsModel->tabs.emplace(context->GetID(), descriptor);
    DVASSERT(emplaceResult.second, "emplace new tab failed");
    QString path = data->GetPackageAbsolutePath();
    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    watcherData->Watch(path);
}

void DocumentsModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);
    tabsModel->tabs.erase(context->GetID());

    DocumentData* data = context->GetData<DocumentData>();
    QString path = data->GetPackageAbsolutePath();
    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    watcherData->Unwatch(path);
}

void DocumentsModule::OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne)
{
    if (current != nullptr)
    {
        DocumentData* documentData = current->GetData<DocumentData>();
        DVASSERT(nullptr != documentData);
        //check that we do not leave document in non valid state
        DVASSERT(documentData->package->CanUpdateAll());
    }
}

void DocumentsModule::OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    tabsModel->activeContexID = current->GetID();
}

void DocumentsModule::InitEditorSystems()
{
    DVASSERT(nullptr == systemsManager);
    systemsManager.reset(new EditorSystemsManager(GetAccessor()));
    systemsManager->dragStateChanged.Connect(this, &DocumentsModule::OnDragStateChanged);
    systemsManager->propertyChanged.Connect(this, &DocumentsModule::OnPropertyChanged);
}

void DocumentsModule::InitCentralWidget()
{
    using namespace DAVA;
    using namespace TArc;

    UI* ui = GetUI();
    ContextAccessor* accessor = GetAccessor();

    RenderWidget* renderWidget = GetContextManager()->GetRenderWidget();

    previewWidget = new PreviewWidget(accessor, renderWidget, systemsManager.get());
    previewWidget->requestCloseTab.Connect([this](uint64 id) { CloseDocument(id); });
    previewWidget->requestChangeTextInNode.Connect(this, &DocumentsModule::ChangeControlText);
    PanelKey panelKey(QStringLiteral("CentralWidget"), CentralPanelInfo());
    ui->AddView(QEGlobal::windowKey, panelKey, previewWidget);

    //legacy part. Remove it when package will be refactored
    MainWindow* mainWindow = qobject_cast<MainWindow*>(GetUI()->GetWindow(QEGlobal::windowKey));

    QObject::connect(mainWindow, &MainWindow::EmulationModeChanged, previewWidget, &PreviewWidget::OnEmulationModeChanged);
    QObject::connect(previewWidget, &PreviewWidget::DropRequested, mainWindow->GetPackageWidget()->GetPackageModel(), &PackageModel::OnDropMimeData, Qt::DirectConnection);
    QObject::connect(previewWidget, &PreviewWidget::DeleteRequested, mainWindow->GetPackageWidget(), &PackageWidget::OnDelete);
    QObject::connect(previewWidget, &PreviewWidget::ImportRequested, mainWindow->GetPackageWidget(), &PackageWidget::OnImport);
    QObject::connect(previewWidget, &PreviewWidget::CutRequested, mainWindow->GetPackageWidget(), &PackageWidget::OnCut);
    QObject::connect(previewWidget, &PreviewWidget::CopyRequested, mainWindow->GetPackageWidget(), &PackageWidget::OnCopy);
    QObject::connect(previewWidget, &PreviewWidget::PasteRequested, mainWindow->GetPackageWidget(), &PackageWidget::OnPaste);
}

void DocumentsModule::InitWatcher()
{
    using namespace DAVA;
    using namespace TArc;

    std::unique_ptr<DocumentsWatcherData> data(new DocumentsWatcherData());
    connections.AddConnection(data->watcher.get(), &QFileSystemWatcher::fileChanged, MakeFunction(this, &DocumentsModule::OnFileChanged));
    QApplication* app = PlatformApi::Qt::GetApplication();
    connections.AddConnection(app, &QApplication::applicationStateChanged, MakeFunction(this, &DocumentsModule::OnApplicationStateChanged));

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::move(data));
}

void DocumentsModule::CreateDocumentsActions()
{
    using namespace DAVA;
    using namespace TArc;

    const QString toolBarName("mainToolbar");
    const QString fileMenuName("File");

    const QString saveDocumentActionName("Save document");
    const QString saveAllDocumentsActionName("Force save all");
    const QString reloadDocumentActionName("Reload document");
    const QString closeDocumentActionName("Close document");
    const QString toolBarSeparatorName("documents separator");

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();
    //action save document
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/savescene.png"), saveDocumentActionName);
        action->setShortcut(QKeySequence("Ctrl+S"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::SaveCurrentDocument, this));

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::canSavePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, "Close project" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, "project actions separator" }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //action save all documents
    {
        QAction* action = new QAction(QIcon(":/Icons/savesceneall.png"), saveAllDocumentsActionName, nullptr);
        action->setShortcut(QKeySequence("Ctrl+Shift+S"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::SaveAllDocuments, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, saveDocumentActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, saveDocumentActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //action close document
    {
        QtAction* action = new QtAction(accessor, closeDocumentActionName);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::CloseActiveDocument, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, saveAllDocumentsActionName }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    // action reload document
    {
        QtAction* action = new QtAction(accessor, reloadDocumentActionName);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+R")
                             << QKeySequence("F5"));

        action->setShortcutContext(Qt::ApplicationShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::ReloadCurrentDocument, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, closeDocumentActionName }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(toolBarSeparatorName, nullptr);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, saveAllDocumentsActionName }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, separator);
    }
}

void DocumentsModule::CreateUndoRedoActions()
{
    using namespace DAVA;
    using namespace TArc;

    const QString undoActionName("Undo");
    const QString redoActionName("Redo");

    const QString toolBarName("mainToolbar");
    const QString editMenuName("Edit");
    const QString editMenuSeparatorName("undo redo separator");

    Function<Any(QString, const Any&)> makeActionName = [](QString baseName, const Any& actionText) {
        if (actionText.CanCast<QString>())
        {
            QString text = actionText.Cast<QString>();
            if (text.isEmpty() == false)
            {
                baseName += ": " + text;
            }
        }
        return Any(baseName);
    };

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    //Undo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_undo.png"), undoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcut(QKeySequence("Ctrl+Z"));

        FieldDescriptor fieldDescrCanUndo;
        fieldDescrCanUndo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanUndo.fieldName = FastName(DocumentData::canUndoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanUndo, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::undoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, Bind(makeActionName, "Undo", _1));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &DocumentsModule::OnUndo));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(editMenuName, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, "documents separator" }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //Redo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_redo.png"), redoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+Y")
                             << QKeySequence("Ctrl+Shift+Z"));

        FieldDescriptor fieldDescrCanRedo;
        fieldDescrCanRedo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanRedo.fieldName = FastName(DocumentData::canRedoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanRedo, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::redoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, Bind(makeActionName, "Redo", _1));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &DocumentsModule::OnRedo));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(editMenuName, { InsertionParams::eInsertionMethod::AfterItem, undoActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, undoActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(editMenuSeparatorName, nullptr);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateToolbarPoint(editMenuName, { InsertionParams::eInsertionMethod::AfterItem, redoActionName }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, separator);
    }
}

void DocumentsModule::OnUndo()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    data->commandStack->Undo();
}

void DocumentsModule::OnRedo()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    data->commandStack->Redo();
}

void DocumentsModule::CreateViewActions()
{
    using namespace DAVA;
    using namespace TArc;

    const QString zoomInActionName("Zoom In");
    const QString zoomOutActionName("Zoom Out");
    const QString actualZoomActionName("Actual zoom");
    const QString zoomSeparator("zoom separator");

    const QString viewMenuName("View");

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setObjectName(zoomSeparator);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(viewMenuName, { InsertionParams::eInsertionMethod::AfterItem, "menuGridColor" }));
        ui->AddAction(QEGlobal::windowKey, placementInfo, separator);
    }

    //Zoom in
    {
        QAction* action = new QAction(zoomInActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+=")
                             << QKeySequence("Ctrl++"));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget, &PreviewWidget::OnIncrementScale));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(viewMenuName, { InsertionParams::eInsertionMethod::AfterItem, zoomSeparator }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //Zoom out
    {
        QAction* action = new QAction(zoomOutActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+-"));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget, &PreviewWidget::OnDecrementScale));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(viewMenuName, { InsertionParams::eInsertionMethod::AfterItem, zoomInActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //Actual zoom
    {
        QAction* action = new QAction(actualZoomActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+0"));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget, &PreviewWidget::SetActualScale));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(viewMenuName, { InsertionParams::eInsertionMethod::AfterItem, zoomOutActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }
}

void DocumentsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::OpenDocumentByPath.ID, this, &DocumentsModule::OpenDocument);
    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &DocumentsModule::CloseAllDocuments);
    RegisterOperation(QEGlobal::SelectControl.ID, this, &DocumentsModule::SelectControl);
}

DAVA::TArc::DataContext::ContextID DocumentsModule::OpenDocument(const QString& path)
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext::ContextID id = DataContext::Empty;
    accessor->ForEachContext([&id, path](const DAVA::TArc::DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->GetPackageAbsolutePath() == path)
        {
            DVASSERT(id == DataContext::Empty);
            id = context.GetID();
        }
    });

    ContextManager* contextManager = GetContextManager();

    if (id == DataContext::Empty)
    {
        RefPtr<PackageNode> package = CreatePackage(path);
        if (package != nullptr)
        {
            DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>> initialData;
            initialData.emplace_back(new DocumentData(package));
            initialData.emplace_back(new EditorCanvasData());
            id = contextManager->CreateContext(std::move(initialData));
        }
    }
    if (id != DataContext::Empty)
    {
        contextManager->ActivateContext(id);
    }
    return id;
}

DAVA::RefPtr<PackageNode> DocumentsModule::CreatePackage(const QString& path)
{
    using namespace DAVA;
    using namespace TArc;

    QString canonicalFilePath = QFileInfo(path).canonicalFilePath();
    FilePath davaPath(canonicalFilePath.toStdString());

    ProjectData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);

    QuickEdPackageBuilder builder;
    UIPackageLoader packageLoader(projectData->GetPrototypes());
    bool packageLoaded = packageLoader.LoadPackage(davaPath, &builder);

    if (packageLoaded)
    {
        RefPtr<PackageNode> packageRef = builder.BuildPackage();
        DVASSERT(packageRef.Get() != nullptr);
        return packageRef;
    }
    else
    {
        //we need to continue current operation and not stop it with a modal message
        delayedExecutor.DelayedExecute([this, path]() {
            ModalMessageParams params;
            params.icon = ModalMessageParams::Warning;
            params.title = QObject::tr("Can not create document");
            params.message = QObject::tr("Can not create document by path:\n%1").arg(path);
            params.buttons = ModalMessageParams::Ok;
            GetUI()->ShowModalMessage(QEGlobal::windowKey, params);
        });
    }
    return RefPtr<PackageNode>();
}

void DocumentsModule::OnActiveTabChanged(const DAVA::Any& contextID)
{
    using namespace DAVA::TArc;
    ContextManager* contextManager = GetContextManager();
    DataContext::ContextID newContextID = DataContext::Empty;
    if (contextID.CanCast<DAVA::uint64>())
    {
        newContextID = static_cast<DataContext::ContextID>(contextID.Cast<DAVA::uint64>());
    }
    contextManager->ActivateContext(newContextID);
}

void DocumentsModule::OnCanSaveChanged(const DAVA::Any& canSave)
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    DataContext::ContextID id = activeContext->GetID();
    DocumentData* data = activeContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);

    SceneTabsModel* tabsModel = accessor->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel->tabs.find(id) != tabsModel->tabs.end());
    TabDescriptor& descriptor = tabsModel->tabs[id];
    QString newTitle = data->GetName() + (data->CanSave() ? "*" : "");
    descriptor.tabTitle = newTitle.toStdString();
}

void DocumentsModule::SelectControl(const QString& documentPath, const QString& controlPath)
{
    using namespace DAVA;
    using namespace TArc;
    DataContext::ContextID id = OpenDocument(documentPath);
    if (id == DataContext::Empty)
    {
        return;
    }
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetContext(id);
    DVASSERT(id == activeContext->GetID());
    DocumentData* data = activeContext->GetData<DocumentData>();
    const PackageNode* package = data->GetPackageNode();
    String controlPathStr = controlPath.toStdString();
    ControlNode* node = package->GetPrototypes()->FindControlNodeByPath(controlPathStr);
    if (node == nullptr)
    {
        node = package->GetPackageControlsNode()->FindControlNodeByPath(controlPathStr);
    }
    if (node != nullptr)
    {
        data->SetSelectedNodes({ node });
    }
}

//TODO: generalize changes in central widget
void DocumentsModule::ChangeControlText(ControlNode* node)
{
    using namespace DAVA;
    using namespace TArc;
    DVASSERT(node != nullptr);

    UIControl* control = node->GetControl();

    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    DVASSERT(staticText != nullptr);

    RootProperty* rootProperty = node->GetRootProperty();
    AbstractProperty* textProperty = rootProperty->FindPropertyByName("Text");
    DVASSERT(textProperty != nullptr);

    String text = textProperty->GetValue().Cast<String>();

    QString label = QObject::tr("Enter new text, please");
    bool ok;
    QString inputText = MultilineTextInputDialog::GetMultiLineText(GetUI()->GetWindow(QEGlobal::windowKey), label, label, QString::fromStdString(text), &ok);
    if (ok)
    {
        ContextAccessor* accessor = GetAccessor();
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(nullptr != activeContext);
        DocumentData* data = activeContext->GetData<DocumentData>();
        CommandStack* stack = data->commandStack.get();
        stack->BeginBatch("change text by user");
        AbstractProperty* multilineProperty = rootProperty->FindPropertyByName("Multi Line");
        DVASSERT(multilineProperty != nullptr);
        UIStaticText::eMultiline multilineType = multilineProperty->GetValue().Cast<UIStaticText::eMultiline>();
        if (inputText.contains('\n') && multilineType == UIStaticText::MULTILINE_DISABLED)
        {
            stack->Exec(std::make_unique<ChangePropertyValueCommand>(node, multilineProperty, VariantType(UIStaticText::MULTILINE_ENABLED)));
        }
        stack->Exec(std::make_unique<ChangePropertyValueCommand>(node, textProperty, VariantType(inputText.toStdString())));
        stack->EndBatch();
    }
}

void DocumentsModule::CloseActiveDocument()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* active = accessor->GetActiveContext();
    DVASSERT(active != nullptr);
    CloseDocument(active->GetID());
}

void DocumentsModule::CloseDocument(const DAVA::TArc::DataContext::ContextID& id)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();

    DataContext* context = accessor->GetContext(id);
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    if (data->CanSave())
    {
        QString status = data->documentExists ? "modified" : "renamed or removed";
        ModalMessageParams params;
        params.title = QObject::tr("Save changes");
        params.message = QObject::tr("The file %1 has been %2.\n"
                                     "Do you want to save it?")
                         .arg(data->GetName())
                         .arg(status);
        params.defaultButton = ModalMessageParams::Save;
        params.buttons = ModalMessageParams::Save | ModalMessageParams::Discard | ModalMessageParams::Cancel;
        ModalMessageParams::Button ret = GetUI()->ShowModalMessage(QEGlobal::windowKey, params);

        if (ret == ModalMessageParams::Save)
        {
            SaveDocument(id);
        }
        else if (ret == ModalMessageParams::Cancel)
        {
            return;
        }
    }
    contextManager->DeleteContext(id);
}

void DocumentsModule::CloseAllDocuments()
{
    using namespace DAVA::TArc;
    bool hasUnsaved = HasUnsavedDocuments();

    if (hasUnsaved)
    {
        ModalMessageParams params;
        params.title = QObject::tr("Save changes");
        params.message = QObject::tr("Some files has been modified.\n"
                                     "Do you want to save your changes?");
        params.buttons = ModalMessageParams::SaveAll | ModalMessageParams::NoToAll | ModalMessageParams::Cancel;
        params.icon = ModalMessageParams::Question;
        ModalMessageParams::Button button = GetUI()->ShowModalMessage(QEGlobal::windowKey, params);
        if (button == ModalMessageParams::Cancel)
        {
            return;
        }
        else if (button == ModalMessageParams::SaveAll)
        {
            SaveAllDocuments();
        }
    }
    DeleteAllDocuments();
}

void DocumentsModule::DeleteAllDocuments()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();
    DAVA::Vector<DataContext::ContextID> contexts;
    accessor->ForEachContext([&contexts](DataContext& context)
                             {
                                 contexts.push_back(context.GetID());
                             });
    for (DataContext::ContextID id : contexts)
    {
        contextManager->DeleteContext(id);
    }
}

void DocumentsModule::CloseDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();
    for (const DataContext::ContextID& id : ids)
    {
        contextManager->ActivateContext(id);
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);
        ModalMessageParams::Button button = ModalMessageParams::No;
        ModalMessageParams params;
        params.title = QObject::tr("File %1 is renamed or removed").arg(data->GetName());
        params.icon = ModalMessageParams::Warning;
        params.message = QObject::tr("%1\n\nThis file has been renamed or removed. Do you want to close it?")
                         .arg(data->GetPackageAbsolutePath());
        params.buttons = ModalMessageParams::Yes | ModalMessageParams::No;
        params.defaultButton = ModalMessageParams::No;
        button = GetUI()->ShowModalMessage(QEGlobal::windowKey, params);
        if (button == ModalMessageParams::Yes)
        {
            contextManager->DeleteContext(id);
        }
    }
}

void DocumentsModule::ReloadCurrentDocument()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* active = accessor->GetActiveContext();
    DVASSERT(active != nullptr);
    if (active != nullptr)
    {
        ReloadDocument(active->GetID());
    }
}

void DocumentsModule::ReloadDocument(const DAVA::TArc::DataContext::ContextID& contextID)
{
    using namespace DAVA;
    using namespace TArc;

    ContextManager* contextManager = GetContextManager();
    contextManager->ActivateContext(contextID);

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(contextID);
    DVASSERT(context != nullptr);
    DVASSERT(contextID == accessor->GetActiveContext()->GetID());
    DocumentData* currentData = context->GetData<DocumentData>();
    QString path = currentData->GetPackageAbsolutePath();

    RefPtr<PackageNode> package = CreatePackage(path);
    //if document was created successfully - delete previous data and create new one with new package.
    //this required because current program modules storing selection and another data as pointers to package children
    //else, if document was broken or damaged, can not use this context any more
    if (package != nullptr)
    {
        context->DeleteData<DocumentData>();
        context->CreateData(std::make_unique<DocumentData>(package));
    }
    else
    {
        contextManager->DeleteContext(contextID);
    }
}

void DocumentsModule::ReloadDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();

    bool yesToAll = false;
    bool noToAll = false;

    int changedCount = std::count_if(ids.begin(), ids.end(), [accessor](const DataContext::ContextID& id) {
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);
        return data->CanSave();
    });
    ContextManager* manager = GetContextManager();

    for (const DataContext::ContextID& id : ids)
    {
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);

        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        ModalMessageParams::Button button = ModalMessageParams::No;
        if (!data->CanSave())
        {
            button = ModalMessageParams::Yes;
        }
        else
        {
            if (!yesToAll && !noToAll)
            {
                //activate this document to show it to a user
                manager->ActivateContext(id);

                ModalMessageParams params;
                params.title = QObject::tr("File %1 changed").arg(data->GetName());
                params.message = QObject::tr("%1\n\nThis file has been modified outside of the editor. Do you want to reload it?").arg(data->GetPackageAbsolutePath());
                params.icon = ModalMessageParams::Warning;
                if (changedCount == 1)
                {
                    params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Yes;
                }
                else
                {
                    params.buttons = ModalMessageParams::Yes | ModalMessageParams::YesToAll | ModalMessageParams::No | ModalMessageParams::NoToAll;
                }

                button = GetUI()->ShowModalMessage(QEGlobal::windowKey, params);
                yesToAll = (button == ModalMessageParams::YesToAll);
                noToAll = (button == ModalMessageParams::NoToAll);
            }
            if (yesToAll)
            {
                button = ModalMessageParams::Yes;
            }
        }
        if (button == ModalMessageParams::Yes)
        {
            ReloadDocument(id);
        }
    }
}

bool DocumentsModule::HasUnsavedDocuments() const
{
    bool hasUnsaved = false;
    const DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([&hasUnsaved](const DAVA::TArc::DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        hasUnsaved |= (data->CanSave());
    });
    return hasUnsaved;
}

void DocumentsModule::SaveDocument(const DAVA::TArc::DataContext::ContextID& contextID)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(contextID);
    DVASSERT(nullptr != context);
    DataContext* globalContext = accessor->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    DVASSERT(nullptr != watcherData);
    DocumentData* data = context->GetData<DocumentData>();

    QString path = data->GetPackageAbsolutePath();
    watcherData->Unwatch(path);
    YamlPackageSerializer serializer;
    serializer.SerializePackage(data->package.Get());
    serializer.WriteToFile(data->package->GetPath());
    data->commandStack->SetClean();
    watcherData->Watch(path);
}

void DocumentsModule::SaveAllDocuments()
{
    GetAccessor()->ForEachContext([this](DAVA::TArc::DataContext& context)
                                  {
                                      SaveDocument(context.GetID());
                                  });
}

void DocumentsModule::SaveCurrentDocument()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    SaveDocument(activeContext->GetID());
}

void DocumentsModule::OnFileChanged(const QString& path)
{
    using namespace DAVA::TArc;
    DataContext::ContextID id = GetContextByPath(path);

    ContextAccessor* accessor = GetAccessor();
    DocumentsWatcherData* watcherData = accessor->GetGlobalContext()->GetData<DocumentsWatcherData>();
    watcherData->changedDocuments.insert(id);
    DocumentData* data = GetAccessor()->GetContext(id)->GetData<DocumentData>();
    QFileInfo fileInfo(path);
    data->documentExists = fileInfo.exists();
    if (!data->CanSave() || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsModule::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsModule::ApplyFileChanges()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DocumentsWatcherData* watcherData = accessor->GetGlobalContext()->GetData<DocumentsWatcherData>();

    DAVA::Set<DAVA::TArc::DataContext::ContextID> changed;
    DAVA::Set<DAVA::TArc::DataContext::ContextID> removed;
    for (DataContext::ContextID id : watcherData->changedDocuments)
    {
        DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        DocumentData* data = context->GetData<DocumentData>();

        if (data->documentExists)
        {
            changed.insert(id);
        }
        else
        {
            removed.insert(id);
        }
    }

    watcherData->changedDocuments.clear();

    if (!changed.empty())
    {
        ReloadDocuments(changed);
    }
    if (!removed.empty())
    {
        CloseDocuments(removed);
    }
}

DAVA::TArc::DataContext::ContextID DocumentsModule::GetContextByPath(const QString& path) const
{
    using namespace DAVA::TArc;
    DataContext::ContextID ret = DataContext::Empty;
    GetAccessor()->ForEachContext([path, &ret](const DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->GetPackageAbsolutePath() == path)
        {
            DVASSERT(ret == DataContext::Empty);
            ret = context.GetID();
        }
    });
    DVASSERT(ret != DataContext::Empty);
    return ret;
}

void DocumentsModule::OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(nullptr != documentData);
    //TODO: move this code to the TransformSystem when systems will be moved to the TArc
    if (dragState == EditorSystemsManager::Transform)
    {
        documentData->canClose = false;
        documentData->commandStack->BeginBatch("transformations");
    }
    else if (previousState == EditorSystemsManager::Transform)
    {
        documentData->canClose = true;
        documentData->commandStack->EndBatch();
    }
}

void DocumentsModule::OnPropertyChanged(ControlNode* node, AbstractProperty* property, const DAVA::Any& newValue)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* data = activeContext->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    data->commandStack->Exec(std::make_unique<ChangePropertyValueCommand>(node, property, newValue));
}

DECL_GUI_MODULE(DocumentsModule);
