#ifndef __DAVAENGINE_WASD_CONTROLLER_SYSTEM_H__
#define __DAVAENGINE_WASD_CONTROLLER_SYSTEM_H__

#include "Entity/SceneSystem.h"

namespace DAVA
{
class Camera;
class UIEvent;
class InputCallback;

class WASDControllerSystem : public SceneSystem
{
    enum eDirection
    {
        DIRECTION_STRAIGHT = 1,
        DIRECTION_INVERSE = -1
    };

public:
    WASDControllerSystem(Scene* scene);
    virtual ~WASDControllerSystem();

    virtual void AddEntity(Entity* entity);
    virtual void RemoveEntity(Entity* entity);

    virtual void Process(float32 timeElapsed);

    float32 GetMoveSpeeed() const;
    void SetMoveSpeed(float32 moveSpeed);

private:
    void MoveForward(Camera* camera, float32 speed, eDirection direction);
    void MoveRight(Camera* camera, float32 speed, eDirection direction);

    float32 moveSpeed;

    Vector<Entity*> entities;
};

inline float32 WASDControllerSystem::GetMoveSpeeed() const
{
    return moveSpeed;
}

inline void WASDControllerSystem::SetMoveSpeed(float32 _moveSpeed)
{
    moveSpeed = _moveSpeed;
}
};

#endif //__DAVAENGINE_WASD_CONTROLLER_SYSTEM_H__
