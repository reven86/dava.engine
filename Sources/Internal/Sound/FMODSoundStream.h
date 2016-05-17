#pragma once

#include "SoundStream.h"
#include "Sound/FMODUtils.h"
#include "fmod.h"

namespace DAVA
{
class FMODSoundStream : public SoundStream
{
public:
    FMODSoundStream() = delete;
    FMODSoundStream(StreamDelegate* streamDelegate, uint32 channelsCount);

    ~FMODSoundStream();

    bool Init();

    // SoundStream interface implementation
    void Play() override
    {
        if (fmodChannel)
        {
            fmodChannel->setPaused(false);
        }
    }

    void Pause() override
    {
        if (fmodChannel)
        {
            fmodChannel->setPaused(true);
        }
    }

private:
    static FMOD_RESULT F_CALLBACK PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);

    FMOD_CREATESOUNDEXINFO exinfo;
    FMOD::Sound* sound = nullptr;
    FMOD::Channel* fmodChannel = nullptr;
    StreamDelegate* dataSender = nullptr;
};
}