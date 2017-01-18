#include "Utils/FpsMeter.h"

namespace DAVA
{
FpsMeter::FpsMeter(DAVA::float32 duration)
    : measureDurationSec(duration)
{
}

void FpsMeter::Update(DAVA::float32 timeElapsed)
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
}
