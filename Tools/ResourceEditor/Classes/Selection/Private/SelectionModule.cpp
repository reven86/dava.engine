#include "Classes/Selection/SelectionModule.h"
#include "Classes/Selection/SelectionData.h"

#include "Classes/SceneManager/SceneData.h"

#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"
#include "Classes/Qt/Scene/System/HoodSystem.h"
#include "Classes/Qt/Scene/System/WayEditSystem.h"

#include "Logger/Logger.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

void SelectionModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<SelectionData> selectionData = std::make_unique<SelectionData>();
    selectionData->selectionSystem.reset(new SelectionSystem(scene));
    scene->AddSystem(selectionData->selectionSystem.get(), 0, DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS | DAVA::Scene::SCENE_SYSTEM_REQUIRE_INPUT, scene->renderUpdateSystem);

    //TODO: Workaround to save old process
    selectionData->selectionSystem->AddDelegate(scene->modifSystem);
    selectionData->selectionSystem->AddDelegate(scene->hoodSystem);
    selectionData->selectionSystem->AddDelegate(scene->wayEditSystem);
    selectionData->selectionSystem->EnableSystem();
    //END of TODO

    context->CreateData(std::move(selectionData));
}

void SelectionModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    SelectionData* selectionData = context->GetData<SelectionData>();
    scene->RemoveSystem(selectionData->selectionSystem.get());
}

void SelectionModule::PostInit()
{
}
