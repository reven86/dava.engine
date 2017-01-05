#include "ViewSceneScreen.h"
#include "GameCore.h"

#include "Math/MathHelpers.h"

using namespace DAVA;

void ViewSceneScreen::LoadResources()
{
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

        VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
        vcs->RegisterAvailableResourceSize(vcs->GetVirtualScreenSize().dx, vcs->GetVirtualScreenSize().dy, "Gfx");
        float32 aspect = (float32)vcs->GetVirtualScreenSize().dy / (float32)vcs->GetVirtualScreenSize().dx;
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

        fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/korinna.ttf");
        fileSystemDialog->SetDelegate(this);
        fileSystemDialog->SetExtensionFilter(".sc2");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);

        info.reset(new UIStaticText(Rect(screenRect.dy / 2, 30, 100, 30)));
        info->SetFont(font);
        info->SetTextColor(Color::White);
        info->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
        AddControl(info);

        //         dbg = new UIStaticText(Rect(screenRect.dy / 2, 130, 400, 30));
        //         dbg->SetFont(font);
        //         dbg->SetTextColor(Color::White);
        //         dbg->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
        //         AddControl(dbg);

        moveJoyPAD = new UIJoypad(Rect(10, screenRect.dy - 210.f, 200.f, 200.f));
        moveJoyPAD->SetDebugDraw(true);
        moveJoyPAD->SetStickSprite("~res:/Gfx/Joypad/joypad.tex", 0);
        AddControl(moveJoyPAD);
    }
}

void ViewSceneScreen::SetCameraAtCenter(Camera* camera)
{
    camera->SetLeft(Vector3(1, 0, 0));
    camera->SetUp(Vector3(0, 0, 1.f));
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetPosition(Vector3(0, -45, 10));
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

const uint32 PT_GRID_SIZE = 4;
const uint32 PT_ANGLE_COUNT = 4;
const float32 PT_EXPOSURE_INTERVAL_SEC = 0.1f;
const float32 PT_YELLOW_THRESHOLD_FPS = 57.0f;

void ViewSceneScreen::OnButtonPerformanceTest(BaseObject* caller, void* param, void* callerData)
{
    if (performanceTestState != PT_State::Finished)
        return;

    Landscape* landscape = FindLandscape(scene);
    if (landscape)
    {
        samples.reserve(PT_GRID_SIZE * PT_GRID_SIZE * PT_ANGLE_COUNT);

        float32 f = landscape->GetLandscapeSize();

        float32 step = f / (PT_GRID_SIZE + 1);
        float32 xMin = -f / 2 + step;
        float32 xMax = f / 2 - step;
        float32 yMin = -f / 2 + step;
        float32 angleStep = 360.f / PT_ANGLE_COUNT;

        bool invertedDirection = false;
        float32 yPos = yMin;
        for (uint32 y = 0; y < PT_GRID_SIZE; ++y, yPos += step)
        {
            float32 xPos = invertedDirection ? xMax : xMin;
            float32 xInc = invertedDirection ? -step : step;
            for (uint32 x = 0; x < PT_GRID_SIZE; ++x, xPos += xInc)
            {
                static float32 angle = 0.1f;
                for (uint32 n = 0; n < PT_ANGLE_COUNT; ++n, angle += angleStep)
                {
                    samples.push_back(Sample());
                    Sample& testPosition = samples.back();

                    testPosition.pos.x = xPos;
                    testPosition.pos.y = yPos;

                    float landscapeHeight = 0.0;
                    landscape->GetHeightAtPoint(testPosition.pos, landscapeHeight);
                    testPosition.pos.z = landscapeHeight + 10.0f;

                    testPosition.angle = angle;
                    SinCosFast(DegToRad(angle), testPosition.sine, testPosition.cos);
                }
            }

            invertedDirection = !invertedDirection;
        }

        performanceTestState = PT_State::Running;
        DateTime now = DateTime::Now();
        reportFolderPath = FilePath(Format("~doc:/PerformanceReports/Report_%u/", now.GetTimestamp()));
        FileSystem::Instance()->CreateDirectory(reportFolderPath, true);
        reportFile = File::Create(reportFolderPath + "report.txt", File::CREATE | File::WRITE);

        screenshotsToStart.clear();
        screenshotsToSave.clear();

        sampleIndex = 0;
        SetSamplePosition(samples[sampleIndex]);
    }
}

void ViewSceneScreen::SetSamplePosition(Sample& sample)
{
    Camera* camera = scene->GetCurrentCamera();
    camera->SetPosition(Vector3(sample.pos.x, sample.pos.y, sample.pos.z));
    camera->SetDirection(Vector3(sample.cos, sample.sine, 0));
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

void ViewSceneScreen::MakeScreenshot(ScreenshotSaver& screenshotSaver)
{
    SetSamplePosition(screenshotSaver.sample);
    UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(sceneView, FORMAT_RGBA8888, MakeFunction(&screenshotSaver, &ScreenshotSaver::SaveTexture));
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
    UpdatePerformanceTest(timeElapsed);
    ProcessUserInput(timeElapsed);
}

static const float32 INFO_UPDATE_INTERVAL_SEC = 1.0f;

void ViewSceneScreen::UpdateInfo(float32 timeElapsed)
{
    ++frameCounter;
    framesTime += timeElapsed;

    if (framesTime > INFO_UPDATE_INTERVAL_SEC)
    {
        float32 fps = frameCounter / framesTime;
        info->SetText(Format(L"FPS: %.0f", fps));

        //         Camera* camera = scene->GetCurrentCamera();
        //         auto v = camera->GetPosition();
        //         auto d = camera->GetDirection();
        //         dbg->SetText(Format(L"pos: %2.1f,%2.1f,%2.1f d: %2.1f,%2.1f,%2.1f", v.x, v.y, v.z, d.x, d.y, d.z));

        framesTime = 0;
        frameCounter = 0;

        drawTime = updateTime = 0;
    }
}

void ViewSceneScreen::UpdatePerformanceTest(float32 timeElapsed)
{
    switch (performanceTestState)
    {
    case PT_State::Running:
    {
        ++frameCounterPT;
        framesTimePT += timeElapsed;

        if (framesTimePT > PT_EXPOSURE_INTERVAL_SEC)
        {
            Sample& sample = samples[sampleIndex];
            sample.fps = frameCounterPT / framesTimePT;

            framesTimePT = 0;
            frameCounterPT = 0;

            if (++sampleIndex < samples.size())
            {
                SetSamplePosition(samples[sampleIndex]);
            }
            else
            {
                float32 avgFps = 0.0;
                float32 minFps = 60.0;
                float32 maxFps = 0.0;
                for (uint32 sampleIndex = 0; sampleIndex < samples.size(); ++sampleIndex)
                {
                    Sample& sample = samples[sampleIndex];

                    reportFile->WriteLine(Format("Sample %.0f.%.0f.%.0f angle %.0f-%.0f: fps %.1f", sample.pos.x, sample.pos.y, sample.pos.z, sample.cos, sample.sine, sample.fps));

                    avgFps += sample.fps;

                    if (sample.fps < minFps)
                        minFps = sample.fps;

                    if (sample.fps > maxFps)
                        maxFps = sample.fps;

                    if (sample.fps < PT_YELLOW_THRESHOLD_FPS)
                    {
                        String screenshotName = Format("shot%u_fps%2.0f.png", sampleIndex, sample.fps);
                        FilePath path = reportFolderPath + screenshotName;
                        screenshotsToStart.emplace_back(ScreenshotSaver(path, sample));
                    }
                }
                avgFps /= samples.size();
                String total = Format("Avg fps: %.1f, min %.1f, max %.1f", avgFps, minFps, maxFps);
                Logger::Info("%s", total.c_str());
                reportFile->WriteLine(total);
                reportFile.reset();

                performanceTestState = PT_State::MakingScreenshots;
            }
        }
        return;
    }
    case PT_State::MakingScreenshots:
    {
        isEvenFrame = !isEvenFrame;
        if (!screenshotsToStart.empty())
        {
            if (isEvenFrame) // hack: making screenshots only on even frames (to avoid bugs in screenshot maker)
            {
                // move first element from screenshotsToStart to the end of screenshotsToSave
                screenshotsToSave.splice(screenshotsToSave.end(), screenshotsToStart, screenshotsToStart.begin());

                MakeScreenshot(screenshotsToSave.back());
            }
        }
        else
        {
            auto& it = screenshotsToSave.begin();
            while (it != screenshotsToSave.end())
            {
                ScreenshotSaver& screenshotSaver = *it;
                if (screenshotSaver.saved)
                {
                    auto itDel = it++;
                    screenshotsToSave.erase(itDel);
                }
                else
                {
                    ++it;
                }
            }

            if (screenshotsToSave.empty())
            {
                performanceTestState = PT_State::Finished;
                SetCameraAtCenter(scene->GetCurrentCamera());
            }
        }

        return;
    }
    case PT_State::Finished:
    default:
    {
        return;
    }
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
