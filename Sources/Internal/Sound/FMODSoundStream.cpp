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

#include "FMODSoundStream.h"
#include "SoundStream.h"
#include "Sound/FMODSoundStream.h"
#include "Sound/SoundSystem.h"


#ifdef DAVA_FMOD

namespace DAVA
{
SoundStream* SoundStream::Create(StreamDelegate* streamDelegate, uint32 channelsCount)
{
    FMODSoundStream* fmodStream = new FMODSoundStream(streamDelegate, channelsCount);
    bool isInited = fmodStream->Init();
    if (!isInited)
    {
        SafeDelete(fmodStream);
    }

    return fmodStream;
}

uint32 SoundStream::GetDefaultSampleRate()
{
    static const uint32 outSampleRate = 44100;
    return outSampleRate;
}

FMODSoundStream::~FMODSoundStream()
{
    if (fmodChannel)
    {
        fmodChannel->stop();
        fmodChannel = nullptr;
    }
    if (sound)
    {
        sound->release();
        sound = nullptr;
    }

    DataSender = nullptr;
}

FMOD_RESULT F_CALLBACK FMODSoundStream::PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
{
    Memset(data, 0, datalen);

    if (nullptr == sound)
    {
        return FMOD_OK;
    }

    // Read from your buffer here...
    void* soundData;
    reinterpret_cast<FMOD::Sound*>(sound)->getUserData(&soundData);

    StreamDelegate* streamDelegate = reinterpret_cast<StreamDelegate*>(soundData);
    if (nullptr != streamDelegate)
    {
        streamDelegate->PcmDataCallback(reinterpret_cast<uint8*>(data), static_cast<uint32>(datalen));
    }
    return FMOD_OK;
}

FMODSoundStream::FMODSoundStream(StreamDelegate* streamDelegate, uint32 channelsCount)
    : dataSender(streamDelegate)
{
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

    int sampleRate = static_cast<int>(SoundStream::GetDefaultSampleRate());
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO); /* required. */
    exinfo.length = sampleRate * channelsCount * sizeof(signed short) * 5; /* Length of PCM data in bytes of whole song (for Sound::getLength) */
    exinfo.numchannels = channelsCount; /* Number of channels in the sound. */
    exinfo.defaultfrequency = sampleRate; /* Default playback rate of sound. */
    exinfo.format = FMOD_SOUND_FORMAT_PCM16; /* Data format of sound. */
    exinfo.pcmreadcallback = FMODSoundStream::PcmReadDecodeCallback; /* User callback for reading. */
}

bool FMODSoundStream::Init()
{
    FMOD::System* system = SoundSystem::Instance()->GetFmodSystem();
    FMOD_RESULT result = system->createStream(nullptr, FMOD_OPENUSER, &exinfo, &sound);
    sound->setUserData(dataSender);

    FMOD_RESULT res = system->playSound(FMOD_CHANNEL_FREE, sound, true, &fmodChannel);
    if (FMOD_OK == res && fmodChannel)
    {
        fmodChannel->setLoopCount(-1);
        fmodChannel->setMode(FMOD_LOOP_NORMAL | FMOD_NONBLOCKING);
        fmodChannel->setPosition(0, FMOD_TIMEUNIT_MS); // this flushes the buffer to ensure the loop mode takes effect
        return true;
    }
    return false;
}
}

#endif
