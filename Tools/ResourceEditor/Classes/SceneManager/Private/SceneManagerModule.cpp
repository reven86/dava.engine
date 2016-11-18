#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/mainwindow.h"

#include "Classes/SceneManager/Private/SceneRenderWidget.h"
#include "Classes/SceneManager/Private/SceneTabsModel.h"

#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"

#include "QtTools/FileDialogs/FindFileDialog.h"

#include "Engine/EngineContext.h"
#include "Reflection/ReflectedType.h"
#include "Render/Renderer.h"
#include "Render/DynamicBufferAllocator.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Base/FastName.h"
#include "Base/Any.h"

void SceneManagerModule::OnRenderSystemInitialized(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);
    DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    DAVA::uint32 val = SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32();
    DAVA::eGPUFamily family = static_cast<DAVA::eGPUFamily>(val);
    DAVA::Texture::SetGPULoadingOrder({ family });

    QtMainWindow* wnd = qobject_cast<QtMainWindow*>(GetUI()->GetWindow(REGlobal::MainWindowKey));
    DVASSERT(wnd != nullptr);
    wnd->OnRenderingInitialized();
}

bool SceneManagerModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key)
{
    return false;
}

bool SceneManagerModule::ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event)
{
    return true;
}

void SceneManagerModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void SceneManagerModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void SceneManagerModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);

    tabsModel->tabs.emplace(context->GetID(), TabDescriptor());
}

void SceneManagerModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;
    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel != nullptr);

    tabsModel->tabs.erase(context->GetID());
}

void SceneManagerModule::OnContextWillChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne)
{
    using namespace DAVA::TArc;
    if (current == nullptr)
    {
        return;
    }

    SceneData* data = current->GetData<SceneData>();
    data->scene->Deactivate();
}

void SceneManagerModule::OnContextDidChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
    using namespace DAVA::TArc;
    if (current == nullptr)
    {
        return;
    }

    SceneData* data = current->GetData<SceneData>();
    DVASSERT(data->scene != nullptr);
    data->scene->Activate();

    SceneTabsModel* tabsModel = GetAccessor()->GetGlobalContext()->GetData<SceneTabsModel>();
    tabsModel->activeContexID = current->GetID();
    tabsModel->tabs[current->GetID()].tabTitle = data->scene->GetScenePath().GetFilename();
}

void SceneManagerModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
}

void SceneManagerModule::PostInit()
{
    DAVA::TArc::UI* ui = GetUI();

    CreateModuleControls(ui);
    CreateModuleActions(ui);
    RegisterOperations();

    fieldBinder.reset(new DAVA::TArc::FieldBinder(GetAccessor()));

    {
        DAVA::TArc::FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedType::Get<SceneTabsModel>();
        fieldDescr.fieldName = DAVA::FastName(DAVA::TArc::SceneTabbar::activeTabPropertyName);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &SceneManagerModule::OnActiveTabChanged));
    }
    {
        DAVA::TArc::FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedType::Get<SceneData>();
        fieldDescr.fieldName = DAVA::FastName(SceneData::sceneChangedPropertyName);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &SceneManagerModule::OnScenePathChanged));

        fieldDescr.fieldName = DAVA::FastName(SceneData::scenePathPropertyName);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &SceneManagerModule::OnScenePathChanged));
    }

    QtMainWindow* mainWindow = qobject_cast<QtMainWindow*>(ui->GetWindow(REGlobal::MainWindowKey));
    mainWindow->EnableGlobalTimeout(true);
}

void SceneManagerModule::CreateModuleControls(DAVA::TArc::UI* ui)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();

    SceneRenderWidget* renderWidget = new SceneRenderWidget(accessor, GetContextManager()->GetRenderWidget());
    renderWidget->SetWidgetDelegate(this);

    DAVA::TArc::PanelKey panelKey(QStringLiteral("SceneTabBar"), DAVA::TArc::CentralPanelInfo());
    GetUI()->AddView(REGlobal::MainWindowKey, panelKey, renderWidget);
}

void SceneManagerModule::CreateModuleActions(DAVA::TArc::UI* ui)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    // New Scene action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/newscene.png"), QString("New Scene"));
        action->setShortcut(QKeySequence("Ctrl+N"));
        action->setShortcutContext(Qt::WindowShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldDescr.type = DAVA::ReflectedType::Get<ProjectManagerData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.CanCast<DAVA::FilePath>() && !value.Cast<DAVA::FilePath>().IsEmpty();
        });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::CreateNewScene, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::BeforeItem, "actionExport" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::BeforeItem }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);

        RegisterOperation(REGlobal::CreateNewSceneOperation.ID, this, &SceneManagerModule::CreateNewScene);
    }

    // Open Scene action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/openscene.png"), QString("Open Scene"));
        action->setShortcut(QKeySequence("Ctrl+O"));
        action->setShortcutContext(Qt::WindowShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldDescr.type = DAVA::ReflectedType::Get<ProjectManagerData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any
                                         {
                                             return value.CanCast<DAVA::FilePath>() && !value.Cast<DAVA::FilePath>().IsEmpty();
                                         });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::OpenScene, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::AfterItem, "New Scene" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::AfterItem, "New Scene" }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);

        RegisterOperation(REGlobal::OpenSceneOperation.ID, this, &SceneManagerModule::OpenSceneByPath);
    }

    // Open Scene Quickly Action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/openscene.png"), QString("Open Scene Quickly"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        QList<QKeySequence> keySequences;
        keySequences << Qt::CTRL + Qt::SHIFT + Qt::Key_O;
        keySequences << Qt::ALT + Qt::SHIFT + Qt::Key_O;

        action->setShortcuts(keySequences);

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldDescr.type = DAVA::ReflectedType::Get<ProjectManagerData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any
                                         {
                                             return value.CanCast<DAVA::FilePath>() && !value.Cast<DAVA::FilePath>().IsEmpty();
                                         });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::OpenSceneQuckly, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::AfterItem, "Open Scene" }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);
    }

    // Save Scene Action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/savescene.png"), QString("Save Scene"));
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+S"));

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
        fieldDescr.type = DAVA::ReflectedType::Get<SceneData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any
                                         {
                                             return value.CanCast<SceneEditor2*>() && value.Cast<SceneEditor2*>() != nullptr;
                                         });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::SaveScene, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::AfterItem, "Open Scene Quickly" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::AfterItem, "Open Scene" }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);
    }

    // Save Scene As Action
    {
        QtAction* action = new QtAction(accessor, QString("Save Scene As"));
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+Shift+S"));

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
        fieldDescr.type = DAVA::ReflectedType::Get<SceneData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any
                                         {
                                             return value.CanCast<SceneEditor2*>() && value.Cast<SceneEditor2*>() != nullptr;
                                         });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::SaveSceneAs, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::AfterItem, "Save Scene" }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* action = new QAction(nullptr);
        action->setObjectName(QStringLiteral("saveSeparator"));
        action->setSeparator(true);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::AfterItem, "Save Scene As" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::AfterItem, "Save Scene" }));

        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);
    }
}

void SceneManagerModule::RegisterOperations()
{
    RegisterOperation(REGlobal::CloseAllScenesOperation.ID, this, &SceneManagerModule::CloseAllScenes);
}

void SceneManagerModule::CreateNewScene()
{
    using namespace DAVA::TArc;
    ContextManager* contextManager = GetContextManager();
    DataContext::ContextID newContext = contextManager->CreateContext();

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(newContext);
    DVASSERT(context != nullptr);

    UI* ui = GetUI();
    WaitDialogParams waitDlgParams;
    waitDlgParams.message = QStringLiteral("Creating new scene");
    waitDlgParams.needProgressBar = false;

    std::unique_ptr<WaitHandle> waitHandle = ui->ShowWaitDialog(REGlobal::MainWindowKey, waitDlgParams);

    DAVA::FilePath scenePath = QString("newscene%1.sc2").arg(++newSceneCounter).toStdString();
    SceneEditor2* scene = OpenSceneImpl(scenePath);

    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();
    sceneData->scene = scene;
    context->CreateData(std::move(sceneData));
    contextManager->ActivateContext(newContext);
}

void SceneManagerModule::OpenScene()
{
    using namespace DAVA::TArc;

    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);

    FileDialogParams params;
    params.dir = QString::fromStdString(data->GetDataSourcePath().GetAbsolutePathname());
    params.filters = QStringLiteral("DAVA Scene V2 (*.sc2)");
    params.title = QStringLiteral("Open scene file");

    QString scenePath = GetUI()->GetOpenFileName(REGlobal::MainWindowKey, params);
    OpenSceneByPath(DAVA::FilePath(scenePath.toStdString()));
}

void SceneManagerModule::OpenSceneQuckly()
{
    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    /// TODO GetFilePath should take UI interface and create widget through it
    QString path = FindFileDialog::GetFilePath(data->GetDataSourceSceneFiles(), "sc2", GetUI()->GetWindow(REGlobal::MainWindowKey));
    if (!path.isEmpty())
    {
        OpenSceneByPath(DAVA::FilePath(path.toStdString()));
    }
}

void SceneManagerModule::OpenSceneByPath(const DAVA::FilePath& scenePath)
{
    using namespace DAVA::TArc;

    if (scenePath.IsEmpty())
    {
        return;
    }

    ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::FilePath projectPath(data->GetProjectPath());

    if (!DAVA::FilePath::ContainPath(scenePath, projectPath))
    {
        ModalMessageParams params;
        params.buttons = ModalMessageParams::Ok;
        params.title = QStringLiteral("Open scene error");
        params.message = QString("Can't open scene file outside project path.\n\nScene:\n%1\n\nProject:\n%2").
                         arg(scenePath.GetAbsolutePathname().c_str())
                         .
                         arg(projectPath.GetAbsolutePathname().c_str());

        GetUI()->ShowModalMessage(REGlobal::MainWindowKey, params);
        return;
    }

    ContextManager* contextManager = GetContextManager();
    ContextAccessor* accessor = GetAccessor();

    // strange logic in my opinion, but...
    // here we check that we have only one scene, this scene was created by default (with index 1 in name) and user did nothing with this scene
    // if we find such scene, we will replace it by loaded scene
    {
        if (accessor->GetContextCount() == 1)
        {
            DataContext* activeContext = accessor->GetActiveContext();
            SceneEditor2* scene = activeContext->GetData<SceneData>()->scene;
            if (!scene->IsLoaded() && !scene->IsChanged() && scene->GetScenePath().GetFilename() == "newscene1.sc2")
            {
                contextManager->ActivateContext(DataContext::Empty);
                contextManager->DeleteContext(activeContext->GetID());
            }
        }
    }

    UI* ui = GetUI();
    WaitDialogParams waitDlgParams;
    waitDlgParams.message = QString("Opening scene\n%1").arg(scenePath.GetAbsolutePathname().c_str());
    waitDlgParams.needProgressBar = false;

    std::unique_ptr<WaitHandle> waitHandle = ui->ShowWaitDialog(REGlobal::MainWindowKey, waitDlgParams);

    SceneEditor2* scene = OpenSceneImpl(scenePath);
    DataContext::ContextID newContext = contextManager->CreateContext();
    DataContext* context = accessor->GetContext(newContext);
    DVASSERT(context != nullptr);

    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();
    sceneData->scene = scene;
    context->CreateData(std::move(sceneData));
    contextManager->ActivateContext(newContext);
}

void SceneManagerModule::SaveScene()
{
}

void SceneManagerModule::SaveSceneAs()
{
}

void SceneManagerModule::CloseAllScenes()
{
    using namespace DAVA::TArc;
    DAVA::Vector<DataContext::ContextID> contexts;
    GetAccessor()->ForEachContext([&contexts](DataContext& context)
                                  {
                                      contexts.push_back(context.GetID());
                                  });

    for (DataContext::ContextID id : contexts)
    {
        if (CloseSceneRequest(id) == false)
        {
            return;
        }
    }
}

void SceneManagerModule::OnActiveTabChanged(const DAVA::Any& contextID)
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

void SceneManagerModule::OnScenePathChanged(const DAVA::Any& scenePath)
{
    using namespace DAVA::TArc;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    UpdateTabTitle(ctx->GetID());
}

bool SceneManagerModule::CloseSceneRequest(DAVA::uint64 id)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();

    DataContext* context = accessor->GetContext(id);
    if (context == nullptr)
    {
        // This situation is undefined behavior and Assert should signalize about this
        DVASSERT(false);
        // But if we didn't find context with "id", that means that context already closed, or never exists
        // so we will not discard any user changes if simple say "Yes, scene was successfully closed"
        return true;
    }

    SceneData* data = context->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene != nullptr);

    if (CanCloseScene(data->scene))
    {
        contextManager->DeleteContext(id);
        return true;
    }

    return false;
}

///////////////////////////////
///           Helpers       ///
///////////////////////////////
void SceneManagerModule::UpdateTabTitle(DAVA::uint64 contextID)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext);
    SceneData* sceneData = activeContext->GetData<SceneData>();
    DVASSERT(sceneData);

    const DAVA::FilePath& scenePath = sceneData->GetScenePath();
    DAVA::String tabName = scenePath.GetFilename();
    DAVA::String tabTooltip = scenePath.GetAbsolutePathname();

    if (sceneData->IsSceneChanged())
    {
        tabName += "*";
    }

    SceneTabsModel* tabsModel = accessor->GetGlobalContext()->GetData<SceneTabsModel>();
    DVASSERT(tabsModel->tabs.count(contextID) > 0);
    TabDescriptor& tabDescr = tabsModel->tabs[contextID];
    tabDescr.tabTitle = tabName;
    tabDescr.tabTooltip = tabTooltip;
}

bool SceneManagerModule::CanCloseScene(SceneEditor2* scene)
{
    using namespace DAVA::TArc;

    DAVA::int32 toolsFlags = scene->GetEnabledTools();
    if (!scene->IsChanged())
    {
        if (toolsFlags)
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);
        }
        return true;
    }

    if (!IsSavingAllowed(scene))
    {
        return false;
    }

    UI* ui = GetUI();
    ModalMessageParams params;
    params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Cancel;
    params.defaultButton = ModalMessageParams::Cancel;
    params.message = "Do you want to save changes, made to scene?";
    params.title = "Scene was changed";

    ModalMessageParams::Button answer = ui->ShowModalMessage(REGlobal::MainWindowKey, params);

    if (answer == ModalMessageParams::Cancel)
    {
        return false;
    }

    if (answer == QMessageBox::No)
    {
        if (toolsFlags)
        {
            scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, false);
        }
        return true;
    }

    if (toolsFlags != 0)
    {
        scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, true);
    }

    if (!SaveSceneImpl(scene))
    {
        return false;
    }

    return true;
}

bool SceneManagerModule::IsSavingAllowed(SceneEditor2* scene)
{
    DVASSERT(scene != nullptr);
    QString warningMessage;
    if (scene->GetEnabledTools() != 0)
    {
        warningMessage = "Disable landscape editing before save!";
    }
    else if (scene->wayEditSystem->IsWayEditEnabled())
    {
        warningMessage = "Disable path editing before save!";
    }

    if (warningMessage.isEmpty())
    {
        return true;
    }

    using namespace DAVA::TArc;
    UI* ui = GetUI();

    ModalMessageParams params;
    params.buttons = ModalMessageParams::Ok;
    params.title = "Saving is not allowed";
    params.message = warningMessage;
    ui->ShowModalMessage(REGlobal::MainWindowKey, params);
    return false;
}

SceneEditor2* SceneManagerModule::OpenSceneImpl(const DAVA::FilePath& scenePath)
{
    using namespace DAVA::TArc;

    SceneEditor2* scene = new SceneEditor2();
    scene->SetScenePath(scenePath);

    ContextAccessor* accessor = GetAccessor();
    DAVA::EngineContext* engineCtx = accessor->GetEngineContext();

    if (engineCtx->fileSystem->Exists(scenePath))
    {
        DAVA::SceneFileV2::eError sceneWasLoaded = scene->LoadScene(scenePath);
        if (sceneWasLoaded != DAVA::SceneFileV2::ERROR_NO_ERROR)
        {
            ModalMessageParams params;
            params.buttons = ModalMessageParams::Ok;
            params.message = QStringLiteral("Unexpected opening error. See logs for more info.");
            params.title = QStringLiteral("Open scene error.");

            GetUI()->ShowModalMessage(REGlobal::MainWindowKey, params);
        }
    }
    scene->EnableEditorSystems();

    return scene;
}

bool SceneManagerModule::SaveSceneImpl(SceneEditor2* scene, const DAVA::FilePath& scenePath)
{
    DAVA::FilePath pathToSaveScene = scenePath;
    if (pathToSaveScene.IsEmpty())
    {
        if (scene->IsLoaded())
        {
            DAVA::FilePath currentScenePath = scene->GetScenePath();
            DVASSERT(!currentScenePath.IsEmpty());
            if (!scene->IsChanged())
            {
                return false;
            }

            pathToSaveScene = scene->GetScenePath();
        }
        else
        {
            pathToSaveScene = GetSceneSavePath(scene);
        }
    }

    if (pathToSaveScene.IsEmpty())
    {
        return false;
    }

    scene->SaveEmitters(DAVA::MakeFunction(this, &SceneManagerModule::SaveEmitterFallback));
    DAVA::SceneFileV2::eError ret = scene->SaveScene(pathToSaveScene);
    if (DAVA::SceneFileV2::ERROR_NO_ERROR != ret)
    {
        using namespace DAVA::TArc;
        UI* ui = GetUI();
        ModalMessageParams params;
        params.buttons = ModalMessageParams::Ok;
        params.title = QStringLiteral("Save error");
        params.message = QStringLiteral("An error occurred while saving the scene.See log for more info.");
        ui->ShowModalMessage(REGlobal::MainWindowKey, params);
        return false;
    }

    scene->SetScenePath(pathToSaveScene);
    return true;
}

DAVA::FilePath SceneManagerModule::GetSceneSavePath(const SceneEditor2* scene)
{
    DVASSERT(scene != nullptr);

    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DAVA::EngineContext* engineContext = accessor->GetEngineContext();

    DAVA::FilePath initialPath = scene->GetScenePath();
    if (!engineContext->fileSystem->Exists(initialPath))
    {
        ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
        DVASSERT(data != nullptr);
        DAVA::FilePath dataSourcePath = data->GetDataSourcePath();
        initialPath = dataSourcePath.MakeDirectoryPathname() + scene->GetScenePath().GetFilename();
    }

    FileDialogParams params;
    params.dir = QString::fromStdString(initialPath.GetDirectory().GetAbsolutePathname());
    params.title = QStringLiteral("Save scene as");
    params.filters = "DAVA Scene V2 (*.sc2)";
    QString saveScenePath = GetUI()->GetSaveFileName(REGlobal::MainWindowKey, params);
    return DAVA::FilePath(saveScenePath.toStdString());
}

DAVA::FilePath SceneManagerModule::SaveEmitterFallback(const DAVA::String& entityName, const DAVA::String& emitterName)
{
    DAVA::FilePath defaultPath = SettingsManager::GetValue(Settings::Internal_ParticleLastEmitterDir).AsFilePath();
    if (defaultPath.IsEmpty())
    {
        ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
        DVASSERT(data != nullptr);
        defaultPath = data->GetParticlesConfigPath().GetAbsolutePathname();
    }

    DAVA::FileSystem* fileSystem = GetAccessor()->GetEngineContext()->fileSystem;
    fileSystem->CreateDirectory(defaultPath.GetDirectory(), true);

    DAVA::String emitterYamlName = entityName + "_" + emitterName + ".yaml";

    using namespace DAVA::TArc;
    FileDialogParams params;
    params.dir = QString::fromStdString((defaultPath + emitterYamlName).GetAbsolutePathname());
    params.title = QString("Save Particle Emitter %1").arg(emitterName.c_str());
    params.filters = QStringLiteral("YAML File (*.yaml)");

    QString savePath = GetUI()->GetSaveFileName(REGlobal::MainWindowKey, params);
    DAVA::FilePath result(savePath.toStdString());
    if (!result.IsEmpty())
    {
        SettingsManager::SetValue(Settings::Internal_ParticleLastEmitterDir, DAVA::VariantType(result));
    }

    return result;
}
