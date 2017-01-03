#include "Time/SystemTimer.h"

#include "Base/Platform.h"
#include "Debug/DVAssert.h"
#include "Debug/Replay.h"

#include <chrono>

#if defined(__DAVAENGINE_APPLE__)
#include <sys/sysctl.h>
#include <sys/time.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <sys/time.h>
// Crystax NDK complains that CLOCK_BOOTTIME is undefined symbol, though it is present in headers
// Google NDK compiles well
#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME 7
#endif
#endif

namespace DAVA
{
int64 SystemTimer::frameTimestamp = 0;
int64 SystemTimer::frameTimestampForRealDelta = 0;
float32 SystemTimer::frameDelta = 0.f;
float32 SystemTimer::realFrameDelta = 0.f;
float32 SystemTimer::globalTime = 0.f;
bool SystemTimer::globalTimePaused = false;
int64 SystemTimer::adjustmentMillis = 0;
int64 SystemTimer::adjustmentMicros = 0;
int64 SystemTimer::adjustmentNanos = 0;

int64 SystemTimer::GetAbsoluteMillis()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() + adjustmentMillis;
}

int64 SystemTimer::GetAbsoluteMicros()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() + adjustmentMicros;
}

int64 SystemTimer::GetAbsoluteNanos()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count() + adjustmentNanos;
}

int64 SystemTimer::GetSystemTime()
{
    using namespace std::chrono;
    return static_cast<int64>(system_clock::to_time_t(system_clock::now()));
}

int64 SystemTimer::GetSystemUptimeMicros()
{
#if defined(__DAVAENGINE_WINDOWS__)
    // Windows provides only milliseconds elapsed from boot
    return static_cast<int64>(::GetTickCount64()) * 1000ll;
#elif defined(__DAVAENGINE_APPLE__)
    // iOS and macOS keep UTC boot timestamp which is updated when machine clock is adjusted.
    // So time elapsed from boot is the difference between current time and boot time.
    timeval bootTime;
    timeval curTime;
    gettimeofday(&curTime, nullptr);

    size_t timevalSize = sizeof(bootTime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    sysctl(mib, 2, &bootTime, &timevalSize, nullptr, 0);

    int64 bootTimeMicro = bootTime.tv_sec * 1000000ll + bootTime.tv_usec;
    int64 curTimeMicro = curTime.tv_sec * 1000000ll + curTime.tv_usec;
    return curTimeMicro - bootTimeMicro;
#elif defined(__DAVAENGINE_ANDROID__)
    timespec bootTime;
    clock_gettime(CLOCK_BOOTTIME, &bootTime);
    return bootTime.tv_sec * 1000000ll + bootTime.tv_nsec / 1000ll;
#else
#error "SystemTimer: unknown platform"
#endif
}

int64 SystemTimer::GetFrameTimestamp()
{
    return frameTimestamp;
}

float32 SystemTimer::GetFrameDelta()
{
    return frameDelta;
}

float32 SystemTimer::GetRealFrameDelta()
{
    return realFrameDelta;
}

void SystemTimer::SetFrameDelta(float32 delta)
{
    DVASSERT(Replay::IsPlayback() || Replay::IsRecord());
    frameDelta = delta;
}

float32 SystemTimer::GetGlobalTime()
{
    return globalTime;
}

void SystemTimer::UpdateGlobalTime(float32 timeElapsed)
{
    if (!globalTimePaused)
    {
        globalTime += timeElapsed;
    }
}

void SystemTimer::ResetGlobalTime()
{
    globalTime = 0.f;
}

void SystemTimer::PauseGlobalTime()
{
    globalTimePaused = true;
}

void SystemTimer::ResumeGlobalTime()
{
    globalTimePaused = false;
}

void SystemTimer::StartFrame()
{
    if (frameTimestamp == 0)
    {
        frameTimestamp = GetAbsoluteMillis();
        frameTimestampForRealDelta = frameTimestamp;
    }

    int64 timestamp = GetAbsoluteMillis();
    frameDelta = static_cast<float32>((timestamp - frameTimestamp) / 1000.0);
    realFrameDelta = frameDelta;
    frameDelta = std::min(0.1f, std::max(0.001f, frameDelta));

    frameTimestamp = timestamp;
}

void SystemTimer::ComputeRealFrameDelta()
{
    realFrameDelta = static_cast<float32>((frameTimestamp - frameTimestampForRealDelta) / 1000.0);
    frameTimestampForRealDelta = frameTimestamp;
}

void SystemTimer::Adjust(int64 micros)
{
    /*
        On several platforms GetAbsoluteMillis() and friends may stop after some amount of time
        when device enters sleep mode (screen is darkened, power cable is not connected).
        To ensure clock monotonicity SystemTimer uses time adjustment which tells how long
        device has spent in deep sleep.
        Plaform backend knows how to compute time spent is deep sleep and indirectly calls 
        SystemTimer::Adjust.

        Timer behavior without adjustment:
          active |      sleep      | again active
        1  2  3  4  5  6  6  6  6  7  8  9  10          <--- timer without adjustment
        1  2  3  4  5  6  6  6  6  10 11 12 13          <--- timer with adjustment
                                   ^
                                  / \
                                   |
                           here adjust timer
                               by 6 ticks

        Device spent 6 ticks in deep sleep and timer continues from value when clock has stopped.
        Without adjustment real frame delta will be 3 tick (defference between 7 and 4) instead of 6.
        Platform impementation can measure time spent in deep sleep and adjust SystemTimer by 6 ticks
        and real frame delta will be real 6 ticks.
    */
    // Precompute adjustments for clocks with various precision
    adjustmentMicros += micros;
    adjustmentMillis += micros / 1000;
    adjustmentNanos += micros * 1000;
}

} // namespace DAVA
