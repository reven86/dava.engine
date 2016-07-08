#ifndef __DAVAENGINE_WASD_CONTROLLER_COMPONENT_H__
#define __DAVAENGINE_WASD_CONTROLLER_COMPONENT_H__

#include "Entity/Component.h"
#include "Scene3D/Entity.h"

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
};
};

#endif //__DAVAENGINE_WASD_CONTROLLER_COMPONENT_H__
