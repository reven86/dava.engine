#include "Modules/DocumentsModule/DocumentsModule.h"
#include "Modules/ProjectModule/Project.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/WidgetsData.h"
#include "Modules/DocumentsWatcherModule/DocumentsWatcherData.h"

//legacy
#include "Modules/LegacySupportModule/LegacySupportData.h"
#include "UI/Package/PackageModel.h"

//helpers
#include "UI/QtModelPackageCommandExecutor.h"
#include "Modules/DocumentsModule/Document.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"

#include "EditorSystems/EditorSystemsManager.h"

#include "Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/DocumentGroupView.h"
#include "UI/ProjectView.h"
#include "UI/Preview/PreviewWidget.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/QuickEdPackageBuilder.h"
#include "Model/YamlPackageSerializer.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Models/SceneTabsModel.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>

#include <QTTools/Utils/Themes/Themes.h>
#include <QtTools/InputDialogs/MultilineTextInputDialog.h>

#include <Command/CommandStack.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIStaticText.h>
#include <Render/Renderer.h>
#include <Render/DynamicBufferAllocator.h>
#include <Particles/ParticleEmitter.h>

#include <QAction>

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

    //we can not invoke operations inside RenderInitialized function
    //because RenderInitialized invokes inside DAVA frame and main eventLoop can not be continued to prevent another OnFrame call
    delayedExecutor.DelayedExecute([this]() {
        InvokeOperation(QEGlobal::OpenLastProject.ID);
    });
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

    Themes::InitFromQApplication();
    InitEditorSystems();
    InitCentralWidget();

    RegisterOperations();
    CreateActions();

    fieldBinder.reset(new FieldBinder(GetAccessor()));
    FieldDescriptor descriptor;
    descriptor.type = ReflectedTypeDB::Get<DocumentData>();
    descriptor.fieldName = FastName(DocumentData::canSavePropertyName);
    fieldBinder->BindField(descriptor, MakeFunction(this, &DocumentsModule::OnCanSaveChanged));
}

void DocumentsModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
}

void DocumentsModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    TabDescriptor descriptor;
    descriptor.tabTitle = data->GetName().toStdString();
    descriptor.tabTooltip = data->GetPackageAbsolutePath().toStdString();
    tabsModel->tabs.emplace(context->GetID(), TabDescriptor());
}

void DocumentsModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);

    tabsModel->tabs.erase(context->GetID());
}

void DocumentsModule::OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne)
{
    systemsManager->OnContextWillBeChanged(current, newOne);
    previewWidget->OnContextWillBeChanged(current, newOne);
}

void DocumentsModule::OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
    systemsManager->OnContextWasChanged(current, oldOne);
    previewWidget->OnContextWasChanged(current, oldOne);

    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    tabsModel->activeContexID = current->GetID();
}

void DocumentsModule::InitEditorSystems()
{
    DVASSERT(nullptr == systemsManager);
    systemsManager.reset(new EditorSystemsManager(GetAccessor()));
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

    //legacy part
    LegacySupportData* legacyData = accessor->GetGlobalContext()->GetData<LegacySupportData>();
    MainWindow* mainWindow = legacyData->GetMainWindow();

    QObject::connect(mainWindow, &MainWindow::EmulationModeChanged, previewWidget, &PreviewWidget::OnEmulationModeChanged);
    QObject::connect(previewWidget, &PreviewWidget::DropRequested, mainWindow->packageWidget->GetPackageModel(), &PackageModel::OnDropMimeData, Qt::DirectConnection);
    QObject::connect(previewWidget, &PreviewWidget::DeleteRequested, mainWindow->packageWidget, &PackageWidget::OnDelete);
    QObject::connect(previewWidget, &PreviewWidget::ImportRequested, mainWindow->packageWidget, &PackageWidget::OnImport);
    QObject::connect(previewWidget, &PreviewWidget::CutRequested, mainWindow->packageWidget, &PackageWidget::OnCut);
    QObject::connect(previewWidget, &PreviewWidget::CopyRequested, mainWindow->packageWidget, &PackageWidget::OnCopy);
    QObject::connect(previewWidget, &PreviewWidget::PasteRequested, mainWindow->packageWidget, &PackageWidget::OnPaste);
    QObject::connect(previewWidget, &PreviewWidget::SelectionChanged, mainWindow->packageWidget, &PackageWidget::OnSelectionChanged);

    QObject::connect(mainWindow->packageWidget, &PackageWidget::SelectedNodesChanged, previewWidget, &PreviewWidget::OnSelectionChanged);
}

void DocumentsModule::CreateActions()
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
            return fieldValue.CanCast<bool>() && fieldValue.Cast<bool>();
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
        action->setShortcut(QKeySequence("Ctrl+W"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<RefPtr<PackageNode>>() && fieldValue.Cast<RefPtr<PackageNode>>().Get() != nullptr;
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
                             << QKeySequence(Qt::Key_F5));

        action->setShortcutContext(Qt::ApplicationShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<RefPtr<PackageNode>>() && fieldValue.Cast<RefPtr<PackageNode>>().Get() != nullptr;
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

void DocumentsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::OpenDocumentByPath.ID, this, &DocumentsModule::OpenDocument);
    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &DocumentsModule::CloseAllDocuments);
    RegisterOperation(QEGlobal::ReloadDocuments.ID, this, &DocumentsModule::ReloadDocuments);
    RegisterOperation(QEGlobal::CloseDocuments.ID, this, &DocumentsModule::CloseDocuments);
}

void DocumentsModule::OpenDocument(const QString& path)
{
    using namespace DAVA;
    using namespace TArc;
    std::unique_ptr<DocumentData> documentData = CreateDocument(path);
    if (documentData != nullptr)
    {
        DAVA::Vector<std::unique_ptr<DAVA::TArc::DataNode>> initialData;
        initialData.push_back(std::move(documentData));
        initialData.emplace_back(new WidgetsData());
        ContextManager* contextManager = GetContextManager();
        DataContext::ContextID id = contextManager->CreateContext(std::move(initialData));
        contextManager->ActivateContext(id);
    }
}

std::unique_ptr<DocumentData> DocumentsModule::CreateDocument(const QString& path)
{
    using namespace DAVA;
    using namespace TArc;
    QString canonicalFilePath = QFileInfo(path).canonicalFilePath();
    FilePath davaPath(canonicalFilePath.toStdString());
    QuickEdPackageBuilder builder;
    ProjectData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);
    UIPackageLoader packageLoader(projectData->GetPrototypes());
    std::unique_ptr<DocumentData> documentData;
    bool packageLoaded = packageLoader.LoadPackage(davaPath, &builder);
    if (packageLoaded)
    {
        RefPtr<PackageNode> packageRef = builder.BuildPackage();
        DVASSERT(packageRef.Get() != nullptr);
        documentData.reset(new DocumentData(packageRef));
    }
    return documentData;
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

    String text = textProperty->GetValue().AsString();

    QString label = QObject::tr("Enter new text, please");
    bool ok;
    QString inputText = MultilineTextInputDialog::GetMultiLineText(GetUI()->GetWindow(QEGlobal::windowKey), label, label, QString::fromStdString(text), &ok);
    if (ok)
    {
        ContextAccessor* accessor = GetAccessor();
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(nullptr != activeContext);
        //TODO: remove this code when commandExecutor will be removed
        Document* document = activeContext->GetData<Document>();
        DVASSERT(nullptr != document);
        QtModelPackageCommandExecutor* executor = document->GetCommandExecutor();
        executor->BeginMacro("change text by user");
        AbstractProperty* multilineProperty = rootProperty->FindPropertyByName("Multi Line");
        DVASSERT(multilineProperty != nullptr);
        UIStaticText::eMultiline multilineType = static_cast<UIStaticText::eMultiline>(multilineProperty->GetValue().AsInt32());
        if (inputText.contains('\n') && multilineType == UIStaticText::MULTILINE_DISABLED)
        {
            executor->ChangeProperty(node, multilineProperty, VariantType(UIStaticText::MULTILINE_ENABLED));
        }
        executor->ChangeProperty(node, textProperty, VariantType(inputText.toStdString()));
        executor->EndMacro();
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

void DocumentsModule::CloseDocuments(const QEGlobal::IDList& ids)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();
    for (const DataContext::ContextID& id : ids)
    {
        contextManager->ActivateContext(id);
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        contextManager->ActivateContext(id);
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
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(contextID);
    DVASSERT(context != nullptr);
    ContextManager* contextManager = GetContextManager();
    bool reactivate = accessor->GetActiveContext()->GetID() == contextID;
    if (reactivate)
    {
        contextManager->ActivateContext(DataContext::Empty);
    }
    DocumentData* currentData = context->GetData<DocumentData>();
    QString path = currentData->GetPackageAbsolutePath();
    context->DeleteData<DocumentData>();

    std::unique_ptr<DocumentData> newData = CreateDocument(path);
    context->CreateData(std::move(newData));
    if (reactivate)
    {
        contextManager->ActivateContext(contextID);
    }
}

void DocumentsModule::ReloadDocuments(const QEGlobal::IDList& ids)
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
        manager->ActivateContext(id);
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
                ModalMessageParams params;
                params.title = QObject::tr("File %1 changed").arg(data->GetName());
                params.message = QObject::tr("%1\n\nThis file has been modified outside of the editor. Do you want to reload it?").arg(data->GetPackageAbsolutePath());
                params.icon = ModalMessageParams::Warning;
                params.buttons = changedCount > 1 ?
                (ModalMessageParams::Yes | ModalMessageParams::YesToAll | ModalMessageParams::No | ModalMessageParams::NoToAll) :
                (ModalMessageParams::Yes | ModalMessageParams::No, ModalMessageParams::Yes);

                ModalMessageParams::Button button = GetUI()->ShowModalMessage(QEGlobal::windowKey, params);
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
    DocumentsWatcherData* watcherData = context->GetData<DocumentsWatcherData>();
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

DECL_GUI_MODULE(DocumentsModule);
