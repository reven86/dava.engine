#ifndef __DAVAENGINE_SYSTEM_TIMER__
#define __DAVAENGINE_SYSTEM_TIMER__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
}

class SystemTimer : public Singleton<SystemTimer>
{
#if !defined(__DAVAENGINE_COREV2__)
    friend class Core;
#endif
    friend class Private::EngineBackend;
    
#if defined(__DAVAENGINE_WINDOWS__)
    LARGE_INTEGER liFrequency;
    LARGE_INTEGER tLi;
    BOOL bHighTimerSupport;
    float32 t0;
#elif defined(__DAVAENGINE_ANDROID__)
    uint64 t0;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    uint64_t t0;
#else //PLATFORMS
//other platforms
#endif //PLATFORMS

    static float realFrameDelta;

    //frame delta clamped between 0.001f and 0.1f
    static float delta;
    static uint64 stampTime;

    float32 ElapsedSec();
    void Start();

public:
    SystemTimer();
    virtual ~SystemTimer();

    uint64 AbsoluteMS();
    uint64 GetAbsoluteNano();
    uint64 GetAbsoluteUs();

    static void SetFrameDelta(float32 _delta); //for replay playback only

    //returns frame delta clamped between 0.001f and 0.1f
    static float FrameDelta()
    {
        return delta;
    }

    static uint64 FrameStampTimeMS()
    {
        return stampTime;
    }

    //use it if you need synchronization with server
    static float RealFrameDelta()
    {
        return realFrameDelta;
    }

    // Global time is something that can be used by game

    inline void ResetGlobalTime();
    inline void UpdateGlobalTime(float32 timeElapsed);
    inline float32 GetGlobalTime();
    inline void PauseGlobalTime(bool isPaused);
    
    
#if defined(__DAVAENGINE_ANDROID__)
    uint64 GetTickCount();
#endif //#if defined(__DAVAENGINE_ANDROID__)

private:
    float32 globalTime;
    float32 pauseMultiplier;
};

// Inline functions
inline void SystemTimer::ResetGlobalTime()
{
    globalTime = 0.0f;
}

inline void SystemTimer::UpdateGlobalTime(float32 timeElapsed)
{
    globalTime += timeElapsed * pauseMultiplier;
}

inline float32 SystemTimer::GetGlobalTime()
{
    return globalTime;
}

inline void SystemTimer::PauseGlobalTime(bool isPaused)
{
    if (isPaused)
        pauseMultiplier = 0.0f;
    else
        pauseMultiplier = 1.0f;
}
};

#endif // #ifndef __DAVAENGINE_SYSTEM_TIMER__
