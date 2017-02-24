#include "ViewSceneScreen.h"

#include "GameCore.h"
#include "OverdrawTesterComponent.h"
#include "OverdrawTesterSystem.h"
#include "ChartPainterSystem.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"

using OverdrawPerformanceTester::OverdrawTesterComonent;
using OverdrawPerformanceTester::OverdrawTesterSystem;
using OverdrawPerformanceTester::ChartPainterSystem;
using OverdrawPerformanceTester::FrameData;
using DAVA::Scene;
using DAVA::Camera;
using DAVA::float32;
using DAVA::Vector3;

void ViewSceneScreen::LoadResources()
{
    if (!loaded)
    {
        BaseScreen::LoadResources();

        scene = new Scene();
        scene->LoadScene(GameCore::Instance()->GetScenePath());

        testerSystem = new OverdrawTesterSystem(scene, 
            [this](DAVA::Array<DAVA::Vector<FrameData>, 6>* performanceData)
            {
                chartPainterSystem->SetPerformanceData(performanceData);
            });

        scene->AddSystem(testerSystem, MAKE_COMPONENT_MASK(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

        chartPainterSystem = new ChartPainterSystem(scene);
        scene->AddSystem(chartPainterSystem, MAKE_COMPONENT_MASK(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

        ScopedPtr<Camera> camera(new Camera());

        VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;
        float32 aspect = (float32)vcs->GetVirtualScreenSize().dy / (float32)vcs->GetVirtualScreenSize().dx;
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
        overdrawTesterEntity->AddComponent(new OverdrawPerformanceTester::OverdrawTesterComonent());
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
    }
}

void ViewSceneScreen::UnloadResources()
{
    scene->RemoveSystem(chartPainterSystem);
    SafeDelete(chartPainterSystem);

    scene->RemoveSystem(testerSystem);
    SafeDelete(testerSystem);

    SafeRelease(scene);
    BaseScreen::UnloadResources();
}
