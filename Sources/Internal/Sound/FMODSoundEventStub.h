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

#ifdef SOUND_STUB 

#ifndef __DAVAENGINE_FMOD_SOUND_EVENT_STUB_H__
#define __DAVAENGINE_FMOD_SOUND_EVENT_STUB_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/EventDispatcher.h"
#include "Base/FastNameMap.h"
#include "Sound/SoundEvent.h"




namespace DAVA
{

class STUBSoundEvent : public SoundEvent
{
public:

	virtual ~STUBSoundEvent(){};
	STUBSoundEvent(){};
    virtual bool IsActive() const { return false;} ;
    virtual bool Trigger(){return false;};
	virtual void Stop(){};
    virtual void SetPaused(bool paused){};
    
    virtual void SetVolume(float32 volume){};
    
    virtual void SetPosition(const Vector3 & position){};
    virtual void SetDirection(const Vector3 & direction){};
    virtual void UpdateInstancesPosition(){};
    virtual void SetVelocity(const Vector3 & velocity){};
    
    virtual void SetParameterValue(const FastName & paramName, float32 value){};
    virtual float32 GetParameterValue(const FastName & paramName){ return 0.0;};
    virtual bool IsParameterExists(const FastName & paramName){ return false;};

    virtual void GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo)const{};

    virtual String GetEventName()const { return "";};
    
protected:
    STUBSoundEvent(const FastName & eventName){};

    void InitParamsMap();



	bool is3D;
    FastName eventName;
    Vector3 position;
    Vector3 direction;

	FastNameMap<float32> paramsValues;

    
friend class SoundSystem;
};

};

#endif

#endif
