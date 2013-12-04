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

namespace DAVA
{

class ParticleEmitter;
class ModifiablePropertyLineBase;
class ParticleEffectComponent : public Component
{
protected:
    ~ParticleEffectComponent(){};
public:
	IMPLEMENT_COMPONENT_TYPE(PARTICLE_EFFECT_COMPONENT);

	ParticleEffectComponent();

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

	void Start();

	void Stop(bool isDeleteAllParticles = true);

	void Restart();

	/**
     \brief Returns true if all the emitters in the Particle Effect are stopped.    
	 */
	bool IsStopped();

	/**
		\brief Function to pause generation from this effect
		\param[in] isPaused true if you want to pause the generation, false if you want to resume it
	 */
	void Pause(bool isPaused = true);


	bool IsPaused();


	void Step(float32 delta);

    /**
     \brief Function marks that all the emitters must be stopped after N repeats of emitter animation.
     \param[in] numberOfRepeats number of times we need to repeat emitter animation before stop.
     */
    void StopAfterNRepeats(int32 numberOfRepeats);

    /**
     \brief Function marks that this object must be stopped when number of particles will be equal to 0
     */
    void StopWhenEmpty(bool value = true);

    /**
     \brief Per-frame update
     \param[in] timeElapsed time in seconds
	 */
	virtual void EffectUpdate(float32 timeElapsed);

    /**
     \brief Set the message to be called when Playback is complete.
     */
    void SetPlaybackCompleteMessage(const Message & msg);

	/**
     \brief Access to playback speed for the particle emitters. Returns
	 the playback speed for first emitter, sets for all ones.
     */
	float32 GetPlaybackSpeed();
	void SetPlaybackSpeed(float32 value);


	void SetExtertnalValue(const String& name, float32 value);
	float32 GetExternalValue(const String& name);
	
	Set<String> EnumerateVariables();

	void RebuildEffectModifiables();
	void RegisterModifiable(ModifiablePropertyLineBase *propertyLine);
	void UnRegisterModifiable(ModifiablePropertyLineBase *propertyLine);

	/**
     \brief Returns the total active particles count for the whole effect.
     */
	int32 GetActiveParticlesCount();
	

protected:
	// Update the duration for all the child nodes.
	void UpdateDurationForChildNodes(float32 newEmitterLifeTime);

	// Do we need to stop emitter?
	bool IsStopEmitter(ParticleEmitter * emitter) const;

	// Check the "Playback Complete", emit a message, if needed.
	void CheckPlaybackComplete();	

private:
	// "Stop after N repeats" value.
	int32 stopAfterNRepeats;

	// "Stop if the emitter is empty" value.
	bool stopWhenEmpty;

	// Whether we need to emit "Playback Complete" event?
	bool needEmitPlaybackComplete;

	// Playback complete message.
	Message playbackComplete;

	// Effect duration - common for all emitters.
	float32 effectDuration;

	// Count of emitters currently stopped.
	int32 emittersCurrentlyStopped;		

	 bool requireRebuildEffectModifiables;
	MultiMap<String, ModifiablePropertyLineBase *> externalModifiables;	
	Map<String, float32> externalValues;

public:
	INTROSPECTION_EXTEND(ParticleEffectComponent, Component,
		MEMBER(stopAfterNRepeats, "stopAfterNRepeats", I_VIEW | I_EDIT | I_SAVE)
        MEMBER(stopWhenEmpty, "stopWhenEmpty",  I_VIEW | I_EDIT | I_SAVE)
//        MEMBER(needEmitPlaybackComplete, "needEmitPlaybackComplete", INTROSPECTION_SERIALIZABLE)
        MEMBER(effectDuration, "effectDuration",  I_VIEW | I_SAVE)		

    );
};

}

#endif //__PARTICLE_EFFECT_COMPONENT_H__