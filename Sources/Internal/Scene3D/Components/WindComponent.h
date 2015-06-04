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


#ifndef __DAVAENGINE_SCENE3D_WIND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_WIND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"

namespace DAVA 
{
class WindSystem;
class SerializationContext;
class WindComponent : public Component
{
protected:
	virtual ~WindComponent();
public:
	WindComponent();

	IMPLEMENT_COMPONENT_TYPE(WIND_COMPONENT);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    Vector3 GetDirection() const;

    inline const AABBox3 & GetInfluenceBBox() const;
    inline const float32 & GetWindForce() const;
    inline const float32 & GetWindSpeed() const;

    inline void SetInfluenceBBox(const AABBox3 & bbox);
    inline void SetWindForce(const float32 & force);
    inline void SetWindSpeed(const float32 & speed);
    
protected:
    AABBox3 influenceBbox;
    float32 windForce;
    float32 windSpeed;
    
public:
	INTROSPECTION_EXTEND(WindComponent, Component,
                         PROPERTY("influenceBbox", "influenceBbox", GetInfluenceBBox, SetInfluenceBBox, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("windForce", "windForce", GetWindForce, SetWindForce, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("windSpeed", "windSpeed", GetWindSpeed, SetWindSpeed, I_SAVE | I_VIEW | I_EDIT)
                         );
};

inline const AABBox3 & WindComponent::GetInfluenceBBox() const
{
    return influenceBbox;
}

inline const float32 & WindComponent::GetWindForce() const
{
    return windForce;
}

inline const float32 & WindComponent::GetWindSpeed() const
{
    return windSpeed;
}

inline void WindComponent::SetInfluenceBBox(const AABBox3 & bbox)
{
    influenceBbox = bbox;
}

inline void WindComponent::SetWindForce(const float32 & force)
{
    windForce = force;
}

inline void WindComponent::SetWindSpeed(const float32 & speed)
{
    windSpeed = speed;
}

};

#endif