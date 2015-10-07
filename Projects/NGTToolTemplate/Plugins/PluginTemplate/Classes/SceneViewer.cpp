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

#include "SceneViewer.h"
#include "CollisionSystem.h"
#include "CameraSystem.h"
#include "SelectionSystem.h"

#include "Debug/DVAssert.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

#include "SceneUtils.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenManager.h"

#include "QtTools/FrameworkBinding/FrameworkLoop.h"
#include "GLWidget.h"

namespace
{

int NUMBER_OF_SCREEN = 0;


}

class PluginScene : public DAVA::Scene
{
public:
    void Draw() override
    {
        Scene::Draw();

        findSystem<SceneSelectionSystem>(this)->Draw();
    }
};

SceneViewer::SceneViewer()
{
    glWidget.reset(new PluginGLWidget());
    DavaGLWidget * davaGl = glWidget.get();
    DVVERIFY(QObject::connect(davaGl, &DavaGLWidget::Initialized, this, &SceneViewer::OnGlInitialized, Qt::QueuedConnection));
    DVVERIFY(QObject::connect(davaGl, &DavaGLWidget::Resized, this, &SceneViewer::OnGlResized));
    
    FrameworkLoop::Instance()->SetOpenGLWindow(glWidget.get());
}

SceneViewer::~SceneViewer()
{
    DVASSERT(uiView == nullptr);
    glWidget.reset();
}

void SceneViewer::Finalise()
{
    SafeRelease(uiView);
}

IView & SceneViewer::GetView()
{
    DVASSERT(glWidget != nullptr);
    return *glWidget;
}

void SceneViewer::OnOpenScene(std::string const & scenePath)
{
    using namespace DAVA;

    DVASSERT(!scenePath.empty());
    DVASSERT(nullptr != uiView);

    SetSelection(nullptr);

    ScopedPtr<PluginScene> scene(new PluginScene());
    SceneFileV2::eError result = scene->LoadScene(FilePath(scenePath));
    if (result == DAVA::SceneFileV2::ERROR_NO_ERROR)
    {
        WASDControllerSystem * wasdSystem = new WASDControllerSystem(scene);
        wasdSystem->SetMoveSpeed(10.0f);
        uint64 wasdComponentFlag = MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT);
        uint32 wasdProcessFlag = Scene::SCENE_SYSTEM_REQUIRE_PROCESS;
        scene->AddSystem(wasdSystem, wasdComponentFlag, wasdProcessFlag);

        RotationControllerSystem * rotationSystem = new RotationControllerSystem(scene);
        uint64 rotationComponentFlag = MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT);
        uint32 rotationProcessFlag = Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT;
        scene->AddSystem(rotationSystem, rotationComponentFlag, rotationProcessFlag);

        SceneCameraSystem * cameraSystem = new SceneCameraSystem(scene);
        cameraSystem->SetViewportRect(uiView->GetRect());
        scene->AddSystem(cameraSystem, MAKE_COMPONENT_MASK(DAVA::Component::CAMERA_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT, scene->renderUpdateSystem);

        SceneCollisionSystem * collisionSystem = new SceneCollisionSystem(scene);
        uint32_t collisionProcessFlag = Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT;
        scene->AddSystem(collisionSystem, 0, collisionProcessFlag);

        SceneSelectionSystem * selectionSystem = new SceneSelectionSystem(scene, collisionSystem);
        scene->AddSystem(selectionSystem, 0, Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT, scene->renderUpdateSystem);

        uiView->SetScene(scene);

        emit SceneLoaded(scene);
        //statusBar()->showMessage(scenePath);
    }
}

void SceneViewer::SetSelection(DAVA::Entity* entity)
{
    DVASSERT(uiView != nullptr);
    DAVA::Scene* scene = uiView->GetScene();
    if (scene)
    {
        SceneSelectionSystem* selectionSystem = findSystem<SceneSelectionSystem>(scene);
        if (entity)
            selectionSystem->SetSelection(entity);
        else
            selectionSystem->Clear();
    }
}

void SceneViewer::OnGlInitialized()
{
    using namespace DAVA;
    
    ScopedPtr<UIScreen> screen(new UIScreen());
    DVASSERT(nullptr == uiView);
    uiView = new DAVA::UI3DView(screen->GetRect(), true);
    uiView->SetInputEnabled(true, true);
    
    screen->AddControl(uiView);
    screen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    screen->GetBackground()->SetColor(DAVA::Color(1.0f, 0.65f, 0.65f, 1.f));
    screen->GetBackground()->SetDrawColor(DAVA::Color(0.65f, 0.65f, 0.65f, 1.f));
    
    UIScreenManager::Instance()->RegisterScreen(NUMBER_OF_SCREEN, screen);
    UIScreenManager::Instance()->SetFirst(NUMBER_OF_SCREEN);
}

void SceneViewer::OnGlResized(int width, int height, int dpr)
{
    using namespace DAVA;
    UIScreen * screen = UIScreenManager::Instance()->GetScreen(NUMBER_OF_SCREEN);
    if (screen == nullptr)
    {
        return;
    }
    
    qint64 scaleWidth = width * dpr;
    qint64 scaleHeight = height * dpr;
    screen->SetSize(Vector2(scaleWidth, scaleHeight));
    uiView->SetSize(Vector2(scaleWidth, scaleHeight));
    if (uiView->GetScene() != nullptr)
        findSystem<SceneCameraSystem>(uiView->GetScene())->SetViewportRect(uiView->GetRect());
}

