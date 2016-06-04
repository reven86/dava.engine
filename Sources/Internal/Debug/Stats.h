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
    ImmediateTimeMeasure(const FastName& name);
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
            : parent(0)
        {
        }
        FastName name;
        uint32 frameCounter;
        uint64 timeStart;
        uint64 timeSpent;
        HashMap<FunctionMeasure*, FunctionMeasure*> children;
        FunctionMeasure* parent;
    };

    // Now it should work for single thread, but can be extended to multithreaded time measure
    struct ThreadTimeStamps
    {
        List<FunctionMeasure*> topFunctions;
        HashMap<FastName, FunctionMeasure*> functions;
    };

    static ThreadTimeStamps mainThread;

public:
    TimeMeasure(const FastName& blockName);
    ~TimeMeasure();

    static void Dump(FunctionMeasure* function = 0, uint32 level = 0);
    static void ClearFunctions();

    static TimeMeasure* activeTimeMeasure;
    static FunctionMeasure* lastframeTopFunction;

    TimeMeasure* parent;
    FunctionMeasure* function;
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
#endif //__DAVAENGINE_ENABLE_DEBUG_STATS__
};
    
    
#if defined(__DAVAENGINE_ENABLE_FRAMEWORK_STATS__)
    #define TIME_PROFILE(name) TimeMeasure timeMeasure(FastName(name));
    #define IMM_TIME_PROFILE(name) ImmediateTimeMeasure immTimeMeasure(FastName(name));
#else
    #define TIME_PROFILE(name)
    #define IMM_TIME_PROFILE(name)
#endif //__DAVAENGINE_ENABLE_FRAMEWORK_STATS__
    
#if defined(__DAVAENGINE_ENABLE_TOOLS_STATS__)
    #define TOOLS_TIME_PROFILE(name) TimeMeasure timeMeasure(FastName(name));
    #define TOOLS_IMM_TIME_PROFILE(name) ImmediateTimeMeasure immTimeMeasure(FastName(name));
#else
    #define TOOLS_TIME_PROFILE(name)
    #define TOOLS_IMM_TIME_PROFILE(name)
#endif //__DAVAENGINE_ENABLE_TOOLS_STATS__
};

#endif // __DAVAENGINE_STATS_H__
