#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class StreamDelegate
{
public:
    virtual void PcmDataCallback(uint8* data, uint32 datalen) = 0;
};

class SoundStream
{
public:
    virtual ~SoundStream() = default;

    static SoundStream* Create(StreamDelegate* streamDelegate, uint32 channelsCount);

    virtual void Play()
    {
    }
    virtual void Pause()
    {
    }
    static uint32 GetDefaultSampleRate();
};
}