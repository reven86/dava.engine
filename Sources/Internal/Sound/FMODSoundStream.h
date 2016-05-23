#pragma once

#include "SoundStream.h"
#include "Sound/FMODUtils.h"
#include "fmod.h"

namespace DAVA
{
class FMODSoundStream : public SoundStream
{
public:
    FMODSoundStream(SoundStreamDelegate* streamDelegate, uint32 channelsCount);

    ~FMODSoundStream();

    bool Init(FMOD::System* system);

    // SoundStream interface implementation
    void Play() override;
    void Pause() override;

private:
    static FMOD_RESULT F_CALLBACK PcmReadDecodeCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);

    FMOD_CREATESOUNDEXINFO exinfo;
    FMOD::Sound* sound = nullptr;
    FMOD::Channel* fmodChannel = nullptr;
    SoundStreamDelegate* dataSender = nullptr;
};
}