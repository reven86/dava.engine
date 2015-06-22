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


#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Base/ObjectFactory.h"

namespace DAVA
{

Component * Component::CreateByType(uint32 componentType)
{
	switch(componentType)
	{
	case TRANSFORM_COMPONENT:
		return new TransformComponent();
	case RENDER_COMPONENT:
		return new RenderComponent();
	case DEBUG_RENDER_COMPONENT: 
		return new DebugRenderComponent();
	case LOD_COMPONENT:
		return new LodComponent();
	case PARTICLE_EFFECT_COMPONENT:
		return new ParticleEffectComponent();
	case BULLET_COMPONENT:
		return new BulletComponent();
	case UPDATABLE_COMPONENT:
		return new UpdatableComponent();
	case CAMERA_COMPONENT:
		return new CameraComponent();
	case LIGHT_COMPONENT:
		return new LightComponent();
	case SWITCH_COMPONENT:
		return new SwitchComponent();
	case USER_COMPONENT:
		return new UserComponent();
	case SOUND_COMPONENT:
		return new SoundComponent();
    case SPEEDTREE_COMPONENT:
        return new SpeedTreeComponent();
    case WIND_COMPONENT:
        return new WindComponent();
    case WAVE_COMPONENT:
        return new WaveComponent();
	case CUSTOM_PROPERTIES_COMPONENT:
		return new CustomPropertiesComponent();
	case ACTION_COMPONENT:
		return new ActionComponent();
    case STATIC_OCCLUSION_COMPONENT:
        return new StaticOcclusionComponent();
    case STATIC_OCCLUSION_DATA_COMPONENT:
        return new StaticOcclusionDataComponent();
    case QUALITY_SETTINGS_COMPONENT:
        return new QualitySettingsComponent();
    case SKELETON_COMPONENT:
        return new SkeletonComponent();
    case PATH_COMPONENT:
        return new PathComponent();
    case WAYPOINT_COMPONENT:
        return new WaypointComponent();
    case EDGE_COMPONENT:
        return new EdgeComponent();
    case ROTATION_CONTROLLER_COMPONENT:
        return new RotationControllerComponent();
    case SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT:
        return new SnapToLandscapeControllerComponent();
    case WASD_CONTROLLER_COMPONENT:
        return new WASDControllerComponent();


	case ANIMATION_COMPONENT:
	case COLLISION_COMPONENT:
	case SCRIPT_COMPONENT:
        
	default:
		DVASSERT(0);
		return 0;
	}

}

Component::Component()
    : entity(0)
{

}

Component::~Component()
{ }

void Component::SetEntity(Entity * _entity)
{
	entity = _entity;
}

void Component::GetDataNodes(Set<DAVA::DataNode *> &dataNodes)
{
    //Empty as default
}

void Component::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		archive->SetUInt32("comp.type", GetType());
		archive->SetString("comp.typename", ObjectFactory::Instance()->GetName(this));
	}
}

void Component::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(NULL != archive)
	{
		uint32 type = archive->GetUInt32("comp.type", 0xFFFFFFFF);
		DVASSERT(type == GetType());
	}
}

}
