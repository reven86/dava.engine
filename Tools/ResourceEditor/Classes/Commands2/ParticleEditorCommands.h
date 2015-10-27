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


#ifndef __PARTICLE_EDITOR_COMMANDS_H__
#define __PARTICLE_EDITOR_COMMANDS_H__

#include <DAVAEngine.h>
#include "Commands2/CommandAction.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

// Add new Particle Emitter.
class CommandAddParticleEmitter: public CommandAction
{
public:
	CommandAddParticleEmitter(DAVA::Entity* effect);
	virtual void Redo();
	
protected:
	DAVA::Entity* effectEntity;
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect: public CommandAction
{
public:
	CommandStartStopParticleEffect(DAVA::Entity* effect, bool isStart);
	
	virtual DAVA::Entity* GetEntity() const;
   	virtual void Redo();

	bool GetStarted() const {return isStart;};

protected:
	DAVA::Entity* effectEntity;
    bool isStart;
};

class CommandRestartParticleEffect: public CommandAction
{
public:
	CommandRestartParticleEffect(DAVA::Entity* effect);

	virtual DAVA::Entity* GetEntity() const;
	virtual void Redo();

protected:
	DAVA::Entity* effectEntity;
};

// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer: public CommandAction
{
public:
	CommandAddParticleEmitterLayer(ParticleEmitter* emitter);
	virtual void Redo();

	ParticleLayer* GetCreatedLayer() const {return createdLayer;};
	ParticleEmitter *GetParentEmitter() const {return selectedEmitter;}

protected:
	ParticleEmitter* selectedEmitter;
	ParticleLayer* createdLayer;
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer: public CommandAction
{
public:
	CommandRemoveParticleEmitterLayer(ParticleEmitter *emitter, ParticleLayer* layer);
	virtual void Redo();

protected:
	ParticleEmitter *selectedEmitter;
	ParticleLayer* selectedLayer;
};

class CommandRemoveParticleEmitter: public CommandAction
{
public:
	CommandRemoveParticleEmitter(ParticleEffectComponent *effect, ParticleEmitter* emitter);
	virtual void Redo();

protected:
	ParticleEffectComponent* selectedEffect;
	ParticleEmitter *selectedEmitter;
	
};

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer: public CommandAction
{
public:
	CommandCloneParticleEmitterLayer(ParticleEmitter *emitter, ParticleLayer* layer);
	virtual void Redo();
	
protected:
	ParticleEmitter *selectedEmitter;
	ParticleLayer* selectedLayer;
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterForce: public CommandAction
{
public:
	CommandAddParticleEmitterForce(ParticleLayer* layer);
	virtual void Redo();
	
protected:
	ParticleLayer* selectedLayer;
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterForce: public CommandAction
{
public:
	CommandRemoveParticleEmitterForce(ParticleLayer* layer, ParticleForce* force);
	virtual void Redo();
	
protected:
	ParticleLayer* selectedLayer;
	ParticleForce* selectedForce;
};

class CommandUpdateEffect: public CommandAction
{
public:
	CommandUpdateEffect(ParticleEffectComponent* particleEffect);
	void Init(float32 playbackSpeed);
	virtual void Redo();

protected:
	ParticleEffectComponent* particleEffect;

	float32 playbackSpeed;	
};

class CommandUpdateEmitter: public CommandAction
{
public:
	CommandUpdateEmitter(ParticleEmitter* emitter);

	void Init(const FastName& name,
			  ParticleEmitter::eType emitterType,
			  RefPtr<PropertyLine<float32> > emissionRange,
			  RefPtr<PropertyLine<Vector3> > emissionVector,
			  RefPtr<PropertyLine<float32> > radius,
              RefPtr<PropertyLine<float32> > emissionAngle,
              RefPtr<PropertyLine<float32> > emissionAngleVariation,
			  RefPtr<PropertyLine<Color> > colorOverLife,
			  RefPtr<PropertyLine<Vector3> > size,
			  float32 life,			  
			  bool isShortEffect);

	ParticleEmitter* GetEmitter() const {return emitter;};

	virtual void Redo();

protected:

	FastName name;
	ParticleEmitter* emitter;

	ParticleEmitter::eType emitterType;
	RefPtr<PropertyLine<float32> > emissionRange;
    RefPtr<PropertyLine<float32> > emissionAngle;
    RefPtr<PropertyLine<float32> > emissionAngleVariation;
	RefPtr<PropertyLine<Vector3> > emissionVector;
	RefPtr<PropertyLine<float32> > radius;
	RefPtr<PropertyLine<Color> > colorOverLife;
	RefPtr<PropertyLine<Vector3> > size;
	float32 life;	
	bool isShortEffect;
};

class CommandUpdateEmitterPosition: public CommandAction
{
public:
    CommandUpdateEmitterPosition(ParticleEffectComponent *effect, ParticleEmitter* emitter);
    void Init(const Vector3& position);            
    virtual void Redo();
protected:    
    ParticleEmitter* emitter;
    ParticleEffectComponent *effect;
    Vector3 position;
};

class CommandUpdateParticleLayerBase : public CommandAction
{
public:
	CommandUpdateParticleLayerBase(CommandID cmdID) :
		CommandAction(cmdID)
	{
	}

	ParticleLayer* GetLayer() const {return layer;};
	
protected:
	ParticleLayer* layer;
};

class CommandUpdateParticleLayer: public CommandUpdateParticleLayerBase
{
public:
	CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer);
    void Init(const String& layerName,
              ParticleLayer::eType layerType,
              ParticleLayer::eDegradeStrategy degradeStrategy,
              bool isDisabled,
              bool inheritPosition,
              bool isLong,
              float32 scaleVelocityBase,
              float32 scaleVelocityFactor,
              bool isLooped,
              Sprite* sprite,
              eBlending blending,
              bool enableFog,
              bool enableFrameBlending,
              int32 particleOrientation,
              RefPtr<PropertyLine<float32>> life,
              RefPtr<PropertyLine<float32>> lifeVariation,
              RefPtr<PropertyLine<float32>> number,
              RefPtr<PropertyLine<float32>> numberVariation,
              RefPtr<PropertyLine<Vector2>> size,
              RefPtr<PropertyLine<Vector2>> sizeVariation,
              RefPtr<PropertyLine<Vector2>> sizeOverLife,
              RefPtr<PropertyLine<float32>> velocity,
              RefPtr<PropertyLine<float32>> velocityVariation,
              RefPtr<PropertyLine<float32>> velocityOverLife,
              RefPtr<PropertyLine<float32>> spin,
              RefPtr<PropertyLine<float32>> spinVariation,
              RefPtr<PropertyLine<float32>> spinOverLife,
              bool randomSpinDirection,

              RefPtr<PropertyLine<Color>> colorRandom,
              RefPtr<PropertyLine<float32>> alphaOverLife,
              RefPtr<PropertyLine<Color>> colorOverLife,
              RefPtr<PropertyLine<float32>> angle,
              RefPtr<PropertyLine<float32>> angleVariation,

              float32 startTime,
              float32 endTime,
              float32 deltaTime,
              float32 deltaVariation,
              float32 loopEndTime,
              float32 loopVariation,
              bool frameOverLifeEnabled,
              float32 frameOverLifeFPS,
              bool randomFrameOnStart,
              bool loopSpriteAnimation,
              RefPtr<PropertyLine<float32>> animSpeedOverLife,

              float32 pivotPointX,
              float32 pivotPointY);

    virtual void Redo();

protected:
	ParticleEmitter* emitter;

	String layerName;
	ParticleLayer::eType layerType;
    ParticleLayer::eDegradeStrategy degradeStrategy;
	bool isDisabled;
	bool isLong;	
	float32 scaleVelocityBase;
	float32 scaleVelocityFactor;
	bool inheritPosition;
	bool isLooped;
	Sprite* sprite;
    eBlending blending;
    bool enableFog;
	bool enableFrameBlending;
	int32 particleOrientation;
	RefPtr< PropertyLine<float32> > life;
	RefPtr< PropertyLine<float32> > lifeVariation;
	RefPtr< PropertyLine<float32> > number;
	RefPtr< PropertyLine<float32> > numberVariation;
	RefPtr< PropertyLine<Vector2> > size;
	RefPtr< PropertyLine<Vector2> > sizeVariation;
	RefPtr< PropertyLine<Vector2> > sizeOverLife;
	RefPtr< PropertyLine<float32> > velocity;
	RefPtr< PropertyLine<float32> > velocityVariation;
	RefPtr< PropertyLine<float32> > velocityOverLife;
	RefPtr< PropertyLine<float32> > spin;
	RefPtr< PropertyLine<float32> > spinVariation;
	RefPtr< PropertyLine<float32> > spinOverLife;
	bool randomSpinDirection;

	RefPtr< PropertyLine<Color> > colorRandom;
	RefPtr< PropertyLine<float32> > alphaOverLife;
	RefPtr< PropertyLine<Color> > colorOverLife;	
	RefPtr< PropertyLine<float32> > angle;
	RefPtr< PropertyLine<float32> > angleVariation;

	float32 startTime;
	float32 endTime;
	float32 deltaTime;
	float32 deltaVariation;
	float32 loopEndTime;
	float32 loopVariation;
	bool frameOverLifeEnabled;
	float32 frameOverLifeFPS;
	bool randomFrameOnStart;
	bool loopSpriteAnimation;
	RefPtr< PropertyLine<float32> > animSpeedOverLife;

	float32 pivotPointX;
	float32 pivotPointY;
};

class CommandUpdateParticleLayerTime: public CommandUpdateParticleLayerBase
{
public:
	CommandUpdateParticleLayerTime(ParticleLayer* layer);
	void Init(float32 startTime, float32 endTime);

	virtual void Redo();

protected:
	float32 startTime;
	float32 endTime;
};

class CommandUpdateParticleLayerEnabled: public CommandUpdateParticleLayerBase
{
public:
	CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled);
	virtual void Redo();
	
protected:
	bool isEnabled;
};

class CommandUpdateParticleLayerLods: public CommandUpdateParticleLayerBase
{
public:
	CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods);
	virtual void Redo();

protected:
	Vector<bool> lods;
};

class CommandUpdateParticleForce: public CommandAction
{
public:
	CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId);
	
	void Init(RefPtr< PropertyLine<Vector3> > force,			  
			  RefPtr< PropertyLine<float32> > forcesOverLife);
	
	virtual void Redo();
	
	ParticleLayer* GetLayer() const {return layer;};
	uint32 GetForceIndex() const {return forceId;};

protected:
	ParticleLayer* layer;
	uint32 forceId;
	
	RefPtr< PropertyLine<Vector3> > force;	
	RefPtr< PropertyLine<float32> > forcesOverLife;
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public CommandAction
{
public:
	CommandLoadParticleEmitterFromYaml(ParticleEmitter* emitter, const FilePath& path);
    virtual void Redo();
	
	ParticleEmitter* GetEmitter() const {return selectedEmitter;};

protected:
	ParticleEmitter* selectedEmitter;
	FilePath filePath;
};

class CommandSaveParticleEmitterToYaml : public CommandAction
{
public:
	CommandSaveParticleEmitterToYaml(ParticleEmitter* emitter, const FilePath& path);
	virtual void Redo();

	ParticleEmitter* GetEmitter() const {return selectedEmitter;};    

protected:
	ParticleEmitter* selectedEmitter;
	FilePath filePath;    
};

/*
// The same for Inner Emitters.
class CommandLoadInnerEmitterFromYaml : public CommandAction
{
public:
	DAVA_DEPRECATED(CommandLoadInnerEmitterFromYaml()); // DEPRECATED: using ParticlesEditorController(QOBJECT)
	
protected:
    virtual void Execute(); 
};

class CommandSaveInnerEmitterToYaml : public CommandAction
{
public:
	DAVA_DEPRECATED(CommandSaveInnerEmitterToYaml(bool forceAskFilename)); // DEPRECATED: using ParticlesEditorController(QOBJECT)
	
protected:
    virtual void Execute(); 
    
    bool forceAskFilename;
};
*/

#endif //__PARTICLE_EDITOR_COMMANDS_H__