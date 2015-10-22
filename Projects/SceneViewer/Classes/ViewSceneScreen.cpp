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


#include "ViewSceneScreen.h"
#include "GameCore.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

using namespace DAVA;

ViewSceneScreen::ViewSceneScreen()
    : BaseScreen()
    , info(NULL)
    , frameCounter(0)
    , framesTime(0.f)
    , drawTime(0)
    , updateTime(0)
    , cursorSize(.1f)
{
}

void ViewSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();
 
    scene = new Scene();
    scene->LoadScene(GameCore::Instance()->GetScenePath());

/*
{
    Entity* hullNode = entity->FindByName("hull")->Clone();


    //create hull hierarchy to collapse in skinned mesh
    Entity * hullSkeletonRoot = new Entity();
    hullSkeletonRoot->SetName("hull");
    hullSkeletonRoot->AddNode(hullNode);

    Vector<SkeletonComponent::JointConfig> hullJointsConfig;
    RenderObject * skinnedHullObject = MeshUtils::CreateSkinnedMesh(hullSkeletonRoot, hullJointsConfig);
    ((RenderComponent *)hullNode->GetOrCreateComponent(Component::RENDER_COMPONENT))->SetRenderObject(skinnedHullObject);
    skinnedHullObject->Release();
    hullSkeletonRoot->Release();

    Matrix4 x;
    x.CreateTranslation(Vector3(-20,0,0));

//    hullNode->SetLocalTransform( x );
    hullNode->RemoveAllChildren();
    scene->AddNode( hullNode );

    SkeletonComponent * hullSkeleton = new SkeletonComponent();
    hullSkeleton->SetConfigJoints(hullJointsConfig);
    hullNode->AddComponent(hullSkeleton);

    
    Light*          light   = new Light();
    LightComponent* light_c = new LightComponent( light );
    Entity*         light_e = new Entity();
    Matrix4         light_x; light_x.Identity();

    light_c->SetLightType( Light::TYPE_DIRECTIONAL );
    light_c->SetDirection( Vector3(0,-1,0) );

    light_e->SetLocalTransform( light_x );
    
    light_e->SetName( "test-light" );
    light_e->AddComponent( light_c );
    scene->AddNode( light_e );
}
*/



    ScopedPtr<Camera> camera(new Camera());
    
    VirtualCoordinatesSystem* vcs = DAVA::VirtualCoordinatesSystem::Instance();
	float32 aspect = (float32)vcs->GetVirtualScreenSize().dy / (float32)vcs->GetVirtualScreenSize().dx;    
	camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
	camera->SetLeft(Vector3(1, 0, 0));
	camera->SetUp(Vector3(0, 0, 1.f));
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetPosition(Vector3(0, -45, 10));
    //camera->SetPosition(Vector3(0, -10, 1));    
    
    scene->AddCamera(camera);    
    scene->SetCurrentCamera(camera);
    
    Entity * cameraEntity = new Entity();
    cameraEntity->AddComponent(new CameraComponent(camera));
    cameraEntity->AddComponent(new WASDControllerComponent());
    cameraEntity->AddComponent(new RotationControllerComponent());
    scene->AddNode(cameraEntity);
    cameraEntity->Release();

    rotationControllerSystem = new RotationControllerSystem(scene);
    scene->AddSystem(rotationControllerSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT),
        Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

    wasdSystem = new WASDControllerSystem(scene);
    scene->AddSystem(wasdSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT),
        Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    Rect screenRect = GetRect();
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    screenRect.dx = screenSize.dx;
    screenRect.dy = screenSize.dy;
    SetRect(screenRect);
    ScopedPtr<UI3DView> sceneView(new UI3DView(screenRect));
    sceneView->SetScene(scene);
    AddControl(sceneView);
    
    ScopedPtr<UIButton> backButton(CreateButton(Rect(0, 0, 90, 30), L"Back"));
    backButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ViewSceneScreen::OnBack));
    backButton->SetDebugDraw(true);
    AddControl(backButton);
   
    DVASSERT(info == NULL);
    info = new UIStaticText(Rect(0, 0, screenRect.dx, 30.f));
    info->SetFont(font);
    info->SetTextColor(Color::White);
    info->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    
    AddControl(info);
    
    moveJoyPAD = new UIJoypad(Rect(0, screenRect.dy - 200.f, 200.f, 200.f));
    moveJoyPAD->SetDebugDraw(true);
    AddControl(moveJoyPAD);
    moveJoyPAD->Release();
}

void ViewSceneScreen::UnloadResources()
{
    if (scene)
    {
        scene->RemoveSystem(wasdSystem);
        scene->RemoveSystem(rotationControllerSystem);
    }
    SafeDelete(wasdSystem);
    SafeDelete(rotationControllerSystem);

    SafeRelease(scene);
    SafeRelease(info);
    
    BaseScreen::UnloadResources();
}

void ViewSceneScreen::OnBack(BaseObject *caller, void *param, void *callerData)
{
    SetPreviousScreen();
}

void ViewSceneScreen::Draw(const DAVA::UIGeometricData &geometricData)
{
    uint64 startTime = SystemTimer::Instance()->GetAbsoluteNano();


    BaseScreen::Draw(geometricData);

    drawTime += (SystemTimer::Instance()->GetAbsoluteNano() - startTime);
}

void ViewSceneScreen::Update(float32 timeElapsed)
{
    uint64 startTime = SystemTimer::Instance()->GetAbsoluteNano();

    BaseScreen::Update(timeElapsed);
    
    updateTime += (SystemTimer::Instance()->GetAbsoluteNano() - startTime);

    UpdateInfo(timeElapsed);

    KeyboardDevice & keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(DVKEY_NUMPAD6))
        cursorPosition.x += timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(DVKEY_NUMPAD4))
        cursorPosition.x -= timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(DVKEY_NUMPAD8))
        cursorPosition.y += timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(DVKEY_NUMPAD2))
        cursorPosition.y -= timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(DVKEY_SPACE))
        wasdSystem->SetMoveSpeed(30.f);
    else
        wasdSystem->SetMoveSpeed(10.f);
    
    Camera * camera = scene->GetDrawCamera();
    Vector2 joypadPos = moveJoyPAD->GetDigitalPosition();
    Vector3 cameraMoveOffset = (joypadPos.x * camera->GetLeft() - joypadPos.y * camera->GetDirection()) * timeElapsed * 20.f;
    
    camera->SetPosition(camera->GetPosition() + cameraMoveOffset);
    camera->SetTarget(camera->GetTarget() + cameraMoveOffset);
}

static const float32 INFO_UPDATE_TIME = 1.0f;
void ViewSceneScreen::UpdateInfo(float32 timeElapsed)
{
    ++frameCounter;
    framesTime += timeElapsed;
    
    if(framesTime > INFO_UPDATE_TIME)
    {
        int32 fps = (int32)(frameCounter / framesTime);
        info->SetText(Format(L"FPS: %d  Draw: %ldns  Update: %ldns", fps, drawTime / frameCounter, updateTime / frameCounter));
        
        framesTime -= INFO_UPDATE_TIME;
        frameCounter = 0;
        
        drawTime = updateTime = 0;
    }
}

void ViewSceneScreen::DidAppear()
{
    framesTime = 0.f;
    frameCounter = 0;
    
    drawTime = updateTime = 0;
    
    info->SetText(L"");
}

void ViewSceneScreen::Input(UIEvent *currentInput)
{
    if (currentInput->phase == UIEvent::PHASE_KEYCHAR)
    {
        if (currentInput->keyChar == '+')
            cursorSize *= 1.25f;
        if (currentInput->keyChar == '-')
            cursorSize *= .8f;
    }

    BaseScreen::Input(currentInput);
}