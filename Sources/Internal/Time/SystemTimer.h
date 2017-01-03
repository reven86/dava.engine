#pragma once

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
