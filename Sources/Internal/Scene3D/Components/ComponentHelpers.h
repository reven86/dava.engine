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


#ifndef __DAVAENGINE_COMPONENT_HELPERS_H__
#define __DAVAENGINE_COMPONENT_HELPERS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class ParticleEmitter;
class ParticleEffectComponent;
class SkeletonComponent;
class Entity;
class RenderObject;
class Light;
class LightComponent;
class Landscape;
class Camera;
class LodComponent;
class SoundComponent;
class SoundEvent;
class SwitchComponent;
class QualitySettingsComponent;
class TransformComponent;
class RenderComponent;
class VegetationRenderObject;
class CustomPropertiesComponent;
class KeyedArchive;
class SpeedTreeComponent;
class WindComponent;
class WaveComponent;
class SpeedTreeObject;
class AnimationComponent;
class PathComponent;
class WaypointComponent;
class EdgeComponent;
class SnapToLandscapeControllerComponent;

ParticleEffectComponent * GetEffectComponent(const Entity *fromEntity);
AnimationComponent * GetAnimationComponent(const Entity *fromEntity);
TransformComponent * GetTransformComponent(const Entity *fromEntity);
RenderComponent * GetRenderComponent(const Entity *fromEntity);
SkeletonComponent * GetSkeletonComponent(const Entity *fromEntity);
RenderObject * GetRenderObject(const Entity *fromEntity);
VegetationRenderObject * GetVegetation(const Entity *fromEntity);
SpeedTreeObject * GetSpeedTreeObject(const Entity *fromEntity);
SnapToLandscapeControllerComponent * GetSnapToLandscapeControllerComponent(const Entity *fromEntity);

Light *GetLight(const Entity *fromEntity);
LightComponent *GetLightComponent(const Entity *fromEntity);
Landscape *GetLandscape(const Entity *fromEntity);

Camera * GetCamera(const Entity *fromEntity);

SoundComponent * GetSoundComponent(const Entity *fromEntity);

LodComponent * GetLodComponent(const Entity *fromEntity);
SwitchComponent* GetSwitchComponent(const Entity *fromEntity);
    
uint32 GetLodLayersCount(const Entity *fromEntity);
uint32 GetLodLayersCount(LodComponent *fromComponent);
    
    
void RecursiveProcessMeshNode(Entity * curr, void * userData, void(*process)(Entity*, void *));
void RecursiveProcessLodNode(Entity * curr, int32 lod, void * userData, void(*process)(Entity*, void*));

Entity * FindLandscapeEntity(Entity * rootEntity);
Landscape * FindLandscape(Entity * rootEntity);
Entity * FindVegetationEntity(Entity * rootEntity);
VegetationRenderObject* FindVegetation(Entity * rootEntity);

SpeedTreeComponent * GetSpeedTreeComponent(const Entity *fromEntity);
WindComponent * GetWindComponent(const Entity *fromEntity);
WaveComponent * GetWaveComponent(const Entity *fromEntity);

QualitySettingsComponent * GetQualitySettingsComponent(const Entity *fromEntity);
    
CustomPropertiesComponent * GetCustomProperties(const Entity *fromEntity);
CustomPropertiesComponent * GetOrCreateCustomProperties(Entity *fromEntity);
KeyedArchive * GetCustomPropertiesArchieve(const Entity *fromEntity);

PathComponent * GetPathComponent(const Entity *fromEntity);
WaypointComponent* GetWaypointComponent(const Entity* fromEntity);
EdgeComponent* FindEdgeComponent(const Entity *fromEntity, const Entity *toEntity);

}

#endif //__DAVAENGINE_COMPONENT_HELPERS_H__
