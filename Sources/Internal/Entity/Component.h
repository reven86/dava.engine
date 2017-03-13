#pragma once

#include "Base/BaseTypes.h"
#include "Base/Serializable.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "MemoryManager/MemoryProfiler.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
class Component : public Serializable, public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_COMPONENT)

public:
    enum eType
    {
        TRANSFORM_COMPONENT = 0,
        RENDER_COMPONENT,
        LOD_COMPONENT,
        DEBUG_RENDER_COMPONENT,
        SWITCH_COMPONENT,
        CAMERA_COMPONENT,
        LIGHT_COMPONENT,
        PARTICLE_EFFECT_COMPONENT,
        BULLET_COMPONENT,
        UPDATABLE_COMPONENT,
        ANIMATION_COMPONENT,
        COLLISION_COMPONENT, // multiple instances
        PHYSICS_COMPONENT,
        ACTION_COMPONENT, // actions, something simplier than scripts that can influence logic, can be multiple
        SCRIPT_COMPONENT, // multiple instances, not now, it will happen much later.
        USER_COMPONENT,
        SOUND_COMPONENT,
        CUSTOM_PROPERTIES_COMPONENT,
        STATIC_OCCLUSION_COMPONENT,
        STATIC_OCCLUSION_DATA_COMPONENT,
        QUALITY_SETTINGS_COMPONENT, // type as fastname for detecting type of model
        SPEEDTREE_COMPONENT,
        WIND_COMPONENT,
        WAVE_COMPONENT,
        SKELETON_COMPONENT,
        PATH_COMPONENT,
        ROTATION_CONTROLLER_COMPONENT,
        SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT,
        WASD_CONTROLLER_COMPONENT,
        VISIBILITY_CHECK_COMPONENT,

        //debug components - note that everything below won't be serialized
        DEBUG_COMPONENTS,
        STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT,
        WAYPOINT_COMPONENT,
        EDGE_COMPONENT,

        FIRST_USER_DEFINED_COMPONENT = 48,
        COMPONENT_COUNT = 64
    };

public:
    static Component* CreateByType(uint32 componentType);

    ~Component() override;

    virtual uint32 GetType() const = 0;

    /**
    Clone component. Then add cloned component to specified `toEntity` if `toEntity` is not nullptr. Return cloned component.
    */
    virtual Component* Clone(Entity* toEntity) = 0;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline Entity* GetEntity() const;
    virtual void SetEntity(Entity* entity);

    /**
        \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*>& dataNodes);
    /**
	 \brief This function optimize component before export.
	*/
    virtual void OptimizeBeforeExport()
    {
    }

    /**
        \brief Function to get data nodes of requested type to specific container you provide.
     */
    template <template <typename> class Container, class T>
    void GetDataNodes(Container<T>& container);

protected:
    Entity* entity = 0;

public:
    INTROSPECTION(Component,
                  MEMBER(entity, "entity", I_SAVE)
                  )

    DAVA_VIRTUAL_REFLECTION(Component, InspBase);
};

inline Entity* Component::GetEntity() const
{
    return entity;
};

#define IMPLEMENT_COMPONENT_TYPE(TYPE) \
    uint32 GetType() const override { return TYPE; }; \
    static const uint32 C_TYPE = TYPE; 

#define MAKE_COMPONENT_MASK(x) (1ULL << static_cast<DAVA::uint64>(x))

template <template <typename> class Container, class T>
void Component::GetDataNodes(Container<T>& container)
{
    Set<DataNode*> objects;
    GetDataNodes(objects);

    Set<DataNode*>::const_iterator end = objects.end();
    for (Set<DataNode*>::iterator t = objects.begin(); t != end; ++t)
    {
        DataNode* obj = *t;

        T res = dynamic_cast<T>(obj);
        if (res)
            container.push_back(res);
    }
}
};
