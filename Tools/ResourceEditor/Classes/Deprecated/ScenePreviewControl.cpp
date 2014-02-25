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

// ***************** PreviewCameraController *************** //
PreviewCameraController::PreviewCameraController()
    :   CameraController(35)
{
    angleVertical = 0.f;
    angleHorizontal = 0.f;
    
    moveStartPt = Vector2(0, 0);
    moveStopPt = Vector2(0, 0);
    
    zoomStartPt = Vector2(0, 0);
    zoomStopPt = Vector2(0, 0);

    radius = 10.f;
    zoomLevel = 1.f;

    controlHeight = 100;
}

void PreviewCameraController::SetScene(DAVA::Scene *scene)
{
    CameraController::SetScene(scene);

    angleVertical = 0.f;
    angleHorizontal = 0.f;
    
    moveStartPt = Vector2(0, 0);
    moveStopPt = Vector2(0, 0);
    
    zoomStartPt = Vector2(0, 0);
    zoomStopPt = Vector2(0, 0);
    
    radius = 10.f;
    zoomLevel = 1.f;
}

void PreviewCameraController::SetRadius(float32 _radius)
{
    radius = _radius;
}

void PreviewCameraController::Input(DAVA::UIEvent *event)
{
	if (currScene == 0)
		return;
	Camera * camera = currScene->GetCurrentCamera();
    if (!camera)return;

    CameraController::Input(event);
    
    if(UIEvent::BUTTON_1 == event->tid)
    {
        if(UIEvent::PHASE_BEGAN == event->phase)
        {
            moveStartPt = moveStopPt = event->point;
        }
        else if(UIEvent::PHASE_DRAG == event->phase)
        {
            moveStartPt = moveStopPt;
            moveStopPt = event->point;
            UpdateCamera();
        }
        else if(UIEvent::PHASE_ENDED == event->phase)
        {
            moveStartPt = moveStopPt;
            moveStopPt = event->point;
            UpdateCamera();
        }
    }
    else if(UIEvent::BUTTON_2 == event->tid)
    {
        if(UIEvent::PHASE_BEGAN == event->phase)
        {
            zoomStartPt = zoomStopPt = event->point;
        }
        else if(UIEvent::PHASE_DRAG == event->phase)
        {
            zoomStartPt = zoomStopPt;
            zoomStopPt = event->point;
            UpdateCamera();
        }
        else if(UIEvent::PHASE_ENDED == event->phase)
        {
            zoomStartPt = zoomStopPt;
            zoomStopPt = event->point;
            UpdateCamera();
        }
    }
}

void PreviewCameraController::SetControlHeight(int32 height)
{
    controlHeight = height;
}

void PreviewCameraController::UpdateCamera()
{
	if (currScene == 0)
		return;
	Camera * camera = currScene->GetCurrentCamera();
	
    Vector2 zoom = zoomStopPt - zoomStartPt;
    if(Vector2(0, 0) != zoom)
    {
        zoomStartPt = zoomStopPt;
        float32 delta = zoom.y;
        if(0 < delta)
        {
            zoomLevel *= (1 + zoom.y/controlHeight);   
        }
        else
        {
            zoomLevel /= (1 - zoom.y/controlHeight);   
        }
    }

    
    Vector3 target = camera->GetTarget();
    angleHorizontal += (moveStopPt.x - moveStartPt.x);
    angleVertical += (moveStopPt.y - moveStartPt.y);
    
    Vector3 position = target - radius * zoomLevel * Vector3(sinf(DegToRad(angleHorizontal)), 
                                                 cosf(DegToRad(angleHorizontal)), 
                                                 sinf(DegToRad(angleVertical)));
    
    
    camera->SetPosition(position);
}


// ***************** ScenePreviewControl *************** //
ScenePreviewControl::ScenePreviewControl(const Rect & rect)
    :   UI3DView(rect)
{
    needSetCamera = false;
    sceCamera = false;
    rootNode = NULL;
    
    editorScene = new Scene();

    // Camera setup
    cameraController = new PreviewCameraController();
    cameraController->SetRadius(10.f);
    cameraController->SetControlHeight((int32)rect.dy);
    
    SetScene(editorScene);
    cameraController->SetScene(editorScene);
}
    
ScenePreviewControl::~ScenePreviewControl()
{
    ReleaseScene();

    SafeRelease(editorScene);
    SafeRelease(cameraController);
}


void ScenePreviewControl::Input(DAVA::UIEvent *event)
{
    cameraController->Input(event);
    
    UIControl::Input(event);
}

void ScenePreviewControl::RecreateScene()
{
    if(editorScene)
    {
        SetScene(NULL);
        cameraController->SetScene(NULL);
        
        SafeRelease(editorScene);
    }
    
    editorScene = new Scene();
    SetScene(editorScene);
    cameraController->SetScene(editorScene);
}

void ScenePreviewControl::ReleaseScene()
{
    if(!currentScenePath.IsEmpty())
    {
        editorScene->RemoveNode(rootNode);
        editorScene->ReleaseRootNode(currentScenePath);
        
        rootNode = NULL;
        currentScenePath = FilePath();
    }
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
    
    if(SceneFileV2::ERROR_NO_ERROR == retError)
    {
        rootNode = editorScene->GetRootNode(pathToFile);
        if(rootNode)
        {
			rootNode = rootNode->Clone();

            currentScenePath = pathToFile;
            editorScene->AddNode(rootNode);
			rootNode->Release();
            
            needSetCamera = true;
            Camera *cam = editorScene->GetCamera(0);
            if(!cam)
            {
                Camera * cam = new Camera();
                //cam->SetDebugFlags(Entity::DEBUG_DRAW_ALL);
                cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
                cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
                cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
                
                cam->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f); 
                

                ScopedPtr<Entity> node(new Entity());
                node->SetName("preview-camera");
                node->AddComponent(new CameraComponent(cam));
                editorScene->AddNode(node);
                editorScene->AddCamera(cam);
                editorScene->SetCurrentCamera(cam);
                cameraController->SetScene(editorScene);
                
                SafeRelease(cam);
                
                sceCamera = false;
            }
            else
            {
                sceCamera = true;
            }
        }
    }
    
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
    
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
}

void ScenePreviewControl::SetupCamera()
{
    Camera *camera = editorScene->GetCamera(0);
    if (camera)
    {
        AABBox3 sceneBox = rootNode->GetWTMaximumBoundingBoxSlow();
        Vector3 target = sceneBox.GetCenter();
        camera->SetTarget(target);
        Vector3 dir = (sceneBox.max - sceneBox.min); 
        float32 radius = dir.Length();
        if(sceCamera)
        {
            radius = 5.f;
        }
        
        editorScene->SetCurrentCamera(camera);
        editorScene->SetClipCamera(camera);
        
		cameraController->SetScene(editorScene);
        cameraController->SetRadius(radius);
        cameraController->UpdateCamera();
    }
}