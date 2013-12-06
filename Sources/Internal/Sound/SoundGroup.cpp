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



#include "Sound/SoundGroup.h"
#include "Sound/Sound.h"
#include "Animation/LinearAnimation.h"
#include "Sound/FMODUtils.h"
#include "Sound/SoundSystem.h"

namespace DAVA
{
    
#if defined(__DAVAENGINE_HTML5__)
    int SoundGroup::nFreeGroupID = 1;
#endif

SoundGroup::SoundGroup()
{
#if defined(__DAVAENGINE_HTML5__)
    nGroupID = nFreeGroupID++;
#else
	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createSoundGroup(0, &fmodSoundGroup));
#endif
}

SoundGroup::~SoundGroup()
{
#if defined(__DAVAENGINE_HTML5__)
    sounds.clear();
#else
	FMOD_VERIFY(fmodSoundGroup->release());
#endif
}

void SoundGroup::SetVolume(float32 volume)
{
#if defined(__DAVAENGINE_HTML5__)
    fVolume = volume;
    Set<Sound*>::iterator it = sounds.begin();
    for(Set<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it)
    {
        (*it)->SetVolume(volume);
    }
#else
	FMOD_VERIFY(fmodSoundGroup->setVolume(volume));
#endif
}

float32 SoundGroup::GetVolume()
{
	float32 volume;
#if defined(__DAVAENGINE_HTML5__)
    volume = fVolume;
#else
	FMOD_VERIFY(fmodSoundGroup->getVolume(&volume));
#endif
	return volume;
}

void SoundGroup::Stop()
{
#if defined(__DAVAENGINE_HTML5__)
    Set<Sound*>::iterator it = sounds.begin();
    for(Set<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it)
    {
        (*it)->Stop();
    }
#else
	FMOD_VERIFY(fmodSoundGroup->stop());
#endif
}

};