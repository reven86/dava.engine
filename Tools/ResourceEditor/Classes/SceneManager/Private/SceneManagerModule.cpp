#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/Controls/SceneTabbar.h"

#include "Engine/EngineContext.h"
#include "Reflection/ReflectedType.h"
#include "Render/Renderer.h"
#include "Render/DynamicBufferAllocator.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Base/FastName.h"
#include "Main/mainwindow.h"

namespace SceneManagerDetail
{
struct SingleTab
{
    DAVA::String tabTitle;

private:
    DAVA_REFLECTION(SingleTab)
    {
        DAVA::ReflectionRegistrator<SceneManagerDetail::SingleTab>::Begin()
        .Field(DAVA::TArc::SceneTabbar::tabTitlePropertyName, &SingleTab::tabTitle)
        .End();
    }
};

class TabbarData : public DAVA::TArc::DataNode
{
public:
    DAVA::uint64 activeContexID = 0;
    DAVA::Map<DAVA::uint64, SingleTab> tabs;

private:
    DAVA_VIRTUAL_REFLECTION(TabbarData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SceneManagerDetail::TabbarData>::Begin()
        .Field(DAVA::TArc::SceneTabbar::activeTabPropertyName, &TabbarData::activeContexID)
        .Field(DAVA::TArc::SceneTabbar::tabsPropertyName, &TabbarData::tabs)
        .End();
    }
};
}

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
}

void SceneManagerModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void SceneManagerModule::OnContextWillChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne)
{
}

void SceneManagerModule::OnContextDidChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne)
{
}

void SceneManagerModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
}

void SceneManagerModule::PostInit()
{
    DAVA::TArc::UI* ui = GetUI();
    QtMainWindow* mainWindow = qobject_cast<QtMainWindow*>(ui->GetWindow(REGlobal::MainWindowKey));

    mainWindow->InjectRenderWidget(GetContextManager()->GetRenderWidget());
    mainWindow->EnableGlobalTimeout(true);

    CreateModuleControls(ui);
    CreateModuleActions(ui);
}

void SceneManagerModule::CreateModuleControls(DAVA::TArc::UI* ui)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();

    context->CreateData(std::make_unique<SceneManagerDetail::TabbarData>());
    SceneManagerDetail::TabbarData* data = context->GetData<SceneManagerDetail::TabbarData>();
    data->tabs.emplace(1, SceneManagerDetail::SingleTab{ "FirstTab" });
    data->tabs.emplace(21, SceneManagerDetail::SingleTab{ "21" });
    data->tabs.emplace(33, SceneManagerDetail::SingleTab{ "21" });
    data->activeContexID = 21;

    SceneTabbar* sceneTabbar = new SceneTabbar(accessor, DAVA::Reflection::Create(context->GetData<SceneManagerDetail::TabbarData>()));
    DAVA::TArc::DockPanelInfo panelInfo;
    panelInfo.title = QString("SceneTabbar");
    panelInfo.tabbed = true;

    DAVA::TArc::PanelKey panelKey(QStringLiteral("SceneTabBar"), panelInfo);
    GetUI()->AddView(REGlobal::MainWindowKey, panelKey, sceneTabbar);
}

void SceneManagerModule::CreateModuleActions(DAVA::TArc::UI* ui)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    // New Scene action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/newscene.png"), QString("New Scene"));
        action->setShortcut(QKeySequence(Qt::Key_Control + Qt::Key_N));
        action->setShortcutContext(Qt::WindowShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldDescr.type = DAVA::ReflectedType::Get<ProjectManagerData>();
        action->SetStateUpdationFunction(QtAction::Enabling, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.CanCast<DAVA::FilePath>() && !value.Cast<DAVA::FilePath>().IsEmpty();
        });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::CreateNewScene, this));

        InsertionParams params;
        params.item = "actionOpenScene";
        params.method = InsertionParams::eInsertionMethod::BeforeItem;
        ActionPlacementInfo placementInfo(CreateMenuPoint("File/"));
        ui->AddAction(REGlobal::MainWindowKey, placementInfo, action);
    }
}

void SceneManagerModule::CreateNewScene()
{
    using namespace DAVA::TArc;
    ContextManager* contextManager = GetContextManager();
    DataContext::ContextID newContext = contextManager->CreateContext();
    contextManager->ActivateContext(newContext);

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
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
