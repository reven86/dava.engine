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
#include "Commands2/Base/CommandAction.h"

class CommandAddParticleEmitter : public CommandAction
{
public:
    CommandAddParticleEmitter(DAVA::Entity* effect);
    virtual void Redo();

protected:
    DAVA::Entity* effectEntity;
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect : public CommandAction
{
public:
    CommandStartStopParticleEffect(DAVA::Entity* effect, bool isStart);

    virtual DAVA::Entity* GetEntity() const;
    virtual void Redo();

    bool GetStarted() const
    {
        return isStart;
    };

protected:
    DAVA::Entity* effectEntity;
    bool isStart;
};

class CommandRestartParticleEffect : public CommandAction
{
public:
    CommandRestartParticleEffect(DAVA::Entity* effect);

    virtual DAVA::Entity* GetEntity() const;
    virtual void Redo();

protected:
    DAVA::Entity* effectEntity;
};

// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer : public CommandAction
{
public:
    CommandAddParticleEmitterLayer(DAVA::ParticleEmitter* emitter);
    virtual void Redo();

    DAVA::ParticleLayer* GetCreatedLayer() const
    {
        return createdLayer;
    };
    DAVA::ParticleEmitter* GetParentEmitter() const
    {
        return selectedEmitter;
    }

protected:
    DAVA::ParticleEmitter* selectedEmitter;
    DAVA::ParticleLayer* createdLayer;
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer : public CommandAction
{
public:
    CommandRemoveParticleEmitterLayer(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    virtual void Redo();

protected:
    DAVA::ParticleEmitter* selectedEmitter;
    DAVA::ParticleLayer* selectedLayer;
};

class CommandRemoveParticleEmitter : public CommandAction
{
public:
    CommandRemoveParticleEmitter(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    virtual void Redo();

protected:
    DAVA::ParticleEffectComponent* selectedEffect;
    DAVA::ParticleEmitter* selectedEmitter;
};

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer : public CommandAction
{
public:
    CommandCloneParticleEmitterLayer(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    virtual void Redo();

protected:
    DAVA::ParticleEmitter* selectedEmitter;
    DAVA::ParticleLayer* selectedLayer;
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterForce : public CommandAction
{
public:
    CommandAddParticleEmitterForce(DAVA::ParticleLayer* layer);
    virtual void Redo();

protected:
    DAVA::ParticleLayer* selectedLayer;
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterForce : public CommandAction
{
public:
    CommandRemoveParticleEmitterForce(DAVA::ParticleLayer* layer, DAVA::ParticleForce* force);
    virtual void Redo();

protected:
    DAVA::ParticleLayer* selectedLayer;
    DAVA::ParticleForce* selectedForce;
};

class CommandUpdateEffect : public CommandAction
{
public:
    CommandUpdateEffect(DAVA::ParticleEffectComponent* particleEffect);
    void Init(DAVA::float32 playbackSpeed);
    virtual void Redo();

protected:
    DAVA::ParticleEffectComponent* particleEffect;

    DAVA::float32 playbackSpeed;
};

class CommandUpdateEmitter : public CommandAction
{
public:
    CommandUpdateEmitter(DAVA::ParticleEmitter* emitter);

    void Init(const DAVA::FastName& name,
              DAVA::ParticleEmitter::eType emitterType,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionRange,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> emissionVector,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> radius,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionAngle,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionAngleVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorOverLife,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> size,
              DAVA::float32 life,
              bool isShortEffect);

    DAVA::ParticleEmitter* GetEmitter() const
    {
        return emitter;
    };

    virtual void Redo();

protected:
    DAVA::FastName name;
    DAVA::ParticleEmitter* emitter;

    DAVA::ParticleEmitter::eType emitterType;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionRange;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionAngle;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> emissionAngleVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> emissionVector;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> radius;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorOverLife;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> size;
    DAVA::float32 life;
    bool isShortEffect;
};

class CommandUpdateEmitterPosition : public CommandAction
{
public:
    CommandUpdateEmitterPosition(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void Init(const DAVA::Vector3& position);
    virtual void Redo();

protected:
    DAVA::ParticleEmitter* emitter;
    DAVA::ParticleEffectComponent* effect;
    DAVA::Vector3 position;
};

class CommandUpdateParticleLayerBase : public CommandAction
{
public:
    CommandUpdateParticleLayerBase(CommandID cmdID)
        : CommandAction(cmdID)
    {
    }

    DAVA::ParticleLayer* GetLayer() const
    {
        return layer;
    };

protected:
    DAVA::ParticleLayer* layer;
};

class CommandUpdateParticleLayer : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayer(DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    void Init(const DAVA::String& layerName,
              DAVA::ParticleLayer::eType layerType,
              DAVA::ParticleLayer::eDegradeStrategy degradeStrategy,
              bool isDisabled,
              bool inheritPosition,
              bool isLong,
              DAVA::float32 scaleVelocityBase,
              DAVA::float32 scaleVelocityFactor,
              bool isLooped,
              DAVA::int32 particleOrientation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> life,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> lifeVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> number,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> numberVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> size,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> sizeVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> sizeOverLife,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocity,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocityVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocityOverLife,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spin,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spinVariation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spinOverLife,
              bool randomSpinDirection,

              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorRandom,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> alphaOverLife,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorOverLife,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> angle,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> angleVariation,

              DAVA::float32 startTime,
              DAVA::float32 endTime,
              DAVA::float32 deltaTime,
              DAVA::float32 deltaVariation,
              DAVA::float32 loopEndTime,
              DAVA::float32 loopVariation,
              bool frameOverLifeEnabled,
              DAVA::float32 frameOverLifeFPS,
              bool randomFrameOnStart,
              bool loopSpriteAnimation,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> animSpeedOverLife,

              DAVA::float32 pivotPointX,
              DAVA::float32 pivotPointY);

    virtual void Redo();

protected:
    DAVA::ParticleEmitter* emitter;

    DAVA::String layerName;
    DAVA::ParticleLayer::eType layerType;
    DAVA::ParticleLayer::eDegradeStrategy degradeStrategy;
    bool isDisabled;
    bool isLong;
    DAVA::float32 scaleVelocityBase;
    DAVA::float32 scaleVelocityFactor;
    bool inheritPosition;
    bool isLooped;
    DAVA::int32 particleOrientation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> life;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> lifeVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> number;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> numberVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> size;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> sizeVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector2>> sizeOverLife;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocity;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocityVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> velocityOverLife;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spin;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spinVariation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> spinOverLife;
    bool randomSpinDirection;

    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorRandom;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> alphaOverLife;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Color>> colorOverLife;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> angle;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> angleVariation;

    DAVA::float32 startTime;
    DAVA::float32 endTime;
    DAVA::float32 deltaTime;
    DAVA::float32 deltaVariation;
    DAVA::float32 loopEndTime;
    DAVA::float32 loopVariation;
    bool frameOverLifeEnabled;
    DAVA::float32 frameOverLifeFPS;
    bool randomFrameOnStart;
    bool loopSpriteAnimation;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> animSpeedOverLife;

    DAVA::float32 pivotPointX;
    DAVA::float32 pivotPointY;
};

class CommandUpdateParticleLayerTime : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerTime(DAVA::ParticleLayer* layer);
    void Init(DAVA::float32 startTime, DAVA::float32 endTime);

    virtual void Redo();

protected:
    DAVA::float32 startTime;
    DAVA::float32 endTime;
};

class CommandUpdateParticleLayerEnabled : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerEnabled(DAVA::ParticleLayer* layer, bool isEnabled);
    virtual void Redo();

protected:
    bool isEnabled;
};

class CommandUpdateParticleLayerLods : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerLods(DAVA::ParticleLayer* layer, const DAVA::Vector<bool>& lods);
    virtual void Redo();

protected:
    DAVA::Vector<bool> lods;
};

class CommandUpdateParticleForce : public CommandAction
{
public:
    CommandUpdateParticleForce(DAVA::ParticleLayer* layer, DAVA::uint32 forceId);

    void Init(DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> force,
              DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> forcesOverLife);

    virtual void Redo();

    DAVA::ParticleLayer* GetLayer() const
    {
        return layer;
    };
    DAVA::uint32 GetForceIndex() const
    {
        return forceId;
    };

protected:
    DAVA::ParticleLayer* layer;
    DAVA::uint32 forceId;

    DAVA::RefPtr<DAVA::PropertyLine<DAVA::Vector3>> force;
    DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> forcesOverLife;
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadParticleEmitterFromYaml(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, const DAVA::FilePath& path);
    virtual void Redo();

    DAVA::ParticleEmitter* GetEmitter() const
    {
        return selectedEmitter.Get();
    };

protected:
    DAVA::ParticleEffectComponent* selectedEffect = nullptr;
    DAVA::RefPtr<DAVA::ParticleEmitter> selectedEmitter;
    DAVA::FilePath filePath;
};

class CommandSaveParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveParticleEmitterToYaml(DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, const DAVA::FilePath& path);
    virtual void Redo();

    DAVA::ParticleEmitter* GetEmitter() const
    {
        return selectedEmitter.Get();
    };

protected:
    DAVA::ParticleEffectComponent* selectedEffect = nullptr;
    DAVA::RefPtr<DAVA::ParticleEmitter> selectedEmitter;
    DAVA::FilePath filePath;
};

// Load/save Particle Inner Emitter Node.
class CommandLoadInnerParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadInnerParticleEmitterFromYaml(DAVA::ParticleEmitter* emitter, const DAVA::FilePath& path);
    virtual void Redo();

    DAVA::ParticleEmitter* GetEmitter() const
    {
        return selectedEmitter;
    };

protected:
    DAVA::ParticleEmitter* selectedEmitter = nullptr;
    DAVA::FilePath filePath;
};

class CommandSaveInnerParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveInnerParticleEmitterToYaml(DAVA::ParticleEmitter* emitter, const DAVA::FilePath& path);
    virtual void Redo();

    DAVA::ParticleEmitter* GetEmitter() const
    {
        return selectedEmitter;
    };

protected:
    DAVA::ParticleEmitter* selectedEmitter = nullptr;
    DAVA::FilePath filePath;
};

#endif //__PARTICLE_EDITOR_COMMANDS_H__