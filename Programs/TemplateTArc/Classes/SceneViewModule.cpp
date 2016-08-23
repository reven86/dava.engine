#include "SceneViewModule.h"
#include "SceneViewData.h"
#include "SceneViewOperations.h"

#include "WindowSubSystem/UI.h"
#include "WindowSubSystem/ActionUtils.h"
#include "TArcCore/ContextAccessor.h"
#include "TArcCore/ContextManager.h"

#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/UIScreenManager.h"
#include "Engine/Public/EngineContext.h"
#include "Engine/Public/Window.h"
#include "Math/Rect.h"

#include <QAction>
#include <QUrl>
#include <QString>

SceneViewModule::SceneViewModule()
    : windowKey(DAVA::FastName("TemplateTArc"))
{
}

void SceneViewModule::OnRenderSystemInitialized(DAVA::Window& w)
{
    w.sizeScaleChanged.Connect(DAVA::MakeFunction(this, &SceneViewModule::OnWindowResized));
}

void SceneViewModule::OnContextCreated(tarc::DataContext& context)
{
    context.CreateData(std::make_unique<SceneViewData>());
}

void SceneViewModule::OnContextDeleted(tarc::DataContext& context)
{
}

void SceneViewModule::PostInit()
{
    SetupRenderWidget();
    SetupActions();

    RegisterOperation<void, SceneViewModule, const DAVA::String&>(SceneViewOperations::OpenScene, this, &SceneViewModule::OpenScene);
}

void SceneViewModule::OnWindowResized(DAVA::Window& w, DAVA::float32 width, DAVA::float32 height, DAVA::float32 scaleX, DAVA::float32 scaleY)
{
    uiScreen->SetSize(DAVA::Vector2(width, height));
    ui3dView->SetSize(DAVA::Vector2(width, height));
}

void SceneViewModule::SetupRenderWidget()
{
    tarc::UI& ui = GetUI();
    tarc::CentralPanelInfo info;
    ui.AddView(windowKey, tarc::PanelKey("RenderWidget", info), GetContextManager().GetRenderWidget());

    ui3dView.reset(new DAVA::UI3DView(DAVA::Rect(0, 0, 0, 0)));
    ui3dView->SetInputEnabled(true, true);
    ui3dView->GetOrCreateComponent<DAVA::UIFocusComponent>();

    uiScreen.reset(new DAVA::UIScreen());

    DAVA::EngineContext& engineContext = GetAccessor().GetEngineContext();
    const int davaUIScreenID = 0;

    engineContext.uiScreenManager->RegisterScreen(davaUIScreenID, uiScreen.get());
    engineContext.uiScreenManager->SetScreen(davaUIScreenID);
}

void SceneViewModule::SetupActions()
{
    tarc::UI& ui = GetUI();
    tarc::ActionPlacementInfo info(tarc::CreateMenuPoint("File"));
    info.AddPlacementPoint(tarc::CreateToolbarPoint("FileToolbar"));

    QAction* action = new QAction(QIcon(":/icons/openscene.png"), "Open", nullptr);
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));

    connections.AddConnection(action, &QAction::triggered, [this]()
    {
        OpenScene();
    });
    
    ui.AddAction(windowKey, info, action);
}

void SceneViewModule::OpenScene()
{
    tarc::UI& ui = GetUI();
    tarc::ContextAccessor& accessor = GetAccessor();
    if (accessor.HasActiveContext())
    {
        tarc::ModalMessageParams params;
        params.title = "Error";
        params.message = "Scene already opened.";
        params.buttons = tarc::ModalMessageParams::Ok;
        ui.ShowModalMessage(windowKey, params);
        return;
    }

    tarc::FileDialogParams params;
    params.title = QString("Open Scene");
    params.filters = QString("DAVA Scene (*.sc2)");
    QString path = GetUI().GetOpenFileName(windowKey);
    if (!path.isEmpty())
    {
        OpenScene(path.toStdString());
    }

}

void SceneViewModule::OpenScene(const DAVA::String& scenePath)
{
    DVASSERT(ui3dView);
    DVASSERT(uiScreen);

    tarc::UI& ui = GetUI();
    tarc::ContextAccessor& accessor = GetAccessor();

    tarc::ContextManager& manager = GetContextManager();
    tarc::DataContext::ContextID id = manager.CreateContext();
    manager.ActivateContext(id);

    SceneViewData& data = accessor.GetActiveContext().GetData<SceneViewData>();

    tarc::WaitDialogParams waitDialogParams;
    waitDialogParams.needProgressBar = false;
    waitDialogParams.message = QString("Opening scene: %1").arg(scenePath.c_str());
    std::unique_ptr<tarc::WaitHandle> wiatHandle = ui.ShowWaitDialog(windowKey, waitDialogParams);

    DAVA::Texture::SetDefaultGPU(DAVA::GPU_ORIGIN);
    DAVA::ScopedPtr<DAVA::Scene> scene(new DAVA::Scene());
    data.SetScene(scene);
    scene->LoadScene(DAVA::FilePath(scenePath));

    DAVA::WASDControllerSystem* wasdSystem = new DAVA::WASDControllerSystem(scene);
    wasdSystem->SetMoveSpeed(10.0f);
    scene->AddSystem(wasdSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::WASD_CONTROLLER_COMPONENT),
        DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    DAVA::RotationControllerSystem* rotationSystem = new DAVA::RotationControllerSystem(scene);
    scene->AddSystem(rotationSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::ROTATION_CONTROLLER_COMPONENT),
        DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS | DAVA::Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    DAVA::ScopedPtr<DAVA::Camera> topCamera(new DAVA::Camera());
    topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
    topCamera->SetPosition(DAVA::Vector3(-50.0f, 0.0f, 50.0f));
    topCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
    DAVA::float32 cameraFov = 70.0f;
    DAVA::float32 cameraNear = 1.0f;
    DAVA::float32 cameraFar = 5000.0f;
    topCamera->SetupPerspective(cameraFov, 320.0f / 480.0f, cameraNear, cameraFar);
    topCamera->SetAspect(1.0f);

    DAVA::ScopedPtr<DAVA::Entity> topCameraEntity(new DAVA::Entity());
    topCameraEntity->SetName("debug-camera");
    topCameraEntity->SetNotRemovable(true);
    topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
    topCameraEntity->AddComponent(new DAVA::WASDControllerComponent());
    topCameraEntity->AddComponent(new DAVA::RotationControllerComponent());
    if (scene->GetChildrenCount() > 0)
    {
        scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));
    }
    else
    {
        scene->AddNode(topCameraEntity);
    }

    // set current default camera
    if (nullptr == scene->GetCurrentCamera())
    {
        scene->SetCurrentCamera(topCamera);
    }

    scene->AddCamera(topCamera);

    ui3dView->SetScene(scene);
    uiScreen->AddControl(ui3dView);

    ui.ShowMessage(windowKey, QString("Opened Scene : %1").arg(scenePath.c_str()));
}
