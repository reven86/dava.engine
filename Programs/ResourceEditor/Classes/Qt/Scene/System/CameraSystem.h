#pragma once

#include "Entity/SceneSystem.h"
#include "Render/Highlevel/Camera.h"
#include "UI/UIEvent.h"

#include "Base/Introspection.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

class SceneCameraSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;
    friend class EditorLightSystem;

public:
    SceneCameraSystem(DAVA::Scene* scene);
    ~SceneCameraSystem();

    DAVA::Camera* GetCurCamera() const;

    DAVA::Vector3 GetPointDirection(const DAVA::Vector2& point) const;
    DAVA::Vector3 GetCameraPosition() const;
    DAVA::Vector3 GetCameraDirection() const;
    void GetRayTo2dPoint(const DAVA::Vector2& point, DAVA::float32 maxRayLen, DAVA::Vector3& outPointFrom, DAVA::Vector3& outPointTo) const;

    DAVA::float32 GetMoveSpeed();

    DAVA::uint32 GetActiveSpeedIndex();
    void SetMoveSpeedArrayIndex(DAVA::uint32 index);

    void SetViewportRect(const DAVA::Rect& rect);
    const DAVA::Rect& GetViewportRect() const;

    void LookAt(const DAVA::AABBox3& box);
    void MoveTo(const DAVA::Vector3& pos);
    void MoveTo(const DAVA::Vector3& pos, const DAVA::Vector3& target);

    DAVA::Vector2 GetScreenPos(const DAVA::Vector3& pos3) const;
    DAVA::Vector3 GetScreenPosAndDepth(const DAVA::Vector3& pos3) const;
    DAVA::Vector3 GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const;

    DAVA::float32 GetDistanceToCamera() const;
    void UpdateDistanceToCamera();

    void Process(DAVA::float32 timeElapsed) override;
    bool Input(DAVA::UIEvent* event) override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    bool SnapEditorCameraToLandscape(bool snap);
    bool IsEditorCameraSnappedToLandscape() const;

    void MoveToSelection();
    void MoveToStep(int ofs);

    void EnableSystem() override;

protected:
    void Draw() override;

private:
    void OnKeyboardInput(DAVA::UIEvent* event);
    void ScrollCamera(DAVA::float32 dy);

    void CreateDebugCameras();
    void RecalcCameraAspect();

    void MoveAnimate(DAVA::float32 timeElapsed);
    DAVA::Entity* GetEntityFromCamera(DAVA::Camera* camera) const;
    DAVA::Entity* GetEntityWithEditorCamera() const;

    DAVA::Rect viewportRect;

    DAVA::Camera* curSceneCamera = nullptr;

    bool animateToNewPos = false;
    DAVA::float32 animateToNewPosTime = 0;
    DAVA::Vector3 newPos;
    DAVA::Vector3 newTar;

    DAVA::Vector<DAVA::Entity*> sceneCameras;
    DAVA::float32 distanceToCamera = 0.f;

    DAVA::uint32 activeSpeedIndex = 0;
};
