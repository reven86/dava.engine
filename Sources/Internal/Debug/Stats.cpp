#include "Core/Core.h"
#include "Concurrency/Thread.h"
#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
// Immediate Time measure class

ImmediateTimeMeasure::ImmediateTimeMeasure(const FastName& _name)
{
    name = _name;
    time = SystemTimer::Instance()->GetAbsoluteNano();
}

ImmediateTimeMeasure::~ImmediateTimeMeasure()
{
    time = SystemTimer::Instance()->GetAbsoluteNano() - time;
    Logger::Info("%s %0.9f seconds", name.c_str(), double(time / 1e+9));
}

// TimeMeasure class
TimeMeasure* TimeMeasure::activeTimeMeasure = 0;
TimeMeasure::FunctionMeasure* TimeMeasure::lastframeTopFunction = 0;
TimeMeasure::ThreadTimeStamps TimeMeasure::mainThread;

TimeMeasure::TimeMeasure(const FastName& blockName)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (!Thread::IsMainThread())
        return;

    function = mainThread.functions.at(blockName);
    if (!function)
    {
        FunctionMeasure* newFunctionMeasure = new FunctionMeasure();
        mainThread.functions.insert(blockName, newFunctionMeasure);
        function = newFunctionMeasure;
        function->name = blockName;
    }
    if (lastframeTopFunction == 0)
        lastframeTopFunction = function;

    parent = activeTimeMeasure;

    if (parent && function->parent == 0)
    {
        parent->function->children.insert(function, function);
        function->parent = parent->function;
    }

    if (activeTimeMeasure == 0)
    {
        mainThread.topFunctions.push_back(function);
    }

    function->timeStart = SystemTimer::Instance()->GetAbsoluteNano();
    activeTimeMeasure = this;
#endif
}

TimeMeasure::~TimeMeasure()
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (!Thread::IsMainThread())
        return;

    uint32 frameCounter = Core::Instance()->GetGlobalFrameIndex();
    if (frameCounter == function->frameCounter)
    {
        function->timeSpent += SystemTimer::Instance()->GetAbsoluteNano() - function->timeStart;
    }
    else
    {
        function->timeSpent = SystemTimer::Instance()->GetAbsoluteNano() - function->timeStart;
        function->frameCounter = frameCounter;
    }
    activeTimeMeasure = parent;
#endif
}

void TimeMeasure::ClearFunctions()
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    mainThread.topFunctions.clear();
#endif
}

void TimeMeasure::Dump(FunctionMeasure* function, uint32 level)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (level == 0)
    {
        if (!mainThread.topFunctions.empty())
        {
            Logger::Info("Stats for frame: %d", Core::Instance()->GetGlobalFrameIndex());

            for (auto function : mainThread.topFunctions)
            {
                if (function->frameCounter == Core::Instance()->GetGlobalFrameIndex())
                    Dump(function, level + 1);
            }
        }
    }
    else
    {
        Logger::Info("%s %s %0.9llf seconds", GetIndentString('-', level + 1).c_str(), function->name.c_str(), (double)function->timeSpent / 1e+9);
        for (auto childFunction : function->children)
        {
            if (childFunction.second->frameCounter == Core::Instance()->GetGlobalFrameIndex())
                Dump(childFunction.first, level + 1);
        }
    }
#endif
}

Stats::Stats()
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    skipFrameCount = -1;
    frame = 0;
#endif
}

Stats::~Stats()
{
}

void Stats::EnableStatsOutputEventNFrame(int32 _skipFrameCount)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    skipFrameCount = _skipFrameCount;
#endif
}

void Stats::BeginFrame()
{
    TimeMeasure::ClearFunctions();
}

void Stats::EndFrame()
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (skipFrameCount != -1)
    {
        if (frame > skipFrameCount)
        {
            TimeMeasure::Dump();
            frame = 0;
        }
    }
    frame++;
#endif
}
};
