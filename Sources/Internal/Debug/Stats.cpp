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


#include "Core/Core.h"
#include "Concurrency/Thread.h"
#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
// Immediate Time measure class
    
ImmediateTimeMeasure::ImmediateTimeMeasure(const FastName & _name)
{
    name = _name;
    time = SystemTimer::Instance()->GetAbsoluteNano();
}
    
ImmediateTimeMeasure::~ImmediateTimeMeasure()
{
    time = SystemTimer::Instance()->GetAbsoluteNano() - time;
    Logger::Info("%s %0.9llf seconds", name.c_str(), (double)time / 1e+9);
}
    
// TimeMeasure class
TimeMeasure * TimeMeasure::activeTimeMeasure = 0;
TimeMeasure::FunctionMeasure * TimeMeasure::lastframeTopFunction = 0;
TimeMeasure::ThreadTimeStamps TimeMeasure::mainThread;

    
TimeMeasure::TimeMeasure(const FastName & blockName)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (!Thread::IsMainThread())return;
    
    function = mainThread.functions.at(blockName);
    if (!function)
    {
        FunctionMeasure * newFunctionMeasure = new FunctionMeasure();
        mainThread.functions.insert(blockName, newFunctionMeasure);
        function = newFunctionMeasure;
        function->name = blockName;
    }
    if (lastframeTopFunction == 0)
        lastframeTopFunction = function;
    
    parent = activeTimeMeasure;
    
    if (parent &&  function->parent == 0)
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
    if (!Thread::IsMainThread())return;

    uint32 frameCounter = Core::Instance()->GetGlobalFrameIndex();
    if (frameCounter == function->frameCounter)
    {
        function->timeSpent += SystemTimer::Instance()->GetAbsoluteNano() - function->timeStart;
    }else
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
    
void TimeMeasure::Dump(FunctionMeasure * function, uint32 level)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    if (level == 0)
    {
		Logger::Info("Stats for frame: %d", Core::Instance()->GetGlobalFrameIndex());

		for (List<FunctionMeasure*>::iterator it = mainThread.topFunctions.begin(); it != mainThread.topFunctions.end(); ++it)
		{
			FunctionMeasure * function = *it;
			if (function->frameCounter == Core::Instance()->GetGlobalFrameIndex())
				Dump(function, level + 1);
		}
	}
	else
    {
        Logger::Info("%s %s %0.9llf seconds", GetIndentString('-', level + 1).c_str(), function->name.c_str(), (double)function->timeSpent / 1e+9);
        for (HashMap<FunctionMeasure *, FunctionMeasure *>::iterator it = function->children.begin();
             it != function->children.end(); ++it)
        {
            FunctionMeasure * childFunction = it->second;
            if (childFunction->frameCounter == Core::Instance()->GetGlobalFrameIndex())
                Dump(childFunction, level + 1);
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
