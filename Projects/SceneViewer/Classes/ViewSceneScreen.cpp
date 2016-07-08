#include "ViewSceneScreen.h"
#include "GameCore.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

using namespace DAVA;

void ViewSceneScreen::LoadResources()
{
    if (!loaded)
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

        Entity* cameraEntity = new Entity();
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
        screenRect.dx = static_cast<float32>(screenSize.dx);
        screenRect.dy = static_cast<float32>(screenSize.dy);
        SetRect(screenRect);
        ScopedPtr<UI3DView> sceneView(new UI3DView(screenRect));
        sceneView->SetScene(scene);
        //sceneView->SetFrameBufferScaleFactor(0.5f);
        //sceneView->SetDrawToFrameBuffer(true);
        AddControl(sceneView);

        ScopedPtr<UIButton> backButton(CreateButton(Rect(0, 0, 90, 30), L"Back"));
        backButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ViewSceneScreen::OnBack));
        backButton->SetDebugDraw(true);
        AddControl(backButton);

        ScopedPtr<UIButton> reloadShadersButton(CreateButton(Rect(100, 0, 190, 30), L"Reload Shaders"));
        reloadShadersButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ViewSceneScreen::OnReloadShaders));
        reloadShadersButton->SetDebugDraw(true);
        AddControl(reloadShadersButton);

        DVASSERT(info == NULL);
        info = new UIStaticText(Rect(0, 0, 35, 30.f));
        info->SetFont(font);
        info->SetTextColor(Color::White);
        info->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
        AddControl(info);

        moveJoyPAD = new UIJoypad(Rect(0, screenRect.dy - 200.f, 200.f, 200.f));
        moveJoyPAD->SetDebugDraw(true);
        AddControl(moveJoyPAD);
        moveJoyPAD->Release();
    }
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

void ViewSceneScreen::OnBack(BaseObject* caller, void* param, void* callerData)
{
    SetPreviousScreen();
}

void ViewSceneScreen::OnReloadShaders(DAVA::BaseObject* caller, void* param, void* callerData)
{
    ShaderDescriptorCache::RelaoadShaders();

    List<NMaterial*> materials;
    scene->GetDataNodes(materials);
    for (auto material : materials)
    {
        material->InvalidateRenderVariants();
    }

    const Map<uint64, NMaterial*>& particleInstances = scene->particleEffectSystem->GetMaterialInstances();
    for (auto material : particleInstances)
    {
        material.second->InvalidateRenderVariants();
    }

    DAVA::Set<DAVA::NMaterial*> materialList;
    scene->foliageSystem->CollectFoliageMaterials(materialList);
    for (auto material : materialList)
    {
        if (material)
            material->InvalidateRenderVariants();
    }

    scene->renderSystem->GetDebugDrawer()->InvalidateMaterials();
    scene->renderSystem->SetForceUpdateLights();
    
#define INVALIDATE_2D_MATERIAL(material) \
    if (RenderSystem2D::material) \
        RenderSystem2D::material->InvalidateRenderVariants();

    INVALIDATE_2D_MATERIAL(DEFAULT_2D_COLOR_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_FILL_ALPHA_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL)
    
#undef INVALIDATE_2D_MATERIAL
}

void ViewSceneScreen::Draw(const DAVA::UIGeometricData& geometricData)
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

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::NUMPAD6))
        cursorPosition.x += timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(Key::NUMPAD4))
        cursorPosition.x -= timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(Key::NUMPAD8))
        cursorPosition.y += timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(Key::NUMPAD2))
        cursorPosition.y -= timeElapsed / 16.f;
    if (keyboard.IsKeyPressed(Key::SPACE))
        wasdSystem->SetMoveSpeed(30.f);
    else
        wasdSystem->SetMoveSpeed(10.f);

    Camera* camera = scene->GetDrawCamera();
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

    if (framesTime > INFO_UPDATE_TIME)
    {
        int32 fps = (int32)(frameCounter / framesTime);
        info->SetText(Format(L"FPS: %d", fps));

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

void ViewSceneScreen::Input(UIEvent* currentInput)
{
    if (currentInput->phase == UIEvent::Phase::CHAR)
    {
        if (currentInput->keyChar == '+')
            cursorSize *= 1.25f;
        if (currentInput->keyChar == '-')
            cursorSize *= .8f;
    }

    BaseScreen::Input(currentInput);
}