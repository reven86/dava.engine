#include <QApplication>
#include <QDebug>

#include "Scene/SceneEditor2.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Classes/Settings/SettingsManager.h"

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Render/RenderHelper.h"

#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"

#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/AddComponentCommand.h"

#include "../StringConstants.h"

#include "../../Main/QtUtils.h"
#include "Classes/Settings/SettingsManager.h"

#include "Classes/Selection/Selection.h"

#include <QDebug>

namespace
{
const auto wheelAdjust = 0.002;
}

SceneCameraSystem::SceneCameraSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

SceneCameraSystem::~SceneCameraSystem()
{
    SafeRelease(curSceneCamera);
}

DAVA::Camera* SceneCameraSystem::GetCurCamera() const
{
    return curSceneCamera;
}

DAVA::Vector3 SceneCameraSystem::GetPointDirection(const DAVA::Vector2& point) const
{
    DAVA::Vector3 dir;

    if (nullptr != curSceneCamera)
    {
        DAVA::Vector3 pos = curSceneCamera->GetPosition();
        dir = curSceneCamera->UnProject(point.x, point.y, 0, viewportRect);
        dir -= pos;
    }

    return dir;
}

DAVA::Vector3 SceneCameraSystem::GetCameraPosition() const
{
    DAVA::Vector3 pos;

    if (nullptr != curSceneCamera)
    {
        pos = curSceneCamera->GetPosition();
    }

    return pos;
}

DAVA::Vector3 SceneCameraSystem::GetCameraDirection() const
{
    DAVA::Vector3 dir;

    if (nullptr != curSceneCamera)
    {
        dir = curSceneCamera->GetDirection();
    }

    return dir;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
    DAVA::float32 speed = 1.0;

    switch (activeSpeedIndex)
    {
    case 0:
        speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed0).AsFloat();
        break;
    case 1:
        speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed1).AsFloat();
        break;
    case 2:
        speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed2).AsFloat();
        break;
    case 3:
        speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed3).AsFloat();
        break;
    }

    return speed;
}

DAVA::uint32 SceneCameraSystem::GetActiveSpeedIndex()
{
    return activeSpeedIndex;
}

void SceneCameraSystem::SetMoveSpeedArrayIndex(DAVA::uint32 index)
{
    DVASSERT(index < 4u);
    activeSpeedIndex = index;
}

void SceneCameraSystem::SetViewportRect(const DAVA::Rect& rect)
{
    viewportRect = rect;

    RecalcCameraAspect();
}

const DAVA::Rect& SceneCameraSystem::GetViewportRect() const
{
    return viewportRect;
}

DAVA::Vector2 SceneCameraSystem::GetScreenPos(const DAVA::Vector3& pos3) const
{
    DAVA::Vector3 ret3d = GetScreenPosAndDepth(pos3);
    return DAVA::Vector2(ret3d.x, ret3d.y);
}

DAVA::Vector3 SceneCameraSystem::GetScreenPosAndDepth(const DAVA::Vector3& pos3) const
{
    DAVA::Vector3 ret;

    if (nullptr != curSceneCamera)
    {
        ret = curSceneCamera->GetOnScreenPositionAndDepth(pos3, viewportRect);
    }

    return ret;
}

DAVA::Vector3 SceneCameraSystem::GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const
{
    DAVA::Vector3 ret;

    if (nullptr != curSceneCamera)
    {
        ret = curSceneCamera->UnProject(x, y, z, viewportRect);
    }

    return ret;
}

void SceneCameraSystem::LookAt(const DAVA::AABBox3& box)
{
    if (nullptr != curSceneCamera && !box.IsEmpty())
    {
        DAVA::Vector3 pos = curSceneCamera->GetPosition();
        DAVA::Vector3 targ = curSceneCamera->GetTarget();
        DAVA::Vector3 dir = targ - pos;
        dir.Normalize();

        DAVA::float32 boxSize = ((box.max - box.min).Length());
        const DAVA::Vector3 c = box.GetCenter();

        pos = c - (dir * (boxSize + curSceneCamera->GetZNear() * 1.5f));
        targ = c;

        MoveTo(pos, targ);
    }
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3& pos)
{
    if (nullptr != curSceneCamera)
    {
        MoveTo(pos, curSceneCamera->GetTarget());
    }
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3& pos, const DAVA::Vector3& target)
{
    if (nullptr != curSceneCamera && !curSceneCamera->GetIsOrtho())
    {
        animateToNewPos = true;
        animateToNewPosTime = 0;

        newPos = pos;
        newTar = target;
    }
}

void SceneCameraSystem::Process(float timeElapsed)
{
    //TODO: set move speed
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    DAVA::WASDControllerSystem* wasdSystem = scene->wasdSystem;
    if (wasdSystem)
    {
        wasdSystem->SetMoveSpeed((animateToNewPos) ? 0 : GetMoveSpeed());
    }
    DAVA::RotationControllerSystem* rotationSystem = scene->rotationSystem;
    if (rotationSystem)
    {
        rotationSystem->SetRotationSpeeed((animateToNewPos) ? 0 : 0.15f);

        HoodSystem* hoodSystem = scene->hoodSystem;
        if (nullptr != hoodSystem)
        {
            rotationSystem->SetRotationPoint(hoodSystem->GetPosition());
        }
    }
    //TODO: set move speed

    if (nullptr != scene)
    {
        DAVA::Camera* camera = scene->GetDrawCamera();

        // is current camera in scene changed?
        if (curSceneCamera != camera)
        {
            // update collision object for last camera
            if (nullptr != curSceneCamera)
            {
                DAVA::Entity* cameraOwner = GetEntityFromCamera(curSceneCamera);
                if (cameraOwner != nullptr)
                {
                    SceneCollisionSystem* collSystem = (static_cast<SceneEditor2*>(GetScene()))->collisionSystem;
                    collSystem->UpdateCollisionObject(Selectable(cameraOwner));
                }
            }

            // remember current scene camera
            SafeRelease(curSceneCamera);
            curSceneCamera = SafeRetain(camera);

            // Recalc camera aspect
            RecalcCameraAspect();
        }
    }

    // camera move animation
    MoveAnimate(timeElapsed);
}

bool SceneCameraSystem::Input(DAVA::UIEvent* event)
{
    switch (event->phase)
    {
    case DAVA::UIEvent::Phase::KEY_DOWN:
    case DAVA::UIEvent::Phase::KEY_DOWN_REPEAT:
        OnKeyboardInput(event);
        break;
    case DAVA::UIEvent::Phase::WHEEL:
        ScrollCamera(event->wheelDelta.y);
        break;
    case DAVA::UIEvent::Phase::GESTURE:
    {
        const DAVA::UIEvent::Gesture& gesture = event->gesture;
        if (gesture.dy != 0.0f)
        {
            ScrollCamera(gesture.dy);
        }
    }
    break;
    default:
        break;
    }
    return false;
}

void SceneCameraSystem::ScrollCamera(DAVA::float32 dy)
{
    bool moveCamera = SettingsManager::GetValue(Settings::General_Mouse_WheelMoveCamera).AsBool();
    if (!moveCamera)
        return;

    DAVA::int32 reverse = SettingsManager::GetValue(Settings::General_Mouse_InvertWheel).AsBool() ? -1 : 1;
    DAVA::float32 moveIntence = SettingsManager::GetValue(Settings::General_Mouse_WheelMoveIntensity).AsFloat();
    int offset = dy * moveIntence;
#ifdef Q_OS_MAC
    offset *= reverse * -1;
#else
    offset *= reverse;
#endif

    MoveToStep(offset);
}

void SceneCameraSystem::OnKeyboardInput(DAVA::UIEvent* event)
{
    const auto isModificatorPressed =
    DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::Key::LCTRL) ||
    DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::Key::LALT) ||
    DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::Key::LSHIFT);
    if (isModificatorPressed)
        return;

    switch (event->key)
    {
    case DAVA::Key::ADD:
    case DAVA::Key::EQUALS:
    {
        auto entity = GetEntityWithEditorCamera();
        auto snapComponent = GetSnapToLandscapeControllerComponent(entity);
        if (snapComponent != nullptr)
        {
            const auto height = snapComponent->GetHeightOnLandscape() + SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscapeStep).AsFloat();
            snapComponent->SetHeightOnLandscape(height);
            SettingsManager::Instance()->SetValue(Settings::Scene_CameraHeightOnLandscape, DAVA::VariantType(height));
        }
    }
    break;
    case DAVA::Key::SUBTRACT:
    case DAVA::Key::MINUS:
    {
        auto entity = GetEntityWithEditorCamera();
        auto snapComponent = GetSnapToLandscapeControllerComponent(entity);
        if (snapComponent != nullptr)
        {
            const auto height = snapComponent->GetHeightOnLandscape() - SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscapeStep).AsFloat();
            snapComponent->SetHeightOnLandscape(height);
            SettingsManager::Instance()->SetValue(Settings::Scene_CameraHeightOnLandscape, DAVA::VariantType(height));
        }
    }
    break;

    case DAVA::Key::KEY_T:
        MoveTo(DAVA::Vector3(0, 0, 200), DAVA::Vector3(1, 0, 0));
        break;

    case DAVA::Key::KEY_1:
        SetMoveSpeedArrayIndex(0);
        break;
    case DAVA::Key::KEY_2:
        SetMoveSpeedArrayIndex(1);
        break;
    case DAVA::Key::KEY_3:
        SetMoveSpeedArrayIndex(2);
        break;
    case DAVA::Key::KEY_4:
        SetMoveSpeedArrayIndex(3);
        break;

    default:
        break;
    }
}

void SceneCameraSystem::Draw()
{
    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    SceneCollisionSystem* collSystem = sceneEditor->collisionSystem;
    if (nullptr != collSystem)
    {
        for (DAVA::Entity* entity : sceneCameras)
        {
            DVASSERT(entity != nullptr);
            DAVA::Camera* camera = GetCamera(entity);
            if (nullptr != camera && camera != curSceneCamera)
            {
                DAVA::AABBox3 worldBox;
                DAVA::AABBox3 collBox = collSystem->GetBoundingBox(entity);
                DVASSERT(!collBox.IsEmpty());

                DAVA::Matrix4 transform;
                transform.Identity();
                transform.SetTranslationVector(camera->GetPosition());
                collBox.GetTransformedBox(transform, worldBox);
                sceneEditor->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, DAVA::Color(0, 1.0f, 0, 1.0f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void SceneCameraSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(GetCamera(entity) != nullptr);
    sceneCameras.push_back(entity);
}

void SceneCameraSystem::RemoveEntity(DAVA::Entity* entity)
{
    FindAndRemoveExchangingWithLast(sceneCameras, entity);
}

void SceneCameraSystem::CreateDebugCameras()
{
    DAVA::Scene* scene = GetScene();

    // add debug cameras
    // there already can be other cameras in scene
    if (nullptr != scene)
    {
        DAVA::ScopedPtr<DAVA::Camera> topCamera(new DAVA::Camera());
        topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
        topCamera->SetPosition(DAVA::Vector3(-50.0f, 0.0f, 50.0f));
        topCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
        DAVA::float32 cameraFov = SettingsManager::GetValue(Settings::Scene_CameraFOV).AsFloat();
        DAVA::float32 cameraNear = SettingsManager::GetValue(Settings::Scene_CameraNear).AsFloat();
        DAVA::float32 cameraFar = SettingsManager::GetValue(Settings::Scene_CameraFar).AsFloat();
        topCamera->SetupPerspective(cameraFov, 320.0f / 480.0f, cameraNear, cameraFar);
        topCamera->SetAspect(1.0f);

        DAVA::ScopedPtr<DAVA::Entity> topCameraEntity(new DAVA::Entity());
        topCameraEntity->SetName(ResourceEditor::EDITOR_DEBUG_CAMERA);
        topCameraEntity->SetNotRemovable(true);
        topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
        topCameraEntity->AddComponent(new DAVA::WASDControllerComponent());
        topCameraEntity->AddComponent(new DAVA::RotationControllerComponent());
        if (scene->GetChildrenCount() > 0)
        {
            scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));
        }
        else
        {
            scene->AddNode(topCameraEntity);
        }

        // set current default camera
        if (nullptr == scene->GetCurrentCamera())
        {
            scene->SetCurrentCamera(topCamera);
        }

        scene->AddCamera(topCamera);
    }
}

void SceneCameraSystem::RecalcCameraAspect()
{
    if (nullptr != curSceneCamera)
    {
        DAVA::float32 aspect = 1.0;

        if (0 != viewportRect.dx && 0 != viewportRect.dy)
        {
            aspect = viewportRect.dx / viewportRect.dy;
        }

        curSceneCamera->SetAspect(aspect);
    }
}

void SceneCameraSystem::MoveAnimate(DAVA::float32 timeElapsed)
{
    static const DAVA::float32 animationTime = 3.0f;
    static const DAVA::float32 animationStopDistance = 1.0f;

    if (nullptr != curSceneCamera && animateToNewPos)
    {
        DAVA::Vector3 pos = curSceneCamera->GetPosition();
        DAVA::Vector3 tar = curSceneCamera->GetTarget();
        const DAVA::float32 animationDistance = (pos - newPos).Length();

        if ((pos != newPos || tar != newTar) && (animateToNewPosTime < animationTime) && (animationDistance > animationStopDistance))
        {
            animateToNewPosTime += timeElapsed;

            DAVA::float32 fnX = animateToNewPosTime / animationTime;
            DAVA::float32 fnY = sin(1.57 * fnX);

            DAVA::Vector3 dPos = newPos - pos;
            DAVA::Vector3 dTar = newTar - tar;

            if (dPos.Length() > 0.01f)
                dPos = dPos * fnY;
            if (dTar.Length() > 0.01f)
                dTar = dTar * fnY;

            curSceneCamera->SetPosition(pos + dPos);
            curSceneCamera->SetTarget(tar + dTar);
        }
        else
        {
            animateToNewPos = false;
            animateToNewPosTime = 0;

            curSceneCamera->SetTarget(newTar);
            curSceneCamera->SetPosition(newPos);

            SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
            sc->rotationSystem->RecalcCameraViewAngles(curSceneCamera);
        }

        UpdateDistanceToCamera();
    }
}

void SceneCameraSystem::UpdateDistanceToCamera()
{
    distanceToCamera = 0.f;

    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    const DAVA::Camera* cam = sc->GetCurrentCamera();
    if (cam)
    {
        const SelectableGroup& selection = Selection::GetSelection();
        if (!selection.IsEmpty())
        {
            DAVA::AABBox3 bbox = selection.GetTransformedBoundingBox();
            if (!bbox.IsEmpty())
            {
                distanceToCamera = ((cam->GetPosition() - bbox.GetCenter()).Length()) * cam->GetZoomFactor();
            }
        }
    }
}

DAVA::float32 SceneCameraSystem::GetDistanceToCamera() const
{
    return distanceToCamera;
}

DAVA::Entity* SceneCameraSystem::GetEntityFromCamera(DAVA::Camera* c) const
{
    for (auto& entity : sceneCameras)
    {
        if (GetCamera(entity) == c)
        {
            return entity;
        }
    }

    return nullptr;
}

void SceneCameraSystem::GetRayTo2dPoint(const DAVA::Vector2& point, DAVA::float32 maxRayLen, DAVA::Vector3& outPointFrom, DAVA::Vector3& outPointTo) const
{
    if (nullptr != curSceneCamera)
    {
        DAVA::Vector3 camPos = GetCameraPosition();
        DAVA::Vector3 camDir = GetPointDirection(point);

        if (curSceneCamera->GetIsOrtho())
        {
            outPointFrom = DAVA::Vector3(camDir.x, camDir.y, camPos.z);
            outPointTo = DAVA::Vector3(camDir.x, camDir.y, camPos.z + maxRayLen);
        }
        else
        {
            outPointFrom = camPos;
            outPointTo = outPointFrom + camDir * maxRayLen;
        }
    }
}

DAVA::Entity* SceneCameraSystem::GetEntityWithEditorCamera() const
{
    DAVA::int32 cameraCount = GetScene()->GetCameraCount();
    for (DAVA::int32 i = 0; i < cameraCount; ++i)
    {
        DAVA::Camera* c = GetScene()->GetCamera(i);
        DAVA::Entity* e = GetEntityFromCamera(c);
        if (e && e->GetName() == ResourceEditor::EDITOR_DEBUG_CAMERA)
        {
            return e;
        }
    }

    return nullptr;
}

bool SceneCameraSystem::SnapEditorCameraToLandscape(bool snap)
{
    DAVA::Entity* entity = GetEntityWithEditorCamera();
    if (!entity)
        return false;

    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    DAVA::SnapToLandscapeControllerComponent* snapComponent = GetSnapToLandscapeControllerComponent(entity);
    if (snap)
    {
        if (!snapComponent)
        {
            DAVA::float32 height = SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscape).AsFloat();

            snapComponent = static_cast<DAVA::SnapToLandscapeControllerComponent*>(DAVA::Component::CreateByType(DAVA::Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT));
            snapComponent->SetHeightOnLandscape(height);

            scene->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(entity, snapComponent)));
        }
    }
    else if (snapComponent)
    {
        scene->Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(entity, snapComponent)));
    }

    return true;
}

bool SceneCameraSystem::IsEditorCameraSnappedToLandscape() const
{
    DAVA::Entity* entity = GetEntityWithEditorCamera();
    return (GetSnapToLandscapeControllerComponent(entity) != nullptr);
}

void SceneCameraSystem::MoveToSelection()
{
    auto sceneEditor = dynamic_cast<SceneEditor2*>(GetScene());
    if (sceneEditor == nullptr)
        return;

    const SelectableGroup& selection = Selection::GetSelection();
    if (!selection.IsEmpty())
    {
        DAVA::AABBox3 bbox = selection.GetTransformedBoundingBox();
        if (!bbox.IsEmpty())
        {
            LookAt(bbox);
        }
    }
}

void SceneCameraSystem::MoveToStep(int ofs)
{
    const auto pos = GetCameraPosition();
    const auto direction = GetCameraDirection();
    const auto delta = direction * GetMoveSpeed() * ofs * wheelAdjust;
    const auto dest = pos + delta;
    const auto target = dest + direction;

    MoveTo(dest, target);
}

void SceneCameraSystem::EnableSystem()
{
    EditorSceneSystem::EnableSystem();
    CreateDebugCameras();
}
