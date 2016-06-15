#include "ScenePreviewControl.h"
#include "Deprecated/ControlsFactory.h"
#include "Deprecated/SceneValidator.h"
#include "Scene3D/Components/CameraComponent.h"

#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"

ScenePreviewControl::ScenePreviewControl(const DAVA::Rect& rect)
    : UI3DView(rect)
{
    SetInputEnabled(true, true);
    SetBasePriority(-100);
}

ScenePreviewControl::~ScenePreviewControl()
{
    ReleaseScene();

    SafeRelease(editorScene);
    rotationSystem = nullptr;
}

void ScenePreviewControl::Input(DAVA::UIEvent* event)
{
    UI3DView::Input(event);
}

void ScenePreviewControl::RecreateScene()
{
    DVASSERT(editorScene == nullptr);

    editorScene = new DAVA::Scene();
    editorScene->GetMainPassConfig().priority = DAVA::PRIORITY_MAIN_2D - 5;

    rotationSystem = new DAVA::RotationControllerSystem(editorScene);
    rotationSystem->SetRotationSpeeed(0.10f);
    editorScene->AddSystem(rotationSystem, (MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(DAVA::Component::ROTATION_CONTROLLER_COMPONENT)),
                           DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS | DAVA::Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    SetScene(editorScene);
}

void ScenePreviewControl::ReleaseScene()
{
    SafeRelease(editorScene);
    currentScenePath = DAVA::FilePath();
}

DAVA::int32 ScenePreviewControl::OpenScene(const DAVA::FilePath& pathToFile)
{
    ReleaseScene();
    RecreateScene();

    DAVA::int32 retError = DAVA::SceneFileV2::ERROR_NO_ERROR;

    if (pathToFile.IsEqualToExtension(".sc2"))
    {
        DAVA::SceneFileV2* file = new DAVA::SceneFileV2();
        file->EnableDebugLog(false);
        retError = file->LoadScene(pathToFile, editorScene);
        SafeRelease(file);
    }
    else
    {
        retError = ERROR_WRONG_EXTENSION;
    }

    CreateCamera();

    DAVA::Set<DAVA::String> errorsLogToHideDialog;
    SceneValidator::Instance()->ValidateScene(editorScene, pathToFile, errorsLogToHideDialog);

    return retError;
}

void ScenePreviewControl::Update(DAVA::float32 timeElapsed)
{
    UI3DView::Update(timeElapsed);

    if (needSetCamera)
    {
        needSetCamera = false;
        SetupCamera();
    }
}

void ScenePreviewControl::CreateCamera()
{
    auto sceneBox = editorScene->GetWTMaximumBoundingBoxSlow();

    DAVA::ScopedPtr<DAVA::Camera> camera(new DAVA::Camera());
    DAVA::ScopedPtr<DAVA::Entity> cameraNode(new DAVA::Entity());
    camera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
    camera->SetPosition(sceneBox.max * std::sqrt(3.0f));
    camera->SetTarget(sceneBox.min);
    camera->SetupPerspective(70.0f, 1.0f, 1.0f, 5000.0f);
    cameraNode->SetName("preview-camera");
    cameraNode->AddComponent(new DAVA::CameraComponent(camera));
    cameraNode->AddComponent(new DAVA::RotationControllerComponent());

    DAVA::ScopedPtr<DAVA::Light> light(new DAVA::Light());
    DAVA::ScopedPtr<DAVA::Entity> lightNode(new DAVA::Entity());
    light->SetIntensity(300.0f);
    light->SetDiffuseColor(DAVA::Color::White);
    light->SetAmbientColor(DAVA::Color::White);
    light->SetPosition(DAVA::Vector3(0.0f, 0.0f, sceneBox.max.z + std::abs(sceneBox.max.z)));
    light->SetType(DAVA::Light::TYPE_POINT);
    light->AddFlag(DAVA::Light::IS_DYNAMIC);
    light->AddFlag(DAVA::Light::CAST_SHADOW);
    lightNode->SetName("preview-light");
    lightNode->AddComponent(new DAVA::LightComponent(light));
    lightNode->SetLocalTransform(DAVA::Matrix4::MakeTranslation(light->GetPosition()));

    editorScene->AddNode(cameraNode);
    editorScene->AddNode(lightNode);

    editorScene->AddCamera(camera);
    editorScene->SetCurrentCamera(camera);
    editorScene->Update(0.01f);
}

void ScenePreviewControl::SetupCamera()
{
    DAVA::Camera* camera = editorScene->GetCurrentCamera();
    if (camera && editorScene)
    {
        DAVA::AABBox3 sceneBox = editorScene->GetWTMaximumBoundingBoxSlow();
        DAVA::Vector3 target = sceneBox.GetCenter();
        camera->SetTarget(target);
        DAVA::Vector3 dir = (sceneBox.max - sceneBox.min);
        camera->SetPosition(target + dir);

        editorScene->SetCurrentCamera(camera);
        rotationSystem->RecalcCameraViewAngles(camera);
    }
}