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


#ifndef __DAVAENGINE_SYSTEM_TIMER__
#define __DAVAENGINE_SYSTEM_TIMER__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA 
{

class SystemTimer : public Singleton<SystemTimer> 
{
    friend class Core;
    
#if defined(__DAVAENGINE_WINDOWS__)
	LARGE_INTEGER	liFrequency;
	LARGE_INTEGER	tLi;
	BOOL			bHighTimerSupport;
	float32			t0;
#elif defined (__DAVAENGINE_ANDROID__)
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
