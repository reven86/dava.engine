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

    dataSender = nullptr;
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
