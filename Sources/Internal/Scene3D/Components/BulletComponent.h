#ifndef __DAVAENGINE_BULLET_COMPONENT_H__
#define __DAVAENGINE_BULLET_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class BaseObject;
class BulletComponent : public Component
{
protected:
    virtual ~BulletComponent();

public:
    IMPLEMENT_COMPONENT_TYPE(BULLET_COMPONENT);

    BulletComponent();

    void SetBulletObject(BaseObject* bulletObject);
    BaseObject* GetBulletObject();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    BaseObject* bulletObject;

public:
    INTROSPECTION_EXTEND(BulletComponent, Component,
                         PROPERTY("bulletObject", "Bullet Object", GetBulletObject, SetBulletObject, I_SAVE | I_VIEW | I_EDIT)
                         );
};
}

#endif //__DAVAENGINE_BULLET_COMPONENT_H__
