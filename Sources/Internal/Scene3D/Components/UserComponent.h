#ifndef __DAVAENGINE_USER_COMPONENT_H__
#define __DAVAENGINE_USER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class UserComponent : public Component
{
protected:
    ~UserComponent(){};

public:
    IMPLEMENT_COMPONENT_TYPE(USER_COMPONENT);

    UserComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

public:
    /*
		INTROSPECTION_EXTEND(UserComponent, Component,
			NULL
			);
		*/
};
}
#endif //__DAVAENGINE_USER_COMPONENT_H__
