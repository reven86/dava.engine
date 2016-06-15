#ifndef __DAVAENGINE_SWITCH_COMPONENT_H__
#define __DAVAENGINE_SWITCH_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class SwitchComponent : public Component
{
protected:
    ~SwitchComponent(){};

public:
    IMPLEMENT_COMPONENT_TYPE(SWITCH_COMPONENT);

    SwitchComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetSwitchIndex(const int32& switchIndex);
    int32 GetSwitchIndex() const;

private:
    int32 oldSwitchIndex;
    int32 newSwitchIndex;

    friend class SwitchSystem;

public:
    INTROSPECTION_EXTEND(SwitchComponent, Component,
                         PROPERTY("newSwitchIndex", "Switch index", GetSwitchIndex, SetSwitchIndex, I_VIEW | I_EDIT)
                         );
};
}
#endif //__DAVAENGINE_SWITCH_COMPONENT_H__
