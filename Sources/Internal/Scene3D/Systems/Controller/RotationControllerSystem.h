#ifndef __DAVAENGINE_ROTATION_CONTROLLER_SYSTEM_H__
#define __DAVAENGINE_ROTATION_CONTROLLER_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Math/Vector.h"

namespace DAVA
{
class Camera;
class UIEvent;
class InputCallback;
class RotationControllerSystem : public SceneSystem
{
    static const float32 maxViewAngle;

public:
    RotationControllerSystem(Scene* scene);
    ~RotationControllerSystem() override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void Process(float32 timeElapsed) override;

#if defined(__DAVAENGINE_COREV2__)
    bool Input(UIEvent* event) override;
#else
    void Input(UIEvent* event) override;
#endif

    float32 GetRotationSpeeed() const;
    void SetRotationSpeeed(float32 rotateSpeed);

    const Vector3& GetRotationPoint() const;
    void SetRotationPoint(const Vector3& point);

    void RecalcCameraViewAngles(Camera* camera);

private:
    void RotateDirection(Camera* camera);
    void RotatePosition(Camera* camera);
    void RotatePositionAroundPoint(Camera* camera, const Vector3& pos);

    Vector3 rotationPoint;

    Vector2 rotateStartPoint;
    Vector2 rotateStopPoint;

    float32 curViewAngleZ;
    float32 curViewAngleY;

    float32 rotationSpeed;

#if defined(__DAVAENGINE_COREV2__)
//uint32 inputHandlerToken = 0;
#else
    InputCallback* inputCallback = nullptr;
#endif

    Vector<Entity*> entities;

    Camera* oldCamera;
};

inline float32 RotationControllerSystem::GetRotationSpeeed() const
{
    return rotationSpeed;
}

inline void RotationControllerSystem::SetRotationSpeeed(float32 rotateSpeed)
{
    rotationSpeed = rotateSpeed;
}

inline const Vector3& RotationControllerSystem::GetRotationPoint() const
{
    return rotationPoint;
}
inline void RotationControllerSystem::SetRotationPoint(const Vector3& point)
{
    rotationPoint = point;
}
};

#endif //__DAVAENGINE_WASD_CONTROLLER_SYSTEM_H__
