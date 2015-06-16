/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Base/BaseTypes.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/Logger.h"
#include "DAVAConfig.h"
#include "Debug/Replay.h"

namespace DAVA 
{

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
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
	clock_gettime( CLOCK_MONOTONIC, &ts );
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
	(void) mach_timebase_info(&timebase);
    
	while(((timebase.numer % 10) == 0) && ((timebase.denom % 10) == 0))
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

    if(delta < 0.001f)
    {
        delta = 0.001f;
    }
    else if(delta > 0.1f)
    {
        delta = 0.1f;
    }

#if defined(__DAVAENGINE_WINDOWS__)

	if (bHighTimerSupport)
	{
		LARGE_INTEGER liCounter;
		QueryPerformanceCounter(&liCounter);

		tLi = liCounter;
	}else
	{
		t0 = (float32)(GetTickCount64() / 1000.0f);
	}

#elif defined (__DAVAENGINE_ANDROID__)

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
	if(frameCount > 60)
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

		return (float32)(((float64)(liCounter.QuadPart - tLi.QuadPart))/(float64)liFrequency.QuadPart);
	}
	else
	{
		float32 currentTime = GetTickCount64() / 1000.0f;
		Logger::FrameworkDebug("delta %f", currentTime - t0);
		return currentTime - t0;
	}
#elif defined(__DAVAENGINE_ANDROID__)
	uint64 currentTime = GetTickCount();
	return (float32)(currentTime - t0) / 1000.f;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	uint64_t current = mach_absolute_time();
	uint64_t elapsed = (current - t0) * timebase.numer / timebase.denom;
	int highestBitSet = 31;
	for (int k = 32; k < 64; ++k)
		if ((elapsed >> k) & 1)
		{
			highestBitSet = k;
		}
	uint32_t elapsed32 = (uint32_t)(elapsed >> (highestBitSet - 31));
	
	float mult = 1.0f;
	for (int c = 0; c < highestBitSet - 31; ++c)
	{
		mult *= 2.0f;
	}
	
	float t2 = (float)elapsed32 * mult / 1000000000.0f ;
	//float t = ((float)(mach_absolute_time() - t0)) * ((float)timebase.numer) / ((float)timebase.denom) / 1000000000.0f;
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
		return (uint64)(((float64)(liCounter.QuadPart))/(float64)liFrequency.QuadPart * 1000000000.);
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

        ::QueryPerformanceCounter( &t );
    
        return uint64(((t.QuadPart)*1000000) / liFrequency.QuadPart);
	}
	else
	{
		return 0;
	}
    
#elif defined(__DAVAENGINE_ANDROID__)

    timespec    ts;

    clock_gettime( CLOCK_REALTIME, &ts );
    //   this gives more correct time, but slow-as-Hell on lots of devices
    //   clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );

    return long(ts.tv_sec*1000000 + ts.tv_nsec/1000);

#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

    return ((mach_absolute_time() * timebase.numer) / timebase.denom)/1000;

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
		return (uint64)(((float64)(liCounter.QuadPart))/(float64)liFrequency.QuadPart * 1000.);
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


//float SystemTimer::FrameDelta()
//{
//}

};
