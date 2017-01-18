#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    FpsMeter is a simple class for measuring FPS.
 
    Right way to use this class:
    1. create instance of FpsMeter. Say, FpsMeter fpsMeter;
    2. call fpsMeter.Update on every update from engine and pass elapsedTime from engine's update to it.
    3. check fpsMeter.IsFpsReady() after each fpsMeter.Update call. If functions returns true, you may get measured FPS through fpsMeter.GetFps() call.
    
    FpsMeter works permanently, which means that after evaluting FPS value it begins to evaluate next FPS value
 
    For example:
 
    class A
    {
        A()
        {
            engine.update.Connect(this, &A::Update);
        }
 
        void Update(float32 elapsedSec)
        {
            fm.Update(elapsedSec);
            if (fm.IsFpsReady())
            {
                float32 currentFps = fm.GetFps();
                ...
            }
        }
 
        FpsMeter fm;
    }
*/

class FpsMeter
{
public:
    /** creates FpsMeter instance which will be calculating FPS values each `duration` seconds */
    explicit FpsMeter(DAVA::float32 duration = 1.f);

    /** 
    passes time in seconds since last Update call.
    Normally this function should be invoked on each update tick from engine.
    */
    void Update(DAVA::float32 timeElapsed);

    /** returns true if next FPS value is ready */
    bool IsFpsReady() const;

    /** returns last measured FPS value */
    DAVA::float32 GetFps() const;

private:
    DAVA::float32 measureDurationSec = 0.f;
    DAVA::float32 elapsedSec = 0.f;
    DAVA::uint32 elapsedFrames = 0;
    DAVA::float32 lastFps = 0.f;
    bool fpsIsReady = false;
};

inline bool FpsMeter::IsFpsReady() const
{
    return fpsIsReady;
}

inline DAVA::float32 FpsMeter::GetFps() const
{
    return lastFps;
}
}
