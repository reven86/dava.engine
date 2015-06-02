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


#ifndef __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Sound/SoundEvent.h"

namespace DAVA 
{

class SoundComponent;
struct SoundComponentElement
{
    SoundComponentElement(SoundEvent * _soundEvent, uint32 _flags, const Vector3 & _localDirection) : 
        soundEvent(_soundEvent),
        localDirection(_localDirection),
        flags(_flags)
        {}

    SoundEvent * soundEvent;
    Vector3 localDirection;
    uint32 flags;

    INTROSPECTION(SoundComponentElement, NULL);
};

class SoundComponent : public Component
{
public:
    enum eEventFlags
    {
        FLAG_AUTO_DISTANCE_TRIGGER = 1 << 0
    };

    SoundComponent();
    virtual ~SoundComponent();

    virtual Component * Clone(Entity * toEntity);

    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);
    
    inline uint32 GetEventsCount() const;
    inline SoundEvent * GetSoundEvent(uint32 index) const;

    void Trigger();
    void Stop();
    void Trigger(uint32 index);
    void Stop(uint32 index);

    void SetSoundEventFlags(uint32 eventIndex, uint32 flags);
    inline uint32 GetSoundEventFlags(uint32 eventIndex) const;

    void AddSoundEvent(SoundEvent * _event, uint32 flags = 0, const Vector3 & direction = Vector3(1.f, 0.f, 0.f));
    void RemoveSoundEvent(SoundEvent * event);
    void RemoveAllEvents();

    void SetLocalDirection(uint32 eventIndex, const Vector3 & direction);
    void SetLocalDirection(const DAVA::Vector3 &direction);
    inline const Vector3 & GetLocalDirection(uint32 eventIndex) const;

    IMPLEMENT_COMPONENT_TYPE(SOUND_COMPONENT);
    
protected:
    Vector<SoundComponentElement> events;

public:
    INTROSPECTION_EXTEND(SoundComponent, Component,
        COLLECTION(events, "events", I_SAVE | I_VIEW | I_EDIT)
        );
};

//Inline
inline SoundEvent * SoundComponent::GetSoundEvent(uint32 index) const
{
    DVASSERT(index < (uint32)events.size());
    return events[index].soundEvent;
}

inline uint32 SoundComponent::GetEventsCount() const
{
    return static_cast<uint32>(events.size());
}

inline uint32 SoundComponent::GetSoundEventFlags(uint32 index) const
{
    DVASSERT(index < (uint32)events.size());
    return events[index].flags;
}

inline const Vector3 & SoundComponent::GetLocalDirection(uint32 index) const
{
    DVASSERT(index < (uint32)events.size());
    return events[index].localDirection;
}

};

#endif