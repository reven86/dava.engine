#include "OverdrawTestingScreen.h"

#include "OverdrawTestConfig.h"
#include "OverdrawTesterComponent.h"
#include "OverdrawTesterSystem.h"
#include "ChartPainterSystem.h"
#include "Infrastructure/TestBed.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"
#include "UI/UIControlSystem.h"
#include "UI/UIButton.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "Math/Color.h"
#include "Base/Message.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/FTFont.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Render/Highlevel/Camera.h"

using OverdrawPerformanceTester::OverdrawTesterComonent;
using OverdrawPerformanceTester::OverdrawTesterSystem;
using OverdrawPerformanceTester::ChartPainterSystem;
using OverdrawPerformanceTester::FrameData;
using DAVA::Entity;
using DAVA::Scene;
using DAVA::Camera;
using DAVA::float32;
using DAVA::Color;
using DAVA::Vector3;
using DAVA::UI3DView;
using DAVA::UIControlSystem;
using DAVA::VirtualCoordinatesSystem;
using DAVA::CameraComponent;
using DAVA::Size2i;
using DAVA::Rect;
using DAVA::ScopedPtr;
using DAVA::FilePath;
using DAVA::Rect;
using DAVA::UIScreen;
using DAVA::UIAnchorComponent;
using DAVA::UIButton;
using DAVA::Message;
using DAVA::FTFont;

OverdrawTestingScreen::OverdrawTestingScreen(TestBed& app_) : app(app_)
{
}

void OverdrawTestingScreen::LoadResources()
{

    scene = new Scene();
    scene->LoadScene(FilePath("~res:3d/Maps/overdraw_test/TestingScene.sc2"));

    testerSystem = new OverdrawTesterSystem(scene, OverdrawTestConfig::pixelFormat, OverdrawTestConfig::textureResolution,
                                            [this](DAVA::Array<DAVA::Vector<FrameData>, 6>* performanceData)
                                            {
                                                chartPainterSystem->ProcessPerformanceData(performanceData);
                                            });

    scene->AddSystem(testerSystem, MAKE_COMPONENT_MASK(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    chartPainterSystem = new ChartPainterSystem(scene, OverdrawTestConfig::chartHeight);
    scene->AddSystem(chartPainterSystem, MAKE_COMPONENT_MASK(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    ScopedPtr<Camera> camera(new Camera());

    VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
    initialVcsSize = vcs->GetVirtualScreenSize();
    vcs->SetVirtualScreenSize(DAVA::Renderer::GetFramebufferWidth(), DAVA::Renderer::GetFramebufferHeight());
    
    float32 aspect = static_cast<float32>(vcs->GetVirtualScreenSize().dy) / static_cast<float32>(vcs->GetVirtualScreenSize().dx);
    camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
    camera->SetLeft(Vector3(1, 0, 0));
    camera->SetUp(Vector3(0, 0, 1.f));
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetPosition(Vector3(0, -45, 10));

    scene->AddCamera(camera);
    scene->SetCurrentCamera(camera);

    Entity* cameraEntity = new Entity();
    cameraEntity->AddComponent(new CameraComponent(camera));
    scene->AddNode(cameraEntity);
    cameraEntity->Release();

    Entity* overdrawTesterEntity = new Entity();
    overdrawTesterEntity->AddComponent(new OverdrawPerformanceTester::OverdrawTesterComonent(OverdrawTestConfig::textureResolution, OverdrawTestConfig::overdrawScreensCount));
    scene->AddNode(overdrawTesterEntity);
    overdrawTesterEntity->Release();

    Rect screenRect = GetRect();
    Size2i screenSize = UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
    screenRect.dx = static_cast<float32>(screenSize.dx);
    screenRect.dy = static_cast<float32>(screenSize.dy);
    SetRect(screenRect);
    ScopedPtr<UI3DView> sceneView(new UI3DView(screenRect));
    sceneView->SetScene(scene);
    AddControl(sceneView);

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    exitButton = new UIButton(Rect(static_cast<float32>(screenSize.dx - 300), static_cast<float32>(screenSize.dy - 30), 300.0, 30.0));
    exitButton->SetStateFont(0xFF, font);
    exitButton->SetStateFontColor(0xFF, Color::White);
    exitButton->SetStateText(0xFF, L"Exit From Screen");

    exitButton->SetDebugDraw(true);
    exitButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &OverdrawTestingScreen::OnExitButton));

    {
        // Stick button to bottom right corner
        UIAnchorComponent* anchor = exitButton->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetRightAnchorEnabled(true);
    }

    AddControl(exitButton);
}

void OverdrawTestingScreen::UnloadResources()
{
    RemoveAllControls();
    UIScreen::UnloadResources();

    scene->RemoveSystem(chartPainterSystem);
    SafeDelete(chartPainterSystem);

    scene->RemoveSystem(testerSystem);
    SafeDelete(testerSystem);

    DAVA::UIControlSystem::Instance()->vcs->SetVirtualScreenSize(initialVcsSize.dx, initialVcsSize.dy);
    SafeRelease(exitButton);
    SafeRelease(scene);

}

void OverdrawTestingScreen::OnExitButton(DAVA::BaseObject* obj, void* data, void* callerData)
{
    app.ShowStartScreen();
}
