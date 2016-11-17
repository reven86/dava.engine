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
//#include "TArc/Controls/SceneTabbar.h"

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

    DAVA::TArc::FieldDescriptor fieldDescr;
    fieldDescr.type = DAVA::ReflectedType::Get<SceneTabsModel>();
    fieldDescr.fieldName = DAVA::FastName(DAVA::TArc::SceneTabbar::activeTabPropertyName);
    fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &SceneManagerModule::OnActiveTabChanged));

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
        placementInfo.AddPlacementPoint(CreateMenuPoint("File", { InsertionParams::eInsertionMethod::BeforeItem, "actionOpenScene" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::BeforeItem }));

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
    waitDlgParams.message = QStringLiteral("Creating new scene.");

    std::unique_ptr<WaitHandle> waitHandle = ui->ShowWaitDialog(REGlobal::MainWindowKey, waitDlgParams);

    DAVA::FilePath scenePath = QString("newscene%1.sc2").arg(++newSceneCounter).toStdString();
    SceneEditor2* scene = OpenSceneImpl(scenePath);

    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();
    sceneData->scene = scene;
    context->CreateData(std::move(sceneData));
    contextManager->ActivateContext(newContext);
}

void SceneManagerModule::CloseAllScenes()
{
    // TODO UVR
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

void SceneManagerModule::OnSceneModificationFlagChanged(const DAVA::Any& isSceneModified)
{
    using namespace DAVA::TArc;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    UpdateTabTitle(ctx->GetID());
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

void SceneManagerModule::CloseSceneRequest(DAVA::uint64 id)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();

    DataContext* context = accessor->GetContext(id);
    if (context == nullptr)
    {
        return;
    }

    SceneData* data = context->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene != nullptr);

    if (CanCloseScene(data->scene))
    {
        contextManager->DeleteContext(id);
    }
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

    if (toolsFlags)
    {
        DAVA::FilePath colorSystemTexturePath = scene->customColorsSystem->GetCurrentSaveFileName();
        bool customColorActive = (toolsFlags & SceneEditor2::LANDSCAPE_TOOL_CUSTOM_COLOR) == SceneEditor2::LANDSCAPE_TOOL_CUSTOM_COLOR;
        if (!GetAccessor()->GetEngineContext()->fileSystem->Exists(colorSystemTexturePath) && !SelectCustomColorsTexturePath())
        {
            return false;
        }

        scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, true);
    }

    if (!SaveScene(scene))
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

    if (scene->wayEditSystem->IsWayEditEnabled())
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
