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

//==============================================================================

static Handle
gles2_QueryBuffer_Create(uint32 maxObjectCount)
{
    Handle handle = QueryBufferGLES2Pool::Alloc();
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        buf->results.resize(maxObjectCount);
        memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());
        buf->pendingQueries.clear();
        buf->curObjectIndex = DAVA::InvalidIndex;
        buf->bufferCompleted = false;
    }

    return handle;
}

static void
gles2_QueryBuffer_Reset(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());

        if (buf->pendingQueries.size())
        {
            std::vector<GLuint> glQueries(buf->pendingQueries.size());
            std::transform(buf->pendingQueries.begin(), buf->pendingQueries.end(), glQueries.begin(), [](const std::pair<GLuint, uint32>& p) { return p.first; });

            GLCommand cmd1 = { GLCommand::DELETE_QUERIES, { uint64(glQueries.size()), uint64(glQueries.data()) } };
            ExecGL(&cmd1, 1);

            buf->pendingQueries.clear();
        }

        buf->curObjectIndex = DAVA::InvalidIndex;
        buf->bufferCompleted = false;
    }
}

static void
gles2_QueryBuffer_Delete(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        if (buf->pendingQueries.size())
        {
            std::vector<GLuint> glQueries(buf->pendingQueries.size());
            std::transform(buf->pendingQueries.begin(), buf->pendingQueries.end(), glQueries.begin(), [](const std::pair<GLuint, uint32>& p) { return p.first; });

            GLCommand cmd1 = { GLCommand::DELETE_QUERIES, { uint64(glQueries.size()), uint64(glQueries.data()) } };
            ExecGL(&cmd1, 1);

            buf->pendingQueries.clear();
        }
    }

    QueryBufferGLES2Pool::Free(handle);
}

static bool
gles2_QueryBuffer_IsReady(Handle handle, uint32 _1)
{
    bool ready = false;
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        if (buf->bufferCompleted)
        {
            ready = true;

            if (buf->pendingQueries.size())
            {
                for (int32 q = buf->pendingQueries.size() - 1; q >= 0; --q)
                {
                    GLuint queryName = buf->pendingQueries[q].first;
                    GLuint result = 0;

                    GLCommand cmd1 = { GLCommand::GET_QUERYOBJECT_UIV, { uint64(queryName), uint64(GL_QUERY_RESULT_AVAILABLE), uint64(&result) } };
                    ExecGL(&cmd1, 1);

                    if (result == GL_TRUE)
                    {
                        GLCommand cmd2[2] = {
                            { GLCommand::GET_QUERYOBJECT_UIV, { uint64(queryName), uint64(GL_QUERY_RESULT), uint64(&result) } },
                            { GLCommand::DELETE_QUERIES, {
                                                         1, uint64(&(queryName)),
                                                         } }
                        };
                        ExecGL(cmd2, 2);

                        uint32 resultIndex = buf->pendingQueries[q].second;
                        if (resultIndex < buf->results.size())
                            buf->results[resultIndex] = result;

                        buf->pendingQueries[q] = buf->pendingQueries.back();
                        buf->pendingQueries.pop_back();
                    }
                    else
                    {
                        ready = false;
                    }
                }
            }
        }
    }

    return ready;
}

static int
gles2_QueryBuffer_Value(Handle handle, uint32 objectIndex)
{
    if (gles2_QueryBuffer_IsReady(handle, objectIndex))
    {
        QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);
        if (buf && objectIndex < buf->results.size())
        {
            return buf->results[objectIndex];
        }
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

    if (buf)
    {
        if (buf->curObjectIndex != objectIndex)
        {
            if (buf->curObjectIndex != DAVA::InvalidIndex)
                _glEndQuery();

            if (objectIndex != DAVA::InvalidIndex)
            {
                GLuint q = 0;

#if defined(__DAVAENGINE_IPHONE__)
                glGenQueriesEXT(1, &q);
#elif defined(__DAVAENGINE_ANDROID__)
#else
                glGenQueries(1, &q);
#endif

                if (q)
                {
                    _glBeginQuery(q);
                    buf->pendingQueries.push_back(std::make_pair(q, objectIndex));
                }
            }

            buf->curObjectIndex = objectIndex;
        }
    }
}

void QueryComplete(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        if (buf->curObjectIndex != DAVA::InvalidIndex)
        {
            _glEndQuery();
            buf->curObjectIndex = DAVA::InvalidIndex;
        }

        buf->bufferCompleted = true;
    }
}

bool QueryIsCompleted(Handle handle)
{
    QueryBufferGLES2_t* buf = QueryBufferGLES2Pool::Get(handle);

    if (buf)
    {
        return buf->bufferCompleted;
    }

    return false;
}
}

//==============================================================================
} // namespace rhi
