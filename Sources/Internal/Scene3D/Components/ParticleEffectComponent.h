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


#ifndef __PARTICLE_EFFECT_COMPONENT_H__
#define __PARTICLE_EFFECT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Base/BaseObject.h"
#include "Base/Message.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Particles/ParticleGroup.h"
#include "Particles/ParticleRenderObject.h"

namespace DAVA
{

class ParticleEmitter;
class ModifiablePropertyLineBase;


class ParticleEffectComponent : public Component
{
	friend class ParticleEffectSystem;
    friend class UIParticles;            

    static const uint32 PARTICLE_FLAGS_SERIALIZATION_CRITERIA = RenderObject::VISIBLE | RenderObject::VISIBLE_REFLECTION | RenderObject::VISIBLE_REFRACTION;
public:
	IMPLEMENT_COMPONENT_TYPE(PARTICLE_EFFECT_COMPONENT);

	enum eState
	{
		STATE_PLAYING,  //effect is playing
        STATE_STARTING, //effect is starting - on next system update it would be moved to playing state (RunEffect called)
		STATE_STOPPING, //effect is stopping - no new particle generation, still need to update and recalculate
		STATE_STOPPED   //effect is completely stopped and removed from active lists
	};

	ParticleEffectComponent();
    ~ParticleEffectComponent();

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *sceneFile);

	void Start();
	void Stop(bool isDeleteAllParticles = true);
	void Restart(bool isDeleteAllParticles = true);
	bool IsStopped();	
	void Pause(bool isPaused = true);
	bool IsPaused();
	void Step(float32 delta);


    void StopAfterNRepeats(int32 numberOfRepeats);    
    void StopWhenEmpty(bool value = true);
	float32 GetPlaybackSpeed();
	void SetPlaybackSpeed(float32 value);
    
	void SetDesiredLodLevel(int32 level);
    
    void SetPlaybackCompleteMessage(const Message & msg);		

	/*externals stuff*/
	void SetExtertnalValue(const String& name, float32 value);
	float32 GetExternalValue(const String& name);	
	Set<String> EnumerateVariables();
	void RebuildEffectModifiables();
	void RegisterModifiable(ModifiablePropertyLineBase *propertyLine);
	void UnRegisterModifiable(ModifiablePropertyLineBase *propertyLine);

	
	int32 GetActiveParticlesCount();

    void SetRenderObjectVisible(bool visible);

     /*sorting offset allowed in 0..31 range, 15 default, more - closer to camera*/
    void SetSortingOffset(uint32 offset);

    bool GetReflectionVisible() const;
    void SetReflectionVisible(bool visible);
    bool GetRefractionVisible() const;
    void SetRefractionVisible(bool visible);

private:
    void ClearGroup(ParticleGroup& group);
	void ClearCurrentGroups();
    void SetGroupsFinishing();
	
	/*effect playback setup       i bit changed logic*/	
	bool stopWhenEmpty;			  //if true effect is considered finished when no particles left, otherwise effect is considered finished if time>effectDuration
	float32 effectDuration;       //duration for effect
	uint32 repeatsCount;			  // note that now it's really count - not depending if effect is stop when empty or by duration - it would be restarted if currRepeatsCount<repetsCount
	bool clearOnRestart;		  // when effect is restarted repeatsCount
	
	float32 playbackSpeed;
	
	/*state*/
	float32 time;
	uint32 currRepeatsCont;	
	bool isPaused;	
	eState state;	
	
	
	/*completion message stuff*/	
	Message playbackComplete;		

	/*externals setup*/	
	MultiMap<String, ModifiablePropertyLineBase *> externalModifiables;	
	Map<String, float32> externalValues;
    
	/*Emitters setup*/
	Vector<ParticleEmitter*> emitters;
    Vector<Vector3> spawnPositions;
		
	ParticleEffectData effectData;
	ParticleRenderObject *effectRenderObject;

	int32 desiredLodLevel, activeLodLevel;

public: //mostly editor commands
	int32 GetEmittersCount();
	ParticleEmitter* GetEmitter(int32 id);
    Vector3 GetSpawnPosition(int id);
    void SetSpawnPosition(int id, Vector3 position);
	void AddEmitter(ParticleEmitter *emitter);
    int32 GetEmitterId(ParticleEmitter *emitter);
    void InsertEmitterAt(ParticleEmitter *emitter, int32 position);
	void RemoveEmitter(ParticleEmitter *emitter);
    float32 GetCurrTime();

    /*statistics for editor*/
    int32 GetLayerActiveParticlesCount(ParticleLayer *layer);
    float32 GetLayerActiveParticlesSquare(ParticleLayer *layer);

public:
	uint32 loadedVersion;
	void CollapseOldEffect(SerializationContext *serializationContext);

	INTROSPECTION_EXTEND(ParticleEffectComponent, Component,
		MEMBER(repeatsCount, "repeatsCount", I_VIEW | I_EDIT | I_SAVE)
        MEMBER(stopWhenEmpty, "stopWhenEmpty",  I_VIEW | I_EDIT | I_SAVE)
        MEMBER(effectDuration, "effectDuration",  I_VIEW | I_EDIT |I_SAVE)	
		MEMBER(clearOnRestart, "clearOnRestart",  I_VIEW | I_EDIT |I_SAVE)	

        PROPERTY("visibleReflection", "Visible Reflection", GetReflectionVisible, SetReflectionVisible, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("visibleRefraction", "Visible Refraction", GetRefractionVisible, SetRefractionVisible, I_SAVE | I_VIEW | I_EDIT)
    );
};

}

#endif //__PARTICLE_EFFECT_COMPONENT_H__