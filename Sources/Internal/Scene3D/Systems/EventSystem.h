#ifndef __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__
#define __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class SceneSystem;
class Component;
class EventSystem
{
public:
    enum eEventType
    {
        LOCAL_TRANSFORM_CHANGED = 0,
        TRANSFORM_PARENT_CHANGED,
        WORLD_TRANSFORM_CHANGED,
        SWITCH_CHANGED,
        START_PARTICLE_EFFECT,
        STOP_PARTICLE_EFFECT,
        START_ANIMATION,
        STOP_ANIMATION,
        MOVE_ANIMATION_TO_THE_LAST_FRAME,
        MOVE_ANIMATION_TO_THE_FIRST_FRAME,
        SPEED_TREE_MAX_ANIMATED_LOD_CHANGED,
        WAVE_TRIGGERED,
        SOUND_COMPONENT_CHANGED,
        STATIC_OCCLUSION_COMPONENT_CHANGED,
        SKELETON_CONFIG_CHANGED,
        ANIMATION_TRANSFORM_CHANGED,
        SNAP_TO_LANDSCAPE_HEIGHT_CHANGED,
        LOD_DISTANCE_CHANGED,
        LOD_RECURSIVE_UPDATE_ENABLED,

        EVENTS_COUNT
    };

    void RegisterSystemForEvent(SceneSystem* system, uint32 event);
    void UnregisterSystemForEvent(SceneSystem* system, uint32 event);
    void NotifySystem(SceneSystem* system, Component* component, uint32 event);
    void NotifyAllSystems(Component* component, uint32 event);
    void GroupNotifyAllSystems(Vector<Component*>& components, uint32 event);

private:
    Vector<SceneSystem*> registeredSystems[EVENTS_COUNT];
};
}

#endif //__DAVAENGINE_SCENE3D_EVENTSYSTEM_H__