/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "ScenePreviewControl.h"
#include "Deprecated/ControlsFactory.h"
#include "Deprecated/SceneValidator.h"
#include "Scene3D/Components/CameraComponent.h"

#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"

ScenePreviewControl::ScenePreviewControl(const Rect& rect)
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

    editorScene = new Scene();
    editorScene->GetMainPassConfig().priority = DAVA::PRIORITY_MAIN_2D - 5;

    rotationSystem = new RotationControllerSystem(editorScene);
    rotationSystem->SetRotationSpeeed(0.10f);
    editorScene->AddSystem(rotationSystem, (MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT)),
                           Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    SetScene(editorScene);
}

void ScenePreviewControl::ReleaseScene()
{
    SafeRelease(editorScene);
    currentScenePath = FilePath();
}

int32 ScenePreviewControl::OpenScene(const FilePath& pathToFile)
{
    ReleaseScene();
    RecreateScene();

    int32 retError = SceneFileV2::ERROR_NO_ERROR;

    if (pathToFile.IsEqualToExtension(".sc2"))
    {
        SceneFileV2* file = new SceneFileV2();
        file->EnableDebugLog(false);
        retError = file->LoadScene(pathToFile, editorScene);
        SafeRelease(file);
    }
    else
    {
        retError = ERROR_WRONG_EXTENSION;
    }

    CreateCamera();

    Set<String> errorsLogToHideDialog;
    SceneValidator::Instance()->ValidateScene(editorScene, pathToFile, errorsLogToHideDialog);

    return retError;
}

void ScenePreviewControl::Update(float32 timeElapsed)
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

    ScopedPtr<Camera> camera(new Camera());
    ScopedPtr<Entity> cameraNode(new Entity());
    camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    camera->SetPosition(sceneBox.max * std::sqrt(3.0f));
    camera->SetTarget(sceneBox.min);
    camera->SetupPerspective(70.0f, 1.0f, 1.0f, 5000.0f);
    cameraNode->SetName("preview-camera");
    cameraNode->AddComponent(new CameraComponent(camera));
    cameraNode->AddComponent(new RotationControllerComponent());

    ScopedPtr<Light> light(new Light());
    ScopedPtr<Entity> lightNode(new Entity());
    light->SetIntensity(300.0f);
    light->SetDiffuseColor(Color::White);
    light->SetAmbientColor(Color::White);
    light->SetPosition(Vector3(0.0f, 0.0f, sceneBox.max.z + std::abs(sceneBox.max.z)));
    light->SetType(Light::TYPE_POINT);
    light->AddFlag(Light::IS_DYNAMIC);
    light->AddFlag(Light::CAST_SHADOW);
    lightNode->SetName("preview-light");
    lightNode->AddComponent(new LightComponent(light));
    lightNode->SetLocalTransform(Matrix4::MakeTranslation(light->GetPosition()));

    editorScene->AddNode(cameraNode);
    editorScene->AddNode(lightNode);

    editorScene->AddCamera(camera);
    editorScene->SetCurrentCamera(camera);
    editorScene->Update(0.01f);
}

void ScenePreviewControl::SetupCamera()
{
    Camera* camera = editorScene->GetCurrentCamera();
    if (camera && editorScene)
    {
        AABBox3 sceneBox = editorScene->GetWTMaximumBoundingBoxSlow();
        Vector3 target = sceneBox.GetCenter();
        camera->SetTarget(target);
        Vector3 dir = (sceneBox.max - sceneBox.min);
        camera->SetPosition(target + dir);

        editorScene->SetCurrentCamera(camera);
        rotationSystem->RecalcCameraViewAngles(camera);
    }
}