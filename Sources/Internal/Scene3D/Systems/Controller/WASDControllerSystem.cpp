#include "WASDControllerSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Utils/Utils.h"

namespace DAVA
{
WASDControllerSystem::WASDControllerSystem(Scene* scene)
    : SceneSystem(scene)
    , moveSpeed(1.f)
{
}

WASDControllerSystem::~WASDControllerSystem()
{
}

void WASDControllerSystem::AddEntity(Entity* entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");

    entities.push_back(entity);
}

void WASDControllerSystem::RemoveEntity(Entity* entity)
{
    const bool removeResult = FindAndRemoveExchangingWithLast(entities, entity);
    DVASSERT(removeResult);
}

void WASDControllerSystem::Process(float32 timeElapsed)
{
    float32 actualMoveSpeed = moveSpeed * timeElapsed;

    const uint32 size = static_cast<uint32>(entities.size());
    if (0 == size)
        return;

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::LCTRL) || keyboard.IsKeyPressed(Key::LALT) || keyboard.IsKeyPressed(Key::RALT) || keyboard.IsKeyPressed(Key::RCTRL))
    {
        return;
    }

    for (uint32 i = 0; i < size; ++i)
    {
        Camera* camera = GetCamera(entities[i]);
        if ((camera != nullptr) && (camera == GetScene()->GetDrawCamera()))
        {
            if (keyboard.IsKeyPressed(Key::KEY_W) || keyboard.IsKeyPressed(Key::UP))
            {
                MoveForward(camera, actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard.IsKeyPressed(Key::KEY_S) || keyboard.IsKeyPressed(Key::DOWN))
            {
                MoveForward(camera, actualMoveSpeed, DIRECTION_INVERSE);
            }
            if (keyboard.IsKeyPressed(Key::KEY_D) || keyboard.IsKeyPressed(Key::RIGHT))
            {
                MoveRight(camera, actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard.IsKeyPressed(Key::KEY_A) || keyboard.IsKeyPressed(Key::LEFT))
            {
                MoveRight(camera, actualMoveSpeed, DIRECTION_INVERSE);
            }
        }
    }
}

void WASDControllerSystem::MoveForward(Camera* camera, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();

    pos += (dir * speed * static_cast<float32>(direction));

    camera->SetPosition(pos);
    camera->SetDirection(dir);
}

void WASDControllerSystem::MoveRight(Camera* camera, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();
    Vector3 left = camera->GetLeft();

    pos += (left * speed * static_cast<float32>(direction));

    camera->SetPosition(pos);
    camera->SetDirection(dir);
}
};
