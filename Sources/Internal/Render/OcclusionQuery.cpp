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
#include "Render/OcclusionQuery.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"

namespace DAVA
{
#if defined(__DAVAENGINE_OPENGL__)

/////////////////////////////////////////////////////////////////////
///////////OcclusionQuery

OcclusionQuery::OcclusionQuery()
{
    id = 0;
}
    
void OcclusionQuery::Init()
{
#if RHI_COMPLETE
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
        RENDER_VERIFY(glGenQueries(1, &id));
#else
        RENDER_VERIFY(glGenQueriesEXT(1, &id));
#endif
        //Logger::Debug("Init query: %d", id);
#endif // RHI_COMPLETE
}
    
void OcclusionQuery::Release()
{
#if RHI_COMPLETE
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
        RENDER_VERIFY(glDeleteQueries(1, &id));
#else
        RENDER_VERIFY(glDeleteQueriesEXT(1, &id));
#endif
        //Logger::Debug("Release query: %d", id);
#endif // RHI_COMPLETE
}

OcclusionQuery::~OcclusionQuery()
{
    if(id != 0)
    {
        Release();
        id = 0;
    }
}

void OcclusionQuery::BeginQuery()
{
#if RHI_COMPLETE
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glBeginQuery(GL_SAMPLES_PASSED, id));
#else
    RENDER_VERIFY(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, id));
#endif
#endif // RHI_COMPLETE
}
    
void OcclusionQuery::EndQuery()
{
#if RHI_COMPLETE
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glEndQuery(GL_SAMPLES_PASSED));
#else
    RENDER_VERIFY(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#endif
#endif // RHI_COMPLETE
}
    
bool OcclusionQuery::IsResultAvailable()
{
#if RHI_COMPLETE
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    GLint available;
    RENDER_VERIFY(glGetQueryObjectiv(id,
                          GL_QUERY_RESULT_AVAILABLE_ARB,
                          &available));
    return (available != 0);
#elif defined(__DAVAENGINE_ANDROID__)
    GLuint available;
    RENDER_VERIFY(glGetQueryObjectuiv(id,
                                     GL_QUERY_RESULT_AVAILABLE,
                                     &available));
    return (available != 0);
#else
    GLuint available;
    RENDER_VERIFY(glGetQueryObjectuivEXT(id,
                                     GL_QUERY_RESULT_AVAILABLE_EXT,
                                     &available));
    return (available != 0);
#endif
#endif // RHI_COMPLETE
    return true;
}
    
void OcclusionQuery::GetQuery(uint32 * resultValue)
{
#if RHI_COMPLETE
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glGetQueryObjectuiv(id, GL_QUERY_RESULT_ARB, resultValue));
#else
    RENDER_VERIFY(glGetQueryObjectuivEXT(id, GL_QUERY_RESULT_EXT, resultValue));
#endif
#endif // RHI_COMPLETE
}
   
/////////////////////////////////////////////////////////////////////
///////////OcclusionQueryPool
    
OcclusionQueryPool::OcclusionQueryPool(uint32 _occlusionQueryCount)
{
    createdCounter = 0;
    nextFree = 0;
    occlusionQueryCount = _occlusionQueryCount;
    queries.resize(occlusionQueryCount);
    for (uint32 k = 0; k < occlusionQueryCount; ++k)
    {
        queries[k].query.Init();
        queries[k].next = (k == occlusionQueryCount - 1) ? (INVALID_INDEX) : (k + 1);
        queries[k].salt = 0;
        //Logger::FrameworkDebug("i: %ld %ld %ld", queries[k].query.GetId(), queries[k].next, queries[k].salt);
    }
}
    
OcclusionQueryPool::~OcclusionQueryPool()
{
    for (uint32 k = 0; k < occlusionQueryCount; ++k)
    {
        queries[k].query.Release();
    }
    queries.clear();
}

OcclusionQueryPoolHandle OcclusionQueryPool::CreateQueryObject()
{
    if (nextFree == INVALID_INDEX)
    {
        uint32 oldOcclusionQueryCount = occlusionQueryCount;
        queries.resize(occlusionQueryCount + 100);
        occlusionQueryCount += 100;
        
        for (uint32 k = occlusionQueryCount - 1; k >= oldOcclusionQueryCount; --k)
        {
            queries[k].query.Init();
            queries[k].next = nextFree;
            queries[k].salt = 0;
            nextFree = k;
        }
    }
    createdCounter++;
    OcclusionQueryPoolHandle handle;
    handle.index = nextFree;
    handle.salt = queries[nextFree].salt;
    nextFree = queries[nextFree].next;
    return handle;
}
    
void OcclusionQueryPool::ReleaseQueryObject(OcclusionQueryPoolHandle handle)
{
    createdCounter--;
    DVASSERT(handle.salt == queries[handle.index].salt);
    queries[handle.index].salt++;
    queries[handle.index].next = nextFree;
    nextFree = handle.index;
}

/////////////////////////////////////////////////////////////////////
///////////FrameOcclusionQueryManager

FrameOcclusionQueryManager::FrameOcclusionQueryManager() :
behavior(BEHAVIOR_WAIT),
frameBegan(false)
{}

FrameOcclusionQueryManager::~FrameOcclusionQueryManager()
{
    for(auto& frameQuery : frameQueries)
    {
        SafeDelete(frameQuery);
    }
}

FrameOcclusionQueryManager::FrameQuery * FrameOcclusionQueryManager::GetQuery(const FastName & queryName) const
{
    for(auto frameQuery : frameQueries)
    {
        if (frameQuery->queryName == queryName)
        {
            return frameQuery;
        }
    }
    return nullptr;
}

void FrameOcclusionQueryManager::ResetFrameStats() //OnBeginFrame
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS))
        return;

    frameBegan = true;

    for (auto frameQuery : frameQueries)
    {
        frameQuery->drawedFrameStats = 0;
    }
}

void FrameOcclusionQueryManager::ProccesRenderedFrame() //OnEndFrame
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS))
        return;

    frameBegan = false;

    int32 frameQueriesCount = static_cast<int32>(frameQueries.size());
    for(int32 i = 0; i < frameQueriesCount; ++i)
    {
        FrameQuery * frameQuery = frameQueries[i];
        for(int32 q = static_cast<int32>(frameQuery->activeQueries.size() - 1); q >= 0; --q)
        {
            OcclusionQueryPoolHandle queryHandle = frameQuery->activeQueries[q];
            OcclusionQuery & query = frameQuery->queryPool.Get(queryHandle);

            bool needRetrieve = false;
            bool needRemove = false;

            if(behavior == BEHAVIOR_WAIT)
            {
                while(!query.IsResultAvailable()) {}

                needRetrieve = true;
                needRemove = true;
            }
            else if(behavior == BEHAVIOR_SKIP)
            {
                needRemove = true;
                needRetrieve = query.IsResultAvailable();
            }
            else if(behavior == BEHAVIOR_RETRIEVE_ON_NEXT_FRAME)
            {
                needRetrieve = needRemove = query.IsResultAvailable();
            }

            if(needRetrieve)
            {
                uint32 result = 0;
                query.GetQuery(&result);
                frameQuery->drawedFrameStats += result;
            }

            if(needRemove)
            {
                frameQuery->queryPool.ReleaseQueryObject(queryHandle);
                RemoveExchangingWithLast(frameQuery->activeQueries, q);
            }
        }
    }
}

void FrameOcclusionQueryManager::BeginQuery(const FastName & queryName)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS))
        return;

    FrameQuery * frameQuery = GetQuery(queryName);
    if(!frameQuery)
    {
        frameQuery = new FrameQuery(queryName);
        frameQueries.push_back(frameQuery);
    }

    DVASSERT(!frameQuery->isQueryOpen);

    OcclusionQueryPoolHandle handle = frameQuery->queryPool.CreateQueryObject();
    frameQuery->queryPool.Get(handle).BeginQuery();
    frameQuery->activeQueries.push_back(handle);
    frameQuery->isQueryOpen = true;
}

void FrameOcclusionQueryManager::EndQuery(const FastName & queryName)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS))
        return;

    FrameOcclusionQueryManager::FrameQuery * frameQuery = GetQuery(queryName);
    DVASSERT(frameQuery);
    DVASSERT(frameQuery->isQueryOpen);

    OcclusionQueryPoolHandle handle = frameQuery->activeQueries.back();
    frameQuery->queryPool.Get(handle).EndQuery();
    frameQuery->isQueryOpen = false;
}

bool FrameOcclusionQueryManager::IsQueryOpen(const FastName & queryName)
{
	FrameOcclusionQueryManager::FrameQuery * frameQuery = GetQuery(queryName);
	if (frameQuery)
		return frameQuery->isQueryOpen;

	return false;
}

uint32 FrameOcclusionQueryManager::GetFrameStats(const FastName & queryName) const
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LAYER_OCCLUSION_STATS))
        return 0;

    DVASSERT(!frameBegan); //should be called on after EndFrame() and before BeginFrame()

    const FrameQuery * frameQuery = GetQuery(queryName);
    if(frameQuery)
        return frameQuery->drawedFrameStats;

    return 0;
}

void FrameOcclusionQueryManager::GetQueriesNames(Vector<FastName> & names) const
{
    for (auto query : frameQueries)
    {
        names.emplace_back(query->queryName);
    }
}

#else
#error "Require Occlusion Queries Implementation"
#endif

};
