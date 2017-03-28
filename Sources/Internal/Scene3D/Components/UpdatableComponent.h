#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"

namespace DAVA
{
class BaseObject;

class IUpdatable
{
public:
    virtual ~IUpdatable(){};
};

class IUpdatableBeforeTransform : public virtual IUpdatable
{
public:
    virtual void UpdateBeforeTransform(float32 timeElapsed) = 0;
};

class IUpdatableAfterTransform : public virtual IUpdatable
{
public:
    virtual void UpdateAfterTransform(float32 timeElapsed) = 0;
};

class UpdatableComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(UPDATABLE_COMPONENT);

    enum eUpdateType
    {
        UPDATE_PRE_TRANSFORM,
        UPDATE_POST_TRANSFORM,

        UPDATES_COUNT
    };

protected:
    ~UpdatableComponent(){};

public:
    UpdatableComponent();

    Component* Clone(Entity* toEntity) override;

    void SetUpdatableObject(IUpdatable* updatableObject);
    IUpdatable* GetUpdatableObject();

private:
    IUpdatable* updatableObject;

public:
    INTROSPECTION_EXTEND(UpdatableComponent, Component,
                         MEMBER(updatableObject, "Updatable Object", I_SAVE)
                         );

    DAVA_VIRTUAL_REFLECTION(UpdatableComponent, Component);
};
}
