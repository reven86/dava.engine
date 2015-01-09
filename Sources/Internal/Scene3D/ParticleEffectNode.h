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



#ifndef __DAVAENGINE_PARTICLE_EFFECT_NODE_H__
#define __DAVAENGINE_PARTICLE_EFFECT_NODE_H__

#include "Entity.h"
#include "ParticleEmitterNode.h"

namespace DAVA {

// Particle Effect Node.
class ParticleEffectNode : public Entity
{
protected:
    ~ParticleEffectNode(){};
public:
    ParticleEffectNode();

    /**
     \brief Access to Scene Nodes.
     */
	virtual void	AddNode(Entity * node);
    virtual void    InsertBeforeNode(Entity *newNode, Entity *beforeNode);

    /**
     \brief Start the playback for all inner nodes.
     */
    void Start();

	void Stop();

	void Restart();
    
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
     \brief Overloaded function of GameObject to update emitter when GameObjectManager updates GameObject.
     \param[in] timeElapsed time in seconds
	 */
	virtual void Update(float32 timeElapsed);

	virtual void Draw();

    /**
     \brief Set the message to be called when Playback is complete.
     */
    void SetPlaybackCompleteMessage(const Message& msg);

	virtual Entity* Clone(Entity *dstNode = NULL);

protected:
    // Check whether Node is Particle Emitter one and check the duration.
    bool PrepareNewParticleEmitterNode(Entity* node);
    
    // Update the duration for all the child nodes.
    void UpdateDurationForChildNodes(float32 newEmitterLifeTime);
    
    // Do we need to stop emitter?
    bool IsStopEmitter(ParticleEmitter* emitter) const;

    // Check the "Playback Complete", emit a message, if needed.
    void CheckPlaybackComplete();
    
private:
    // "Stop after N repeats" value.
    int32 stopAfterNRepeats;
    
    // "Stop if the emitter is empty" value.
    bool stopWhenEmpty;
    
    // Playback complete message.
    Message playbackComplete;
    
    // Effect duration - common for all emitters.
    float32 effectDuration;
    
    // Count of emitters currently stopped.
    int32 emittersCurrentlyStopped;
};

}
#endif /* defined(#ifndef __DAVAENGINE_PARTICLE_EFFECT_NODE_H__) */
