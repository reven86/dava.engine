/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_SCENE3D_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Base/Serializable.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA 
{
    
class Entity;
class Component : public Serializable, public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_COMPONENT);

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
        COLLISION_COMPONENT,    // multiple instances
        PHYSICS_COMPONENT,
        ACTION_COMPONENT,       // actions, something simplier than scripts that can influence logic, can be multiple
        SCRIPT_COMPONENT,       // multiple instances, not now, it will happen much later.
		USER_COMPONENT,
		SOUND_COMPONENT,
		CUSTOM_PROPERTIES_COMPONENT,
        STATIC_OCCLUSION_COMPONENT,
        STATIC_OCCLUSION_DATA_COMPONENT, 
        QUALITY_SETTINGS_COMPONENT,   // type as fastname for detecting type of model
        SPEEDTREE_COMPONENT,
        WIND_COMPONENT,
        WAVE_COMPONENT,
        SKELETON_COMPONENT,
        PATH_COMPONENT,
        ROTATION_CONTROLLER_COMPONENT,
        SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT,
        WASD_CONTROLLER_COMPONENT,
        
        //debug components - note that everything below won't be serialized
        DEBUG_COMPONENTS,
        STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT,
        WAYPOINT_COMPONENT,
        EDGE_COMPONENT,

        FIRST_USER_DEFINED_COMPONENT = 48,
        COMPONENT_COUNT = 64
    };

public:
	static Component * CreateByType(uint32 componentType);

	Component();
	virtual ~Component();

    virtual uint32 GetType() const = 0;
    virtual Component* Clone(Entity * toEntity) = 0;
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

	inline Entity* GetEntity() const;
	virtual void SetEntity(Entity * entity);
    
    /**
         \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
	/**
	 \brief This function optimize component before export.
	*/
	virtual void OptimizeBeforeExport() {};

    /**
         \brief Function to get data nodes of requested type to specific container you provide.
     */
    template<template <typename> class Container, class T>
    void GetDataNodes(Container<T> & container);

protected:
    Entity * entity; // entity is a Entity, that this component belongs to

public:
	INTROSPECTION(Component, 
		MEMBER(entity, "entity", I_SAVE)
		);
};

inline Entity* Component::GetEntity() const
{
	return entity;
};

#define IMPLEMENT_COMPONENT_TYPE(TYPE) \
    virtual uint32 GetType() const { return TYPE; }; \
    static const uint32 C_TYPE = TYPE; 

#define MAKE_COMPONENT_MASK(x) ((uint64)1 << (uint64)x)
    
template<template <typename> class Container, class T>
void Component::GetDataNodes(Container<T> & container)
{
    Set<DataNode*> objects;
    GetDataNodes(objects);
    
    Set<DataNode*>::const_iterator end = objects.end();
    for (Set<DataNode*>::iterator t = objects.begin(); t != end; ++t)
    {
        DataNode* obj = *t;
        
        T res = dynamic_cast<T> (obj);
        if (res)
            container.push_back(res);
    }	
}

    
};
#endif //__DAVAENGINE_SCENE3D_COMPONENT_H__
