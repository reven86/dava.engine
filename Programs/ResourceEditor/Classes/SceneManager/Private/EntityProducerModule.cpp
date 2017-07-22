#include "Classes/SceneManager/EntityProducerModule.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/Utils/ModuleCollection.h"

#include "Functional/Function.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Entity.h"

void EntityProducerModule::PostInit()
{
    using namespace DAVA::TArc;

    QtAction* instantiateCamera = new QtAction(GetAccessor(), "Clone Current Camera");
    FieldDescriptor fieldDescriptor;
    fieldDescriptor.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    fieldDescriptor.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    instantiateCamera->SetStateUpdationFunction(QtAction::Enabled, fieldDescriptor, [](const DAVA::Any& value) -> DAVA::Any
                                                {
                                                    return value.CanCast<SceneData::TSceneType>();
                                                });

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(QList<QString>() << "menuCreateNode"
                                                                     << "menuAdd"));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, instantiateCamera);
    connections.AddConnection(instantiateCamera, &QAction::triggered, DAVA::MakeFunction(this, &EntityProducerModule::InstantiateCurrentCamera));
}

void EntityProducerModule::InstantiateCurrentCamera()
{
    using namespace DAVA::TArc;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    DAVA::RefPtr<SceneEditor2> sceneEditor = sceneData->GetScene();

    DAVA::Camera* currentCamera = sceneEditor->GetCurrentCamera();
    DVASSERT(currentCamera != nullptr);

    DAVA::ScopedPtr<DAVA::Camera> camera(static_cast<DAVA::Camera*>(currentCamera->Clone()));

    DAVA::ScopedPtr<DAVA::Entity> sceneNode(new DAVA::Entity());
    sceneNode->AddComponent(new DAVA::CameraComponent(camera));
    sceneNode->AddComponent(new DAVA::WASDControllerComponent());
    sceneNode->AddComponent(new DAVA::RotationControllerComponent());
    sceneNode->SetName(ResourceEditor::CAMERA_NODE_NAME);

    sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(sceneNode, sceneEditor.Get())));
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityProducerModule)
{
    DAVA::ReflectionRegistrator<EntityProducerModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(EntityProducerModule);