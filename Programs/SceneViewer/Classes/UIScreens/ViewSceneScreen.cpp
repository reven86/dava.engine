#include "ViewSceneScreen.h"
#include "SceneViewerApp.h"
#include "Settings.h"

#include <Math/MathHelpers.h>
#include <Render/2D/Sprite.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <UI/Update/UIUpdateComponent.h>

namespace ViewSceneScreenDetails
{
const DAVA::float32 ABOVE_LANDSCAPE_ELEVATION = 10.f;
const DAVA::float32 INFO_UPDATE_INTERVAL_SEC = 0.5f;

const DAVA::String BOTSPAWN_PROPERTY_VALUE = "botspawn";
const DAVA::String TANK_MODEL_PATH = "~res:/Tanks/USSR/T54S.sc2";
}

ViewSceneScreen::ViewSceneScreen(SceneViewerData& data)
    : data(data)
    , scenePath(data.scenePath)
    , scene(data.scene)
    , fpsMeter(ViewSceneScreenDetails::INFO_UPDATE_INTERVAL_SEC)
#ifdef WITH_SCENE_PERFORMANCE_TESTS
    , gridTest(data.engine, this)
#endif
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();
}

void ViewSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();
    if (!scene)
    {
        LoadScene();
    }
    AddControls();
    PlaceSceneAtScreen();
}

void ViewSceneScreen::UnloadResources()
{
    RemoveSceneFromScreen();
    RemoveControls();
    BaseScreen::UnloadResources();
}

void ViewSceneScreen::PlaceSceneAtScreen()
{
    using namespace DAVA;

    if (scene)
    {
        Camera* camera = scene->GetCurrentCamera();
        camera->SetupPerspective(70.f, data.screenAspect, 0.5f, 2500.f);
        camera->SetUp(DAVA::Vector3(0.f, 0.f, 1.f));
        SetCameraAtCenter(camera);
        //camera->SetPosition(Vector3(0, -10, 1));

        rotationControllerSystem = new DAVA::RotationControllerSystem(scene);
        scene->AddSystem(rotationControllerSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::ROTATION_CONTROLLER_COMPONENT),
                         Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT);

        wasdSystem = new WASDControllerSystem(scene);
        scene->AddSystem(wasdSystem, MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT),
                         Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

        sceneView = new DAVA::UI3DView(GetRect());
        //sceneView->SetFrameBufferScaleFactor(0.5f);
        //sceneView->SetDrawToFrameBuffer(true);
        AddControl(sceneView);
        sceneView->SetScene(scene);

        if (menu)
        {
            menu->BringAtFront();
            qualitySettingsMenuItem->SetEnabled(true);
            reloadShadersMenuItem->SetEnabled(true);
#ifdef WITH_SCENE_PERFORMANCE_TESTS
            performanceTestMenuItem->SetEnabled(true);
#endif
        }

        if (moveJoyPAD)
        {
            BringChildFront(moveJoyPAD);
        }
    }
}

void ViewSceneScreen::RemoveSceneFromScreen()
{
    if (scene)
    {
        scene->RemoveSystem(rotationControllerSystem);
        SafeDelete(rotationControllerSystem);

        scene->RemoveSystem(wasdSystem);
        SafeDelete(wasdSystem);

        RemoveControl(sceneView);
        sceneView.reset();

        if (menu)
        {
            qualitySettingsMenuItem->SetEnabled(false);
            reloadShadersMenuItem->SetEnabled(false);
            performanceTestMenuItem->SetEnabled(false);
        }
    }
}

void ViewSceneScreen::LoadScene()
{
    using namespace DAVA;

    scene = new Scene();

    SceneFileV2::eError result = scene->LoadScene(scenePath);
    if (result == SceneFileV2::ERROR_NO_ERROR)
    {
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
        scene->AddCamera(camera);
        scene->SetCurrentCamera(camera);

        ScopedPtr<Entity> cameraEntity(new Entity());
        cameraEntity->AddComponent(new CameraComponent(camera));
        cameraEntity->AddComponent(new WASDControllerComponent());
        cameraEntity->AddComponent(new RotationControllerComponent());
        scene->AddNode(cameraEntity);

        AddTanksAtScene();
    }
    else
    {
        scene.reset();
    }
}

void ViewSceneScreen::AddTanksAtScene()
{
    using namespace ViewSceneScreenDetails;
    using namespace DAVA;

    List<Entity*> spawnPoints;
    scene->GetChildEntitiesWithComponent(spawnPoints, Component::CUSTOM_PROPERTIES_COMPONENT, false);

    for (List<Entity*>::iterator it = spawnPoints.begin(); it != spawnPoints.end();)
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(*it);
        DVASSERT(props);

        if (props->IsKeyExists("type") && props->GetString("type") == BOTSPAWN_PROPERTY_VALUE)
        {
            ++it;
        }
        else
        {
            auto itDel = it++;
            spawnPoints.erase(itDel);
        }
    }

    if (!spawnPoints.empty())
    {
        ScopedPtr<Scene> tank(new Scene());
        if (SceneFileV2::eError::ERROR_NO_ERROR == tank->LoadScene(ViewSceneScreenDetails::TANK_MODEL_PATH))
        {
            for (Entity* spawnPoint : spawnPoints)
            {
                Entity* clonedTank = new Entity;
                tank->Clone(clonedTank);
                while (clonedTank->GetChildrenCount() != 0)
                {
                    spawnPoint->AddNode(clonedTank->GetChild(0));
                }
            }
        }
    }
}

void ViewSceneScreen::AddMenuControl()
{
    DVASSERT(!menu);
    DAVA::Rect rect = DAVA::Rect(10.f, 30.f, 250.f, 60.f);
    menu.reset(new Menu(nullptr, this, font, rect));

    SubMenuItem* mainSubMenuItem = menu->AddSubMenuItem(L"Menu");
    Menu* mainSubMenu = mainSubMenuItem->submenu.get();

    SubMenuItem* selectSceneSubMenuItem = mainSubMenu->AddSubMenuItem(L"Select scene");
    qualitySettingsMenuItem = mainSubMenu->AddActionItem(L"Quality settings", DAVA::Message(this, &ViewSceneScreen::OnButtonQualitySettings));
    reloadShadersMenuItem = mainSubMenu->AddActionItem(L"Reload shaders", DAVA::Message(this, &ViewSceneScreen::OnButtonReloadShaders));
    performanceTestMenuItem = mainSubMenu->AddActionItem(L"Performance test", DAVA::Message(this, &ViewSceneScreen::OnButtonPerformanceTest));
    mainSubMenu->AddBackItem();

    qualitySettingsMenuItem->SetEnabled(false);
    reloadShadersMenuItem->SetEnabled(false);
    performanceTestMenuItem->SetEnabled(false);

    Menu* selectSceneSubMenu = selectSceneSubMenuItem->submenu.get();
    selectSceneSubMenu->AddActionItem(L"Select from ~res", DAVA::Message(this, &ViewSceneScreen::OnButtonSelectFromRes));
    selectSceneSubMenu->AddActionItem(L"Select from documents", DAVA::Message(this, &ViewSceneScreen::OnButtonSelectFromDoc));
    selectSceneSubMenu->AddActionItem(L"Select from ext storage", DAVA::Message(this, &ViewSceneScreen::OnButtonSelectFromExt));
    selectSceneSubMenu->AddBackItem();
}

void ViewSceneScreen::AddFileDialogControl()
{
    DVASSERT(!fileSystemDialog);
    fileSystemDialog = new DAVA::UIFileSystemDialog("~res:/Fonts/korinna.ttf");
    fileSystemDialog->SetDelegate(this);
    fileSystemDialog->SetExtensionFilter(".sc2");
    fileSystemDialog->SetOperationType(DAVA::UIFileSystemDialog::OPERATION_LOAD);

    DAVA::UIAnchorComponent* anchor = fileSystemDialog->GetOrCreateComponent<DAVA::UIAnchorComponent>();
    anchor->SetRightAnchorEnabled(true);
    anchor->SetRightAnchor(30.f);
    anchor->SetVCenterAnchorEnabled(true);
}

void ViewSceneScreen::AddJoypadControl()
{
    DVASSERT(!moveJoyPAD);
    moveJoyPAD = new DAVA::UIJoypad(DAVA::Rect(10, GetRect().dy - 210.f, 200.f, 200.f));
    DAVA::ScopedPtr<DAVA::Sprite> stickSprite(DAVA::Sprite::CreateFromSourceFile("~res:/UI/Joypad.png", true));
    moveJoyPAD->SetStickSprite(stickSprite, 0);
    AddControl(moveJoyPAD);
}

void ViewSceneScreen::AddInfoTextControl()
{
    DVASSERT(!infoText);
    infoText = new DAVA::UIStaticText(DAVA::Rect(GetRect().dx / 2 - 150, 30, 300, 30));
    infoText->SetFont(font);
    infoText->SetTextColor(DAVA::Color::White);
    infoText->SetTextAlign(DAVA::ALIGN_HCENTER);
    AddControl(infoText);
}

void ViewSceneScreen::AddQualitySettingsDialog()
{
    DVASSERT(!qualitySettingsDialog);
    qualitySettingsDialog = new QualitySettingsDialog(data.settings);
    qualitySettingsDialog->SetParentControl(this);
    qualitySettingsDialog->SetDelegate(this);
}

void ViewSceneScreen::AddControls()
{
    AddMenuControl();
    AddFileDialogControl();
    AddInfoTextControl();
    AddJoypadControl();
    AddQualitySettingsDialog();
}

void ViewSceneScreen::RemoveControls()
{
    qualitySettingsDialog.reset();
    infoText.reset();
    moveJoyPAD.reset();
    fileSystemDialog.reset();

    menu.reset();
    qualitySettingsMenuItem = nullptr;
    reloadShadersMenuItem = nullptr;
    performanceTestMenuItem = nullptr;
}

void ViewSceneScreen::SetCameraAtCenter(DAVA::Camera* camera)
{
    DAVA::Vector3 position = DAVA::Vector3(0.f, -65.f, 10.f);

    DAVA::Landscape* landscape = FindLandscape(scene);
    if (landscape != nullptr)
    {
        DAVA::float32 landscapeHeight = 0.f;
        landscape->GetHeightAtPoint(position, landscapeHeight);
        position.z = landscapeHeight + ViewSceneScreenDetails::ABOVE_LANDSCAPE_ELEVATION;
    }

    camera->SetLeft(DAVA::Vector3(1.f, 0.f, 0.f));
    camera->SetTarget(DAVA::Vector3(0.f, 0.f, 0.f));
    camera->SetPosition(position);
}

void ViewSceneScreen::OnFileSelected(DAVA::UIFileSystemDialog* forDialog, const DAVA::FilePath& pathToFile)
{
    menu->SetEnabled(true);
    scenePath = pathToFile;
    data.settings.SetLastOpenedScenePath(scenePath);
    ReloadScene();
}

void ViewSceneScreen::OnFileSytemDialogCanceled(DAVA::UIFileSystemDialog* forDialog)
{
    menu->SetEnabled(true);
}

void ViewSceneScreen::OnButtonSelectFromRes(DAVA::BaseObject* caller, void* param, void* callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~res:/");
    fileSystemDialog->Show(this);
    menu->SetEnabled(false);
}

void ViewSceneScreen::OnButtonSelectFromDoc(DAVA::BaseObject* caller, void* param, void* callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~doc:/");
    fileSystemDialog->Show(this);
    menu->SetEnabled(false);
}

void ViewSceneScreen::OnButtonSelectFromExt(DAVA::BaseObject* caller, void* param, void* callerData)
{
    using namespace DAVA;
    DVASSERT(fileSystemDialog);

    List<DeviceInfo::StorageInfo> storageList = DeviceInfo::GetStoragesList();
    for (const DeviceInfo::StorageInfo& storage : storageList)
    {
        if (storage.type == DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL ||
            storage.type == DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL)
        {
            menu->SetEnabled(false);
            fileSystemDialog->SetCurrentDir(storage.path);
            fileSystemDialog->Show(this);
            return;
        }
    }
}

void ViewSceneScreen::OnButtonPerformanceTest(DAVA::BaseObject* caller, void* param, void* callerData)
{
#ifdef WITH_SCENE_PERFORMANCE_TESTS
    if (scene && gridTest.GetState() == GridTest::StateFinished && gridTest.Start(sceneView) == true)
    {
        menu->Show(false);
        RemoveControl(moveJoyPAD);
    }
#endif
}

void ViewSceneScreen::OnButtonQualitySettings(DAVA::BaseObject* caller, void* param, void* callerData)
{
    menu->SetEnabled(false);
    qualitySettingsDialog->SetCurrentScene(scene);
    qualitySettingsDialog->Show();
}

void ViewSceneScreen::OnQualitySettingsEditDone()
{
    menu->SetEnabled(true);
}

void ViewSceneScreen::OnButtonReloadShaders(DAVA::BaseObject* caller, void* param, void* callerData)
{
    using namespace DAVA;

    if (scene)
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
}

void ViewSceneScreen::Draw(const DAVA::UIGeometricData& geometricData)
{
    //DAVA::uint64 startTime = DAVA::SystemTimer::Instance()->GetNs();

    BaseScreen::Draw(geometricData);

    //drawTime += (SystemTimer::Instance()->GetNs() - startTime);
}

void ViewSceneScreen::ReloadScene()
{
    RemoveSceneFromScreen();
    LoadScene();
    PlaceSceneAtScreen();
}

void ViewSceneScreen::ProcessUserInput(DAVA::float32 timeElapsed)
{
    if (scene)
    {
        using namespace DAVA;

        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
        //     if (keyboard.IsKeyPressed(Key::NUMPAD6))
        //         cursorPosition.x += timeElapsed / 16.f;
        //     if (keyboard.IsKeyPressed(Key::NUMPAD4))
        //         cursorPosition.x -= timeElapsed / 16.f;
        //     if (keyboard.IsKeyPressed(Key::NUMPAD8))
        //         cursorPosition.y += timeElapsed / 16.f;
        //     if (keyboard.IsKeyPressed(Key::NUMPAD2))
        //         cursorPosition.y -= timeElapsed / 16.f;
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
}

void ViewSceneScreen::Update(DAVA::float32 timeElapsed)
{
    BaseScreen::Update(timeElapsed);

    UpdateInfo(timeElapsed);
    ProcessUserInput(timeElapsed);
}

void ViewSceneScreen::UpdateInfo(DAVA::float32 timeElapsed)
{
    fpsMeter.Update(timeElapsed);
    if (fpsMeter.IsFpsReady())
    {
        infoText->SetText(DAVA::Format(L"FPS: %.0f", fpsMeter.GetFps()));
        //      drawTime = updateTime = 0;
    }
}

#ifdef WITH_SCENE_PERFORMANCE_TESTS
void ViewSceneScreen::OnGridTestStateChanged()
{
    if (gridTest.GetState() == GridTest::StateFinished)
    {
        data.gridTestResult = gridTest.GetResult();
        SetNextScreen();
    }
}
#endif

// void ViewSceneScreen::DidAppear()
// {
//     framesTime = 0.f;
//     frameCounter = 0;
//
//     drawTime = updateTime = 0;
//
//     info->SetText(L"");
// }

void ViewSceneScreen::Input(DAVA::UIEvent* currentInput)
{
    //     if (currentInput->phase == UIEvent::Phase::CHAR)
    //     {
    //         if (currentInput->keyChar == '+')
    //             cursorSize *= 1.25f;
    //         if (currentInput->keyChar == '-')
    //             cursorSize *= .8f;
    //     }

    BaseScreen::Input(currentInput);
}
