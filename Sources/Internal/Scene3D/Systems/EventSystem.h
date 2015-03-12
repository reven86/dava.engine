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



#ifndef __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__
#define __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class SceneSystem;
class Entity;
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
        SPEED_TREE_MAX_ANIMATED_LOD_CHANGED,
        WAVE_TRIGGERED,
        SOUND_COMPONENT_CHANGED,
        STATIC_OCCLUSION_COMPONENT_CHANGED,
        SKELETON_CONFIG_CHANGED,
        ANIMATION_TRANSFORM_CHANGED,
        SNAP_TO_LANDSCAPE_HEIGHT_CHANGED,

		EVENTS_COUNT
	};

	void RegisterSystemForEvent(SceneSystem * system, uint32 event);
	void UnregisterSystemForEvent(SceneSystem * system, uint32 event);
	void NotifySystem(SceneSystem * system, Entity * entity, uint32 event);
    void NotifyAllSystems(Entity * entity, uint32 event);
    void GroupNotifyAllSystems(Vector<Entity *> & entity, uint32 event);

private:
	Vector<SceneSystem*> registeredSystems[EVENTS_COUNT];
};

}

#endif //__DAVAENGINE_SCENE3D_EVENTSYSTEM_H__