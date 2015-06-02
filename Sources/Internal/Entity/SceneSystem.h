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


#ifndef __DAVAENGINE_SCENE3D_SCENESYSTEM_H__
#define __DAVAENGINE_SCENE3D_SCENESYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class Entity;
class Scene;    
class Component;
class UIEvent;
    
class SceneSystem
{
public:
    SceneSystem(Scene * scene);
    virtual ~SceneSystem();
    
    inline void SetRequiredComponents(uint64 requiredComponents);
    inline uint64 GetRequiredComponents() const;
    
    /**
        \brief  This function is called when any entity registered to scene.
                It sorts out is entity has all necessary components and we need to call AddEntity.
        \param[in] entity entity we've just added
     */
    virtual void RegisterEntity(Entity * entity);
    /**
        \brief  This function is called when any entity unregistered from scene.
                It sorts out is entity has all necessary components and we need to call RemoveEntity.
        \param[in] entity entity we've just removed
     */
    virtual void UnregisterEntity(Entity * entity);
    
    /**
        \brief  This function is called when any component is registered to scene.
                It sorts out is entity has all necessary components and we need to call AddEntity.
        \param[in] entity entity we added component to.
        \param[in] component component we've just added to entity.
     */
    virtual void RegisterComponent(Entity * entity, Component * component);
	
    /**
        \brief  This function is called when any component is unregistered from scene.
                It sorts out is entity has all necessary components and we need to call RemoveEntity.
        \param[in] entity entity we removed component from.
        \param[in] component component we've just removed from entity.
     */
    virtual void UnregisterComponent(Entity * entity, Component * component);

    /**
        \brief  This function can check is (entity, component) pair fits current system required components, if we want to add or remove this component now.
        \param[in] entity entity to check.
        \param[in] component component we want to add or remove.
     */
    bool IsEntityComponentFitsToSystem(Entity * entity, Component * component);
    
    /**
        \brief This function is called only when entity has all required components.
        \param[in] entity entity we want to add.
     */
    virtual void AddEntity(Entity * entity);
    
    /**
        \brief This function is called only when entity had all required components, and don't have them anymore.
        \param[in] entity entity we want to remove.
     */
    virtual void RemoveEntity(Entity * entity);

    /*
        Left these callbacks to full compatibility with old multicomponent solution. Probably will be removed later, if we decide to get rid of multicomponents.
     */
    virtual void AddComponent(Entity * entity, Component * component);
    virtual void RemoveComponent(Entity * entity, Component * component);

	/**
        \brief This function is called when scene loading did finished.
     */
    virtual void SceneDidLoaded();

    /**
        \brief This function is called when event is fired to this system.
        \param[in] entity entity fired an event.
        \param[in] event event id for this event.
     */
    virtual void ImmediateEvent(Entity * entity, uint32 event);
    /**
        \brief This function should be overloaded and perform all processing for this system.
        \param[in] timeElapsed time elapsed from previous frame.
     */
    virtual void Process(float32 timeElapsed);

    
    virtual void Input(UIEvent *event) {};

    
	virtual void SetLocked(bool locked);
	bool IsLocked() const;
    
    
    /**
         \brief This functions should be overloaded if system need to do specific actions on scene activation or deactivation 
     */
    virtual void Activate() {};
    virtual void Deactivate() {};
    
	
protected:
	inline Scene * GetScene() const;

private:
    uint64 requiredComponents;
	Scene * scene;

	bool locked;
};
    
// Inline
inline Scene * SceneSystem::GetScene() const
{
    return scene;
}

inline void SceneSystem::SetRequiredComponents(uint64 _requiredComponents)
{
    requiredComponents = _requiredComponents;
}

inline uint64 SceneSystem::GetRequiredComponents() const
{
    return requiredComponents;
}

};
#endif //__DAVAENGINE_SCENE3D_SCENESYSTEM_H__
