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


#ifndef __DAVAENGINE_STATS_H__
#define __DAVAENGINE_STATS_H__

#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "FileSystem/File.h"
#include "Base/DynamicObjectCache.h"
#include "Base/FastNameMap.h"
#include "DAVAConfig.h"
//#include "Base/HashMap.h"

namespace DAVA
{

/**
    \brief Class to measure performance in framework code. 
 
    This class can help you to determine the bottlenecks of your code and measure the functions pefromance. 
 
    First step is initialization: 
    \code
    Scene::Scene()
    {
        Stats::Instance()->RegisterEvent("Scene", "Time spend in scene processing");
        Stats::Instance()->RegisterEvent("Scene.Update", "Time spend in draw function");
        Stats::Instance()->RegisterEvent("Scene.Draw", "Time spend in draw function");
    }
    \endcode
    
    Second step is measurement code: 
    \code
    void Scene::Update(float timeElapsed)
    {
        Stats::Instance()->BeginTimeMeasure("Scene.Update", this);
        // .. function code
        Stats::Instance()->EndTimeMeasure("Scene.Update", this);
    }
    
    void Scene::Draw()
    {
        Stats::Instance()->BeginTimeMeasure("Scene.Draw", this);
        // .. function code
        Stats::Instance()->EndTimeMeasure("Scene.Draw", this);
    }   
    \endcode
 
    Third step is initialization of output somewhere in your code: 
    \code
    void GameCore::OnAppStarted()
    {
        // show statistics every 30 frame, to avoid slowdown because of logger messages
        Stats::Instance()->EnableStatsOutputEventNFrame(30);
    }
 
    \endcode
    
 
 */
class ImmediateTimeMeasure
{
public:
    ImmediateTimeMeasure(const FastName & name);
    ~ImmediateTimeMeasure();
    
private:
    FastName name;
    uint64 time;
};
    
class TimeMeasure
{
private:
    struct FunctionMeasure
    {
		FunctionMeasure()
		:	parent(0)
		{
		}
        FastName name;
        uint32 frameCounter;
        uint64 timeStart;
        uint64 timeSpent;
        HashMap<FunctionMeasure *, FunctionMeasure *> children;
        FunctionMeasure * parent;
    };
    
    // Now it should work for single thread, but can be extended to multithreaded time measure
    struct ThreadTimeStamps
    {
        List<FunctionMeasure*> topFunctions;
        HashMap<FastName, FunctionMeasure*> functions;
    };
    
    static ThreadTimeStamps mainThread;

public:
    TimeMeasure(const FastName & blockName);
    ~TimeMeasure();
    
    static void Dump(FunctionMeasure * function = 0, uint32 level = 0);
    static void ClearFunctions();
    
    static TimeMeasure * activeTimeMeasure;
    static FunctionMeasure * lastframeTopFunction;
    
    TimeMeasure * parent;
    FunctionMeasure * function;
    
};
    
class Stats : public StaticSingleton<Stats>
{
public:
    Stats();
    virtual ~Stats();
    
    /**
        \brief Function enables automatic output of measured values to log, every N frames. 
        \param[in] skipFrameCount number of frames we should skip before next debug print to log.
     */
    void EnableStatsOutputEventNFrame(int32 skipFrameCount);

    /**
        \brief System function that is called from ApplicationCore::BeginFrame, to initialize the state of the statistics singleton.
     */
    void BeginFrame();

    /**
        \brief System function that is called from ApplicationCore::EndFrame, to finalize the state of the statistics singleton and output debug information.
     */
    void EndFrame();
    
private:
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    int32 frame;
    int32 skipFrameCount;
#endif
};
    
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
#define TIME_PROFILE(name) static FastName fastNameTime(name); TimeMeasure timeMeasure(fastNameTime);
#define IMM_TIME_PROFILE(name) static FastName fastNameIMM(name); ImmediateTimeMeasure immTimeMeasure(fastNameIMM);
#else
#define TIME_PROFILE(name)
#define IMM_TIME_PROFILE(name)
#endif



};

#endif // __DAVAENGINE_STATS_H__
