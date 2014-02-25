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
#include "Render/RenderManager.h"
#include "Render/OcclusionQuery.h"

namespace DAVA
{
#if defined(__DAVAENGINE_OPENGL__)
OcclusionQuery::OcclusionQuery()
{
    id = 0;
}
    
void OcclusionQuery::Init()
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
        RENDER_VERIFY(glGenQueries(1, &id));
#else
        RENDER_VERIFY(glGenQueriesEXT(1, &id));
#endif
        //Logger::Debug("Init query: %d", id);
}
    
void OcclusionQuery::Release()
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
        RENDER_VERIFY(glDeleteQueries(1, &id));
#else
        RENDER_VERIFY(glDeleteQueriesEXT(1, &id));
#endif
        //Logger::Debug("Release query: %d", id);
}

OcclusionQuery::~OcclusionQuery()
{
    id = 0;
}

void OcclusionQuery::BeginQuery()
{
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glBeginQuery(GL_SAMPLES_PASSED, id));
#else
    RENDER_VERIFY(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, id));
#endif
}
    
void OcclusionQuery::EndQuery()
{
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glEndQuery(GL_SAMPLES_PASSED));
#else
    RENDER_VERIFY(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#endif
}
    
bool OcclusionQuery::IsResultAvailable()
{
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
	return false;
}
    
void OcclusionQuery::GetQuery(uint32 * resultValue)
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glGetQueryObjectuiv(id, GL_QUERY_RESULT_ARB, resultValue));
#else
    RENDER_VERIFY(glGetQueryObjectuivEXT(id, GL_QUERY_RESULT_EXT, resultValue));
#endif
}
    
    
OcclusionQueryManager::OcclusionQueryManager(uint32 _occlusionQueryCount)
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
    
OcclusionQueryManager::~OcclusionQueryManager()
{
    for (uint32 k = 0; k < occlusionQueryCount; ++k)
    {
        queries[k].query.Release();
    }
    queries.clear();
}

OcclusionQueryManagerHandle OcclusionQueryManager::CreateQueryObject()
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
    OcclusionQueryManagerHandle handle;
    handle.index = nextFree;
    handle.salt = queries[nextFree].salt;
    nextFree = queries[nextFree].next;
    return handle;
}
    
void OcclusionQueryManager::ReleaseQueryObject(OcclusionQueryManagerHandle handle)
{
    createdCounter--;
    DVASSERT(handle.salt == queries[handle.index].salt);
    queries[handle.index].salt++;
    queries[handle.index].next = nextFree;
    nextFree = handle.index;
}


#else
#error "Require Occlusion Queries Implementation"
#endif

};
