#include "ViewSceneScreen.h"
#include "GameCore.h"

#include <Math/MathHelpers.h>

namespace ViewSceneScreenDetails
{
const float32 INFO_UPDATE_INTERVAL_SEC = 1.0f;
}

ViewSceneScreen::ViewSceneScreen(DAVA::Engine& engine)
    : fpsMeter(ViewSceneScreenDetails::INFO_UPDATE_INTERVAL_SEC)
    , gridTest(engine, this)
{
}

void ViewSceneScreen::LoadResources()
{
    using namespace DAVA;

    if (!loaded)
    {
        BaseScreen::LoadResources();

        selectedScenePath = GameCore::Instance()->GetScenePath();
        scene = new Scene();
        scene->LoadScene(selectedScenePath);

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

        ScopedPtr<Camera> camera(new Camera);

        VirtualCoordinatesSystem* vcs = UIControlSystem::Instance()->vcs;
        vcs->RegisterAvailableResourceSize(vcs->GetVirtualScreenSize().dx, vcs->GetVirtualScreenSize().dy, "Gfx");
        float32 aspect = static_cast<float32>(vcs->GetVirtualScreenSize().dy) / static_cast<float32>(vcs->GetVirtualScreenSize().dx);
        camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
        SetCameraAtCenter(camera);
        //camera->SetPosition(Vector3(0, -10, 1));
        scene->AddCamera(camera);
        scene->SetCurrentCamera(camera);

        Entity* cameraEntity = new Entity();
        cameraEntity->AddComponent(new CameraComponent(camera));
        cameraEntity->AddComponent(new WASDControllerComponent());
        cameraEntity->AddComponent(new RotationControllerComponent());
        scene->AddNode(cameraEntity);
        cameraEntity->Release();

        rotationControllerSystem.reset(new DAVA::RotationControllerSystem(scene));
        scene->AddSystem(rotationControllerSystem.get(), MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT),
                         Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

        wasdSystem.reset(new WASDControllerSystem(scene));
        scene->AddSystem(wasdSystem.get(), MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT),
                         Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

        Rect screenRect = GetRect();
        Size2i screenSize = UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
        screenRect.dx = static_cast<float32>(screenSize.dx);
        screenRect.dy = static_cast<float32>(screenSize.dy);
        SetRect(screenRect);
        sceneView = new UI3DView(screenRect);
        sceneView->SetScene(scene);
        //sceneView->SetFrameBufferScaleFactor(0.5f);
        //sceneView->SetDrawToFrameBuffer(true);
        AddControl(sceneView);

        AddMenuControl();
        AddFileDialogControl();
        AddInfoTextControl();
        AddJoypadControl();
    }
}

void ViewSceneScreen::AddMenuControl()
{
    menu.reset(new Menu(nullptr, this, font, Rect(10, 30, 250, 60)));
    Menu* mainSubMenu = menu->AddSubMenuItem(L"Menu");

    Menu* selectSceneSubMenu = mainSubMenu->AddSubMenuItem(L"Select scene");
    mainSubMenu->AddActionItem(L"Reload shaders", Message(this, &ViewSceneScreen::OnButtonReloadShaders));
    mainSubMenu->AddActionItem(L"Performance test", Message(this, &ViewSceneScreen::OnButtonPerformanceTest));
    mainSubMenu->AddBackItem();

    selectSceneSubMenu->AddActionItem(L"Select from ~res", Message(this, &ViewSceneScreen::OnButtonSelectFromRes));
    selectSceneSubMenu->AddActionItem(L"Select from documents", Message(this, &ViewSceneScreen::OnButtonSelectFromDoc));
    selectSceneSubMenu->AddActionItem(L"Select from ext storage", Message(this, &ViewSceneScreen::OnButtonSelectFromExt));
    selectSceneSubMenu->AddBackItem();
}

void ViewSceneScreen::AddFileDialogControl()
{
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/korinna.ttf");
    fileSystemDialog->SetDelegate(this);
    fileSystemDialog->SetExtensionFilter(".sc2");
    fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);
}

void ViewSceneScreen::AddJoypadControl()
{
    moveJoyPAD = new UIJoypad(Rect(10, GetRect().dy - 210.f, 200.f, 200.f));
    moveJoyPAD->SetDebugDraw(true);
    moveJoyPAD->SetStickSprite("~res:/Gfx/Joypad/joypad.tex", 0);
    AddControl(moveJoyPAD);
}

void ViewSceneScreen::AddInfoTextControl()
{
    info.reset(new UIStaticText(Rect(GetRect().dy / 2, 30, 100, 30)));
    info->SetFont(font);
    info->SetTextColor(Color::White);
    info->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    AddControl(info);
}

void ViewSceneScreen::SetCameraAtCenter(Camera* camera)
{
    camera->SetLeft(DAVA::Vector3(1, 0, 0));
    camera->SetUp(DAVA::Vector3(0, 0, 1.f));
    camera->SetTarget(DAVA::Vector3(0, 0, 0));
    camera->SetPosition(DAVA::Vector3(0, -45, 10));
}

void ViewSceneScreen::OnFileSelected(UIFileSystemDialog* forDialog, const FilePath& pathToFile)
{
    selectedScenePath = pathToFile;
}

void ViewSceneScreen::OnFileSytemDialogCanceled(UIFileSystemDialog* forDialog)
{
}

void ViewSceneScreen::OnButtonSelectFromRes(BaseObject* caller, void* param, void* callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~res:/");
    fileSystemDialog->Show(this);
}

void ViewSceneScreen::OnButtonSelectFromDoc(BaseObject* caller, void* param, void* callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~doc:/../");
    fileSystemDialog->Show(this);
}

void ViewSceneScreen::OnButtonSelectFromExt(BaseObject* caller, void* param, void* callerData)
{
    DVASSERT(fileSystemDialog);

    auto storageList = DeviceInfo::GetStoragesList();
    for (const auto& storage : storageList)
    {
        if (storage.type == DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL ||
            storage.type == DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL)
        {
            fileSystemDialog->SetCurrentDir(storage.path);
            fileSystemDialog->Show(this);
            return;
        }
    }
}

void ViewSceneScreen::OnButtonPerformanceTest(BaseObject* caller, void* param, void* callerData)
{
    if (gridTest.GetState() != GridTest::Finished)
        return;

    gridTest.Start(sceneView);
}

void ViewSceneScreen::UnloadResources()
{
    if (scene)
    {
        scene->RemoveSystem(wasdSystem.get());
        scene->RemoveSystem(rotationControllerSystem.get());
    }

    scene.reset();
    info.reset();
    fileSystemDialog.reset();

    BaseScreen::UnloadResources();
}

void ViewSceneScreen::OnButtonReloadShaders(DAVA::BaseObject* caller, void* param, void* callerData)
{
    ShaderDescriptorCache::ReloadShaders();

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

void ViewSceneScreen::ReloadScene()
{
    static int unloaded = 0;

    if (!unloaded)
    {
        UnloadResources();
        unloaded = true;
        return;
    }

    if (++unloaded == 2)
    {
        GameCore::Instance()->SetScenePath(selectedScenePath);
        LoadResources();
        unloaded = 0;
    }
}

void ViewSceneScreen::ProcessUserInput(float32 timeElapsed)
{
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

void ViewSceneScreen::Update(float32 timeElapsed)
{
    if (selectedScenePath != GameCore::Instance()->GetScenePath())
    {
        ReloadScene();
        return;
    }

    BaseScreen::Update(timeElapsed);

    UpdateInfo(timeElapsed);
    ProcessUserInput(timeElapsed);
}

void ViewSceneScreen::UpdateInfo(float32 timeElapsed)
{
    fpsMeter.Update(timeElapsed);
    if (fpsMeter.IsFpsReady())
    {
        info->SetText(Format(L"FPS: %.0f", fpsMeter.GetFps()));

        drawTime = updateTime = 0;
    }
}

void ViewSceneScreen::OnGridTestStateChanged()
{
    if (gridTest.GetState() == GridTest::Finished)
    {
        SetCameraAtCenter(scene->GetCurrentCamera());
    }
}

// void ViewSceneScreen::DidAppear()
// {
//     framesTime = 0.f;
//     frameCounter = 0;
//
//     drawTime = updateTime = 0;
//
//     info->SetText(L"");
// }

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
