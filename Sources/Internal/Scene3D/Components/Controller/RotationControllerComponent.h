#pragma once

#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class RotationControllerComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(ROTATION_CONTROLLER_COMPONENT);

    Component* Clone(Entity* toEntity) override;

public:
    INTROSPECTION_EXTEND(RotationControllerComponent, Component,
                         NULL
                         );

    DAVA_VIRTUAL_REFLECTION(RotationControllerComponent, Component);
};
};
