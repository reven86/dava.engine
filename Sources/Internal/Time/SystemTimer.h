#pragma once

#if 1

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
}

/**
    \ingroup timers
    SystemTimer provides collection of methods that track time.

    SystemTimer is a static class and its methods can be used even without `Engine` initialization. But I believe
    it's clear that methods returning frame deltas or working with global time make sense only in game loop.

    Methods `GetAbsoluteMillis`, `GetAbsoluteMicros` and `GetAbsoluteNanos` return value of monotonic clock of
    corresponding precision. Monotonic clock always increases and is not related to wall clock and can be used for
    measuring time intervals. Also these methods counter time spent when device was in deep sleep.
*/
class SystemTimer final
{
    friend class Private::EngineBackend;
#if !defined(__DAVAENGINE_COREV2__)
    friend class Core;
#endif

public:
    /** Get monotonic clock value in milliseconds, including time spent in deep sleep. */
    static int64 GetAbsoluteMillis();

    /** Get monotonic clock value in microseconds, including time spent in deep sleep. */
    static int64 GetAbsoluteMicros();

    /** Get monotonic clock value in nanoseconds, including time spent in deep sleep. */
    static int64 GetAbsoluteNanos();

    /** Get current calendar time in seconds from epoch beginning 1970-01-01 00:00 UTC. */
    static int64 GetSystemTime();

    /** Get number of microseconds elapsed from system boot, including time spent in deep sleep. */
    static int64 GetSystemUptimeMicros();

    /**
        Get current frame timestamp in milliseconds.

        Frame timestamp is a result of `GetAbsoluteMillis()` call at the beginning of a frame.
    */
    static int64 GetFrameTimestamp();

    /**
        Get clamped time difference in seconds between current frame timestamp and previous frame timestamp.

        The difference is clamped to the range [0.001, 0.1].
        Application can modify delta for current frame by `SetFrameDelta` method.
    */
    static float32 GetFrameDelta();

    /**
        Get real time difference in seconds between current frame timestamp and previous frame timestamp.

        Real frame delta is not computed when application is suspended due to hardware limitations.
        Application is considered in suspended state after receiving `Engine::suspended` signal and before
        receiving `Engine::resumed` signal.
        Dava.engine ensures that real frame delta also includes time spent in suspended state.
    */
    static float32 GetRealFrameDelta();

    /** Modify frame delta for current frame: used mainly in replays. */
    static void SetFrameDelta(float32 delta);

    static float32 GetGlobalTime();
    static void UpdateGlobalTime(float32 timeElapsed);
    static void ResetGlobalTime();
    static void PauseGlobalTime();
    static void ResumeGlobalTime();

private:
    static void StartFrame();
    static void ComputeRealFrameDelta();
    static void Adjust(int64 micros);

    static int64 frameTimestamp;
    static int64 frameTimestampForRealDelta;
    static float32 frameDelta;
    static float32 realFrameDelta;

    static float32 globalTime;
    static bool globalTimePaused;

    static int64 adjustmentMillis;
    static int64 adjustmentMicros;
    static int64 adjustmentNanos;
};

} // namespace DAVA

#else

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Platform.h"

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
    uint64 GetSystemTime(); // seconds since 00:00 hours, Jan 1, 1970

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

#endif
