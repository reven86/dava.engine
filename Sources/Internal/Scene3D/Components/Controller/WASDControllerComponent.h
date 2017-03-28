#pragma once

#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class WASDControllerComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(WASD_CONTROLLER_COMPONENT);

    Component* Clone(Entity* toEntity) override;

public:
    INTROSPECTION_EXTEND(WASDControllerComponent, Component,
                         NULL
                         );

    DAVA_VIRTUAL_REFLECTION(WASDControllerComponent, Component);
};
};
