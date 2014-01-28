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

#ifndef __DAVAENGINE_SOUND_H__
#define __DAVAENGINE_SOUND_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Sound/VolumeAnimatedObject.h"

#if defined(__DAVAENGINE_HTML5__)
#include <SDL/SDL_Mixer.h>
#endif

namespace FMOD
{
class Sound;
class ChannelGroup;
class Channel;
};

namespace DAVA
{

#if defined(__DAVAENGINE_HTML5__)
class SoundGroup;
#endif
    
class Sound : public VolumeAnimatedObject
{
public:

	enum eType
	{
		TYPE_STATIC = 0,
		TYPE_STREAMED
	};

    virtual int32 Release();

	static Sound * Create(const FilePath & fileName, eType type, const FastName & groupName, int32 priority = 128);
	static Sound * Create3D(const FilePath & fileName, eType type, const FastName & groupName, int32 priority = 128);

	void SetVolume(float32 volume);
	float32	GetVolume();

	void Play(const Message & msg = Message());
	void Pause(bool isPaused);
	bool IsPaused();
	void Stop();
#if defined(__DAVAENGINE_HTML5__)
    void PerformCallback();
#else
    void PerformCallback(FMOD::Channel * instance);
#endif

	void SetPosition(const Vector3 & position);
	void UpdateInstancesPosition();

	void SetLoopCount(int32 looping); // -1 = infinity
	int32 GetLoopCount() const;

	eType GetType() const;
    
#if defined(__DAVAENGINE_HTML5__)
    static Map<int, Sound*> channelsUserData;
    static Map<int, Message> callbacksMap;
    static int s_nFreeSoundID;
#endif

private:
	Sound(const FilePath & fileName, eType type, int32 priority);
	~Sound();

	static Sound * CreateWithFlags(const FilePath & fileName, eType type, const FastName & groupName, int32 addFlags, int32 priority = 128);

	void SetSoundGroup(const FastName & groupName);

	bool is3d;
	Vector3 position;

	FilePath fileName;
	eType type;
	int32 priority;
    
#if defined(__DAVAENGINE_HTML5__)
    Mix_Chunk *soundChunk;
    int nChannelID;
    float fVolume;
    int nLoopCount;
    int nPlayedCallbacks;
    int nSoundID;
    SoundGroup *soundGroup;
#endif

	FMOD::Sound * fmodSound;
	FMOD::ChannelGroup * fmodInstanceGroup;

    uint8 * soundData;

    Map<FMOD::Channel *, Message> callbacks;
};

};

#endif //__DAVAENGINE_SOUND_H__