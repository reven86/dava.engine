#ifndef __DAVAENGINE_ROTATION_CONTROLLER_COMPONENT_H__
#define __DAVAENGINE_ROTATION_CONTROLLER_COMPONENT_H__

#include "Entity/Component.h"
#include "Scene3D/Entity.h"

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
};
};

#endif //__DAVAENGINE_ROTATION_CONTROLLER_COMPONENT_H__
