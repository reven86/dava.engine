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


#ifndef __DAVAENGINE_SCENE3D_WAVE_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_WAVE_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{
class WaveSystem;
class WaveComponent : public Component
{
protected:
	virtual ~WaveComponent();
public:
    WaveComponent();
    WaveComponent(float32 waveAmlitude, float32 waveLenght, float32 waveSpeed, float32 dampingRatio, float32 influenceDistance);

	IMPLEMENT_COMPONENT_TYPE(WAVE_COMPONENT);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    void Trigger();
    
    inline const float32 & GetWaveAmplitude() const;
    inline const float32 & GetWaveLenght() const;
    inline const float32 & GetWaveSpeed() const;
    inline const float32 & GetDampingRatio() const;
    inline const float32 & GetInfluenceRadius() const;

    inline void SetWaveAmplitude(const float32 & amplitude);
    inline void SetWaveLenght(const float32 & lenght);
    inline void SetWaveSpeed(const float32 & speed);
    inline void SetDampingRatio(const float32 & damping);
    inline void SetInfluenceRadius(const float32 & infRadius);

protected:
    float32 amplitude;
    float32 lenght;
    float32 speed;
    float32 damping;
    float32 infRadius;
    
public:
	INTROSPECTION_EXTEND(WaveComponent, Component,
                         PROPERTY("amplitude", "amplitude", GetWaveAmplitude, SetWaveAmplitude, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("lenght", "lenght", GetWaveLenght, SetWaveLenght, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("speed", "speed", GetWaveSpeed, SetWaveSpeed, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("damping", "damping", GetDampingRatio, SetDampingRatio, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("infRadius", "infRadius", GetInfluenceRadius, SetInfluenceRadius, I_SAVE | I_VIEW | I_EDIT)
                         );
};

inline const float32 & WaveComponent::GetWaveAmplitude() const
{
    return amplitude;
}

inline const float32 & WaveComponent::GetWaveLenght() const
{
    return lenght;
}

inline const float32 & WaveComponent::GetWaveSpeed() const
{
    return speed;
}

inline const float32 & WaveComponent::GetDampingRatio() const
{
    return damping;
}

inline const float32 & WaveComponent::GetInfluenceRadius() const
{
    return infRadius;
}

inline void WaveComponent::SetWaveAmplitude(const float32 & _amplitude)
{
    amplitude = _amplitude;
}

inline void WaveComponent::SetWaveLenght(const float32 & _lenght)
{
    lenght = _lenght;
}

inline void WaveComponent::SetWaveSpeed(const float32 & _speed)
{
    speed = _speed;
}

inline void WaveComponent::SetDampingRatio(const float32 & _damping)
{
    damping = _damping;
}

inline void WaveComponent::SetInfluenceRadius(const float32 & _infRadius)
{
    infRadius = _infRadius;
}

};

#endif