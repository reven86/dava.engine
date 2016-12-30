#if 1

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
// Crystax NDK complains that CLOCK_BOOTTIME is undefined symbol, through it is present in headers
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

#else

#include <chrono>
#include "Base/BaseTypes.h"
#include "Base/Platform.h"
#include "Time/SystemTimer.h"
#include "Logger/Logger.h"
#include "DAVAConfig.h"
#include "Debug/Replay.h"

#if defined(__DAVAENGINE_APPLE__)
#include "mach/mach_time.h"
#endif

namespace DAVA
{

#if defined(__DAVAENGINE_APPLE__)
static mach_timebase_info_data_t timebase;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

float SystemTimer::realFrameDelta = 0;
float SystemTimer::delta = 0;
uint64 SystemTimer::stampTime = 0;

#ifdef SHOW_FRAME_TIME
static uint64 startTime;
static uint64 curTime;
static int frameCount;
#endif //#ifdef SHOW_FRAME_TIME


#if defined(__DAVAENGINE_ANDROID__)
uint64 SystemTimer::GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec / 1000000 + ts.tv_sec * 1000;
}
#endif //#if defined(__DAVAENGINE_ANDROID__)

SystemTimer::SystemTimer()
{
    globalTime = 0.0f;
    pauseMultiplier = 1.0f;
#if defined(__DAVAENGINE_WINDOWS__)
    t0 = (float32)(GetTickCount64() / 1000.0f);
    QueryPerformanceCounter(&tLi);
    bHighTimerSupport = QueryPerformanceFrequency(&liFrequency);
    if (bHighTimerSupport)
    {
        Logger::FrameworkDebug("[SystemTimer] High frequency timer support enabled\n");
    }
#elif defined(__DAVAENGINE_ANDROID__)
    t0 = GetTickCount();
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    (void)mach_timebase_info(&timebase);

    while (((timebase.numer % 10) == 0) && ((timebase.denom % 10) == 0))
    {
        timebase.numer /= 10;
        timebase.denom /= 10;
    }

    t0 = mach_absolute_time();
#else //PLATFORMS
//other platorfms
#endif //PLATFORMS

#ifdef SHOW_FRAME_TIME
    curTime = startTime = AbsoluteMS();
    frameCount = 0;
#endif //#ifdef SHOW_FRAME_TIME
}

SystemTimer::~SystemTimer()
{
}

void SystemTimer::Start()
{
    realFrameDelta = ElapsedSec();
    delta = realFrameDelta;

    if (delta < 0.001f)
    {
        delta = 0.001f;
    }
    else if (delta > 0.1f)
    {
        delta = 0.1f;
    }

#if defined(__DAVAENGINE_WINDOWS__)

    if (bHighTimerSupport)
    {
        LARGE_INTEGER liCounter;
        QueryPerformanceCounter(&liCounter);

        tLi = liCounter;
    }
    else
    {
        t0 = (float32)(GetTickCount64() / 1000.0f);
    }

#elif defined(__DAVAENGINE_ANDROID__)

    t0 = GetTickCount();

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

    t0 = mach_absolute_time();	

#else //PLATFORMS
//other platforms
#endif //PLATFORMS

    stampTime = AbsoluteMS();
#ifdef SHOW_FRAME_TIME
    curTime = stampTime;
    frameCount++;
    if (frameCount > 60)
    {
        Logger::FrameworkDebug("frame time = %dms", (curTime - startTime) / frameCount);
        startTime = curTime;
        frameCount = 0;
    }
#endif //#ifdef SHOW_FRAME_TIME
}

float32 SystemTimer::ElapsedSec()
{
#if defined(__DAVAENGINE_WINDOWS__)
    if (bHighTimerSupport)
    {
        LARGE_INTEGER liCounter;
        QueryPerformanceCounter(&liCounter);

        return (float32)(((float64)(liCounter.QuadPart - tLi.QuadPart)) / (float64)liFrequency.QuadPart);
    }
    else
    {
        float32 currentTime = GetTickCount64() / 1000.0f;
        Logger::FrameworkDebug("delta %f", currentTime - t0);
        return currentTime - t0;
    }
#elif defined(__DAVAENGINE_ANDROID__)
    uint64 currentTime = GetTickCount();
    return static_cast<float32>(currentTime - t0) / 1000.f;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    uint64 current = mach_absolute_time();
    uint64 elapsed = (current - t0) * timebase.numer / timebase.denom;
    int32 highestBitSet = 31;
    for (int32 k = 32; k < 64; ++k)
        if ((elapsed >> k) & 1)
        {
            highestBitSet = k;
        }
    uint32 elapsed32 = static_cast<uint32>(elapsed >> (highestBitSet - 31));

    float32 mult = 1.0f;
    for (int c = 0; c < highestBitSet - 31; ++c)
    {
        mult *= 2.0f;
    }

    float32 t2 = static_cast<float32>(elapsed32) * mult / 1000000000.0f;
    return t2;
#else //PLATFORMS
    //other platforms
    return 0;
#endif //PLATFORMS
}

uint64 SystemTimer::GetAbsoluteNano()
{
#if defined(__DAVAENGINE_WINDOWS__)
    if (bHighTimerSupport)
    {
        LARGE_INTEGER liCounter;
        QueryPerformanceCounter(&liCounter);
        return (uint64)(((float64)(liCounter.QuadPart)) / (float64)liFrequency.QuadPart * 1000000000.);
    }
    else
    {
        return 0;
    }
    
#elif defined(__DAVAENGINE_ANDROID__)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000 + now.tv_nsec;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    uint64_t numer = timebase.numer;
    uint64_t denom = timebase.denom;
    uint64_t elapsed = mach_absolute_time();
    elapsed *= numer;
    elapsed /= denom;
    return elapsed;
#else //PLATFORMS
    //other plaforms
    return 0;
#endif //PLATFORMS
}

uint64 SystemTimer::GetAbsoluteUs()
{
#if defined(__DAVAENGINE_WINDOWS__)
    if (bHighTimerSupport)
    {
        LARGE_INTEGER t;

        ::QueryPerformanceCounter(&t);

        return uint64(((t.QuadPart) * 1000000) / liFrequency.QuadPart);
    }
    else
    {
        return 0;
    }
    
#elif defined(__DAVAENGINE_ANDROID__)

    timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    //   this gives more correct time, but slow-as-Hell on lots of devices
    //   clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );

    return long(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

    return ((mach_absolute_time() * timebase.numer) / timebase.denom) / 1000;

#else //PLATFORMS
    //other plaforms
    return 0;
#endif //PLATFORMS
}

uint64 SystemTimer::AbsoluteMS()
{
#if defined(__DAVAENGINE_WINDOWS__)
    if (bHighTimerSupport)
    {
        LARGE_INTEGER liCounter;
        QueryPerformanceCounter(&liCounter);
        return (uint64)(((float64)(liCounter.QuadPart)) / (float64)liFrequency.QuadPart * 1000.);
    }
    else
    {
        return 0;
    }

#elif defined(__DAVAENGINE_ANDROID__)
    return GetTickCount();
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    uint64_t numer = timebase.numer;
    uint64_t denom = timebase.denom;
    uint64_t elapsed = mach_absolute_time();
    elapsed *= numer;
    elapsed /= denom;
    return elapsed / 1000000;
#else //PLATFORMS
    //other plaforms
    return 0;
#endif //PLATFORMS
}

void SystemTimer::SetFrameDelta(float32 _delta)
{
    DVASSERT(Replay::IsPlayback() || Replay::IsRecord());
    delta = _delta;
}

uint64 SystemTimer::GetSystemTime()
{
    auto sysTime = std::chrono::system_clock::now();
    time_t sysTimeT = std::chrono::system_clock::to_time_t(sysTime);
    return static_cast<uint64>(sysTimeT);
}

//float SystemTimer::FrameDelta()
//{
//}
};

#endif
