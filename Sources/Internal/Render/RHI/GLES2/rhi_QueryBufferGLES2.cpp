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

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;

    #include "_gl.h"

#if defined(__DAVAENGINE_IPHONE__)

#define _glBeginQuery(q) glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, q)
#define _glEndQuery() glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT)

#elif defined(__DAVAENGINE_ANDROID__)

#define _glBeginQuery(q)
#define _glEndQuery()

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

#define _glBeginQuery(q) glBeginQuery(GL_SAMPLES_PASSED, q)
#define _glEndQuery() glEndQuery(GL_SAMPLES_PASSED)

#else

#define _glBeginQuery(q) glBeginQuery(GL_ANY_SAMPLES_PASSED, q)
#define _glEndQuery() glEndQuery(GL_ANY_SAMPLES_PASSED)

#endif

#define MAX_PENDING_QUERIES 256

namespace rhi
{
//==============================================================================

class
QueryBufferGLES2_t
{
public:
    QueryBufferGLES2_t()
        : curObjectIndex(DAVA::InvalidIndex)
        , bufferCompleted(false){};
    ~QueryBufferGLES2_t(){};

    std::vector<std::pair<GLuint, uint32>> pendingQueries;
    std::vector<uint32> results;
    uint32 curObjectIndex;
    uint32 bufferCompleted : 1;
};

typedef ResourcePool<QueryBufferGLES2_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false> QueryBufferGLES2Pool;
RHI_IMPL_POOL(QueryBufferGLES2_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false);

std::vector<GLuint> QueryObjectPool;

//==============================================================================

static Handle
gles2_QueryBuffer_Create(uint32 maxObjectCount)
{
    Handle handle = QueryBufferGLES2Pool::Alloc();
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    buf->results.resize(maxObjectCount);
    memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());
    buf->pendingQueries.clear();
    buf->curObjectIndex = DAVA::InvalidIndex;
    buf->bufferCompleted = false;

    return handle;
}

static void
gles2_QueryBuffer_Reset(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());

    if (buf->pendingQueries.size())
    {
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
            QueryObjectPool.push_back(buf->pendingQueries[q].first);

        buf->pendingQueries.clear();
    }

    buf->curObjectIndex = DAVA::InvalidIndex;
    buf->bufferCompleted = false;
}

static void
gles2_QueryBuffer_Delete(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    if (buf->pendingQueries.size())
    {
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
            QueryObjectPool.push_back(buf->pendingQueries[q].first);

        buf->pendingQueries.clear();
    }

    QueryBufferGLES2Pool::Free(handle);
}

static void
gles2_Check_Query_Results(QueryBufferGLES2_t* buf)
{
    GLCommand cmd[MAX_PENDING_QUERIES];
    uint32 results[MAX_PENDING_QUERIES];
    uint32 cmdCount = uint32(buf->pendingQueries.size());

    if (cmdCount)
    {
        DVASSERT(cmdCount < MAX_PENDING_QUERIES);

        for (uint32 q = 0; q < cmdCount; ++q)
        {
            results[q] = uint32(-1);
            cmd[q] = { GLCommand::GET_QUERY_RESULT_NO_WAIT, { uint64(buf->pendingQueries[q].first), uint64(&results[q]) } };
        }

        ExecGL(cmd, cmdCount);

        for (int32 q = cmdCount - 1; q >= 0; --q)
        {
            uint32 resultIndex = buf->pendingQueries[q].second;
            if (results[q] != uint32(-1))
            {
                if (resultIndex < buf->results.size())
                    buf->results[resultIndex] = results[q];

                QueryObjectPool.push_back(buf->pendingQueries.back().first);

                buf->pendingQueries[q] = buf->pendingQueries.back();
                buf->pendingQueries.pop_back();
            }
        }
    }
}

static bool
gles2_QueryBuffer_IsReady(Handle handle, uint32 objectIndex)
{
    bool ready = false;
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    if (buf->bufferCompleted)
    {
        gles2_Check_Query_Results(buf);

        ready = true;
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
        {
            if (buf->pendingQueries[q].second == objectIndex)
            {
                ready = false;
                break;
            }
        }
    }

    return ready;
}

static int
gles2_QueryBuffer_Value(Handle handle, uint32 objectIndex)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);
    gles2_Check_Query_Results(buf);

    if (objectIndex < buf->results.size())
    {
        return buf->results[objectIndex];
    }

    return 0;
}

namespace QueryBufferGLES2
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_QueryBuffer_Create = &gles2_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset = &gles2_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete = &gles2_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady = &gles2_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value = &gles2_QueryBuffer_Value;
}

void SetQueryIndex(Handle handle, uint32 objectIndex)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    if (buf->curObjectIndex != objectIndex)
    {
        if (buf->curObjectIndex != DAVA::InvalidIndex)
            _glEndQuery();

        if (objectIndex != DAVA::InvalidIndex)
        {
            GLuint q = 0;
            if (QueryObjectPool.size())
            {
                q = QueryObjectPool.back();
                QueryObjectPool.pop_back();
            }
            else
            {
#if defined(__DAVAENGINE_IPHONE__)
                glGenQueriesEXT(1, &q);
#elif defined(__DAVAENGINE_ANDROID__)
#else
                glGenQueries(1, &q);
#endif
            }

            if (q)
            {
                _glBeginQuery(q);
                buf->pendingQueries.push_back(std::make_pair(q, objectIndex));
            }
        }

        buf->curObjectIndex = objectIndex;
    }
}

void QueryComplete(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    if (buf->curObjectIndex != DAVA::InvalidIndex)
    {
        _glEndQuery();
        buf->curObjectIndex = DAVA::InvalidIndex;
    }

    buf->bufferCompleted = true;
}

bool QueryIsCompleted(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
    DVASSERT(buf);

    return buf->bufferCompleted;
}
}

//==============================================================================
} // namespace rhi
