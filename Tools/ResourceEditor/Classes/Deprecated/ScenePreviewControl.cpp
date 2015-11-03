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


ScenePreviewControl::ScenePreviewControl(const Rect & rect)
    :   UI3DView(rect)
{
    needSetCamera = false;
    editorScene = NULL;
    rotationSystem = NULL;
    RecreateScene();
    
    SetInputEnabled(true, true);
}
    
ScenePreviewControl::~ScenePreviewControl()
{
    ReleaseScene();

    SafeRelease(editorScene);
    rotationSystem = NULL;
}

void ScenePreviewControl::Input(DAVA::UIEvent *event)
{
    UI3DView::Input(event);
}

void ScenePreviewControl::RecreateScene()
{
    if(editorScene)
    {
        SetScene(NULL);
        ReleaseScene();
    }
    
    editorScene = new Scene();

    rotationSystem = new RotationControllerSystem(editorScene);
    rotationSystem->SetRotationSpeeed(0.10f);
    editorScene->AddSystem(rotationSystem, (MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT)), Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    SetScene(editorScene);
}

void ScenePreviewControl::ReleaseScene()
{
    SafeRelease(editorScene);
    currentScenePath = FilePath();
}

int32 ScenePreviewControl::OpenScene(const FilePath &pathToFile)
{
    ReleaseScene();
    RecreateScene();
    
    int32 retError = SceneFileV2::ERROR_NO_ERROR;
    if(pathToFile.IsEqualToExtension(".sce"))
    {
        SceneFile *file = new SceneFile();
        file->SetDebugLog(false);
        if(!file->LoadScene(pathToFile, editorScene))
        {
            retError = ERROR_CANNOT_OPEN_FILE;
        }
        
        SafeRelease(file);
    }
    else if(pathToFile.IsEqualToExtension(".sc2"))
    {
        SceneFileV2 *file = new SceneFileV2();
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
    
    if(needSetCamera)
    {
        needSetCamera = false;
        SetupCamera();
    }
}

void ScenePreviewControl::CreateCamera()
{
    needSetCamera = true;

    Camera * cam = new Camera();
    //cam->SetDebugFlags(Entity::DEBUG_DRAW_ALL);
    cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));

    cam->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

    ScopedPtr<Entity> node(new Entity());
    node->SetName("preview-camera");
    node->AddComponent(new CameraComponent(cam));
    node->AddComponent(new DAVA::RotationControllerComponent());
    editorScene->AddNode(node);
    editorScene->AddCamera(cam);
    editorScene->SetCurrentCamera(cam);

    SafeRelease(cam);
}

void ScenePreviewControl::SetupCamera()
{
    Camera *camera = editorScene->GetCurrentCamera();
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