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


#ifndef __DAVAENGINE_SCENE3D_SPEEDTREE_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SPEEDTREE_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{

class SpeedTreeUpdateSystem;
class SpeedTreeComponent : public Component
{
protected:
	virtual ~SpeedTreeComponent();

public:
	SpeedTreeComponent();

	IMPLEMENT_COMPONENT_TYPE(SPEEDTREE_COMPONENT);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    inline float32 GetTrunkOscillationAmplitude() const;
    inline float32 GetTrunkOscillationSpring() const;
    inline float32 GetTrunkOscillationDamping() const;
    inline float32 GetLeafsOscillationApmlitude() const;
    inline float32 GetLeafsOscillationSpeed() const;
    inline int32 GetMaxAnimatedLOD() const;

    inline void SetTrunkOscillationAmplitude(const float32 & amplitude);
    inline void SetTrunkOscillationSpring(const float32 & spring);
    inline void SetTrunkOscillationDamping(const float32 & damping);
    inline void SetLeafsOscillationApmlitude(const float32 & amplitude);
    inline void SetLeafsOscillationSpeed(const float32 & speed);
    void SetMaxAnimatedLOD(const int32 & lodIndex);
    
protected:
    float32 trunkOscillationAmplitude;
    float32 trunkOscillationSpring;
    float32 trunkOscillationDamping;
    float32 leafsOscillationAmplitude;
    float32 leafsOscillationSpeed;
    int32 maxAnimatedLOD;

    //runtime properties
    Matrix4 wtInvMx;
    Vector3 wtPosition;

    Vector2 oscVelocity;
    Vector2 oscOffset;
    float32 leafTime;

public:
    INTROSPECTION_EXTEND(SpeedTreeComponent, Component, 
        PROPERTY("trunkOscillationAmplitude", "trunkOscillationAmplitude", GetTrunkOscillationAmplitude, SetTrunkOscillationAmplitude, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("trunkOscillationSpring", "trunkOscillationSpring", GetTrunkOscillationSpring, SetTrunkOscillationSpring, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("trunkOscillationDamping", "trunkOscillationDamping", GetTrunkOscillationDamping, SetTrunkOscillationDamping, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("leafsOscillationAmplitude", "leafsOscillationAmplitude", GetLeafsOscillationApmlitude, SetLeafsOscillationApmlitude, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("leafsOscillationSpeed", "leafsOscillationSpeed", GetLeafsOscillationSpeed, SetLeafsOscillationSpeed, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("maxAnimatedLOD", "maxAnimatedLOD", GetMaxAnimatedLOD, SetMaxAnimatedLOD, I_SAVE | I_VIEW | I_EDIT)
        );
    
	friend class SpeedTreeUpdateSystem;
};

inline float32 SpeedTreeComponent::GetTrunkOscillationAmplitude() const
{
    return trunkOscillationAmplitude;
}

inline float32 SpeedTreeComponent::GetTrunkOscillationSpring() const
{
    return trunkOscillationSpring;
}

inline float32 SpeedTreeComponent::GetTrunkOscillationDamping() const
{
    return trunkOscillationDamping;
}

inline float32 SpeedTreeComponent::GetLeafsOscillationApmlitude() const
{
    return leafsOscillationAmplitude;
}

inline float32 SpeedTreeComponent::GetLeafsOscillationSpeed() const
{
    return leafsOscillationSpeed;
}

inline int32 SpeedTreeComponent::GetMaxAnimatedLOD() const
{
    return maxAnimatedLOD;
}

inline void SpeedTreeComponent::SetTrunkOscillationAmplitude(const float32 & amplitude)
{
    trunkOscillationAmplitude = amplitude;
}

inline void SpeedTreeComponent::SetTrunkOscillationSpring(const float32 & spring)
{
    trunkOscillationSpring = Clamp(spring, .5f, 20.f);
}

inline void SpeedTreeComponent::SetTrunkOscillationDamping(const float32 & damping)
{
    trunkOscillationDamping = Clamp(damping, .0f, 20.f);
}

inline void SpeedTreeComponent::SetLeafsOscillationApmlitude(const float32 & amplitude)
{
    leafsOscillationAmplitude = amplitude;
}

inline void SpeedTreeComponent::SetLeafsOscillationSpeed(const float32 & speed)
{
    leafsOscillationSpeed = speed;
}

};

#endif