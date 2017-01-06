#include "Base/BaseTypes.h"

class FpsMeter
{
public:
    explicit FpsMeter(DAVA::float32 duration)
        : measureDurationSec(duration)
    {
    }

    void Update(DAVA::float32 timeElapsed)
    {
        ++elapsedFrames;
        elapsedSec += timeElapsed;

        if (elapsedSec > measureDurationSec)
        {
            lastFps = elapsedFrames / elapsedSec;
            fpsIsReady = true;
            elapsedSec = 0;
            elapsedFrames = 0;
        }
        else
        {
            fpsIsReady = false;
        }
    }

    bool IsFpsReady() const
    {
        return fpsIsReady;
    }

    DAVA::float32 GetFps() const
    {
        return lastFps;
    }

private:
    DAVA::float32 measureDurationSec = 0.f;
    DAVA::float32 elapsedSec = 0.f;
    DAVA::uint32 elapsedFrames = 0;
    DAVA::float32 lastFps = 0.f;
    bool fpsIsReady = false;
};
