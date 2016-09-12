#include "../Common/rhi_Private.h"
#include "../rhi_Public.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_gl.h"

namespace rhi
{
class PerfQueryGLES2_t
{
public:
    struct Desc
    {
    };
    PerfQueryGLES2_t() = default;
    ~PerfQueryGLES2_t() = default;

    uint64 timestamp = 0;
    bool isUsed = false;
    bool isReady = false;
    bool isValid = false;
};

//==============================================================================

typedef ResourcePool<PerfQueryGLES2_t, RESOURCE_PERFQUERY, PerfQueryGLES2_t::Desc, false> PerfQueryGLES2Pool;
RHI_IMPL_POOL(PerfQueryGLES2_t, RESOURCE_PERFQUERY, PerfQueryGLES2_t::Desc, false);

DAVA::Vector<GLuint> queryObjectPool;
DAVA::List<std::pair<PerfQueryGLES2_t*, GLuint>> pendingQueries;
Handle currentFramePerfQuery[2] = { InvalidHandle, InvalidHandle };
GLuint currentTimeElapsedQuery = 0;

//==============================================================================

static Handle gles2_PerfQuery_Create()
{
    DVASSERT(DeviceCaps().isPerfQuerySupported);

    Handle handle = PerfQueryGLES2Pool::Alloc();
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        query->timestamp = 0;
        query->isUsed = false;
        query->isReady = false;
        query->isValid = false;
    }

    return handle;
}

static void gles2_PerfQuery_Delete(Handle handle)
{
    PerfQueryGLES2Pool::Free(handle);
}

static void gles2_PerfQuery_Reset(Handle handle)
{
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        query->timestamp = 0;
        query->isReady = false;
        query->isValid = false;
        query->isUsed = false;
    }
}

static bool gles2_PerfQuery_IsReady(Handle handle)
{
    bool ret = false;
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        ret = query->isReady && query->isUsed;
    }
    return ret;
}

static uint64 gles2_PerfQuery_Value(Handle handle)
{
    uint64 ret = 0;
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query && query->isReady)
    {
        if (query->isValid)
            ret = query->timestamp;
        else
            ret = uint64(-1);
    }
    return ret;
}

static void gles2_PerfQuery_SetCurrent(Handle handle0, Handle handle1)
{
    currentFramePerfQuery[0] = handle0;
    currentFramePerfQuery[1] = handle1;
}

namespace PerfQueryGLES2
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuery_Create = &gles2_PerfQuery_Create;
    dispatch->impl_PerfQuery_Delete = &gles2_PerfQuery_Delete;
    dispatch->impl_PerfQuery_Reset = &gles2_PerfQuery_Reset;
    dispatch->impl_PerfQuery_IsReady = &gles2_PerfQuery_IsReady;
    dispatch->impl_PerfQuery_Value = &gles2_PerfQuery_Value;
    dispatch->impl_PerfQuery_SetCurrent = &gles2_PerfQuery_SetCurrent;
}

GLuint GetQueryFromPool()
{
    GLuint q = 0;
    if (queryObjectPool.size())
    {
        q = queryObjectPool.back();
        queryObjectPool.pop_back();
    }

    if (!q)
    {
#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
        if (glGenQueries)
        {
            GL_CALL(glGenQueries(1, &q));
        }
#else
        GL_CALL(glGenQueries(1, &q));
#endif
    }

    return q;
}

void IssueTimestampQuery(Handle handle)
{
    DVASSERT(_GLES2_TimeStampQuerySupported);

    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        DVASSERT(!query->isUsed);

        GLuint queryObject = GetQueryFromPool();
        
#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
        if (glQueryCounter)
        {
            GL_CALL(glQueryCounter(queryObject, GL_TIMESTAMP));
        }
#elif defined(__DAVAENGINE_MACOS__)
#else
        GL_CALL(glQueryCounter(queryObject, GL_TIMESTAMP));
#endif

        query->isUsed = true;
        pendingQueries.push_back(std::pair<PerfQueryGLES2_t*, GLuint>(query, queryObject));
    }
}

void BeginTimeElapsedQuery(Handle handle)
{
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        DVASSERT(!query->isUsed);
        DVASSERT(!currentTimeElapsedQuery);

        currentTimeElapsedQuery = GetQueryFromPool();

#if defined(__DAVAENGINE_ANDROID__)
        if (glBeginQuery)
        {
            GL_CALL(glBeginQuery(GL_TIME_ELAPSED, currentTimeElapsedQuery));
        }
#else
        GL_CALL(glBeginQuery(GL_TIME_ELAPSED, currentTimeElapsedQuery));
#endif

        query->isUsed = true;
        query->timestamp = 0;
        query->isValid = true;
        query->isReady = true;
    }
}

void EndTimeElapsedQuery(Handle handle)
{
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        DVASSERT(!query->isUsed);
        DVASSERT(currentTimeElapsedQuery);

#if defined(__DAVAENGINE_ANDROID__)
        if (glEndQuery)
        {
            GL_CALL(glEndQuery(GL_TIME_ELAPSED));
        }
#else
        GL_CALL(glEndQuery(GL_TIME_ELAPSED));
#endif

        query->isUsed = true;
        pendingQueries.push_back(std::pair<PerfQueryGLES2_t*, GLuint>(query, currentTimeElapsedQuery));
        currentTimeElapsedQuery = 0;
    }
}

//==============================================================================

void ObtainPerfQueryResults()
{
    DAVA::Vector<std::pair<PerfQueryGLES2_t*, GLuint>> completedQueries;
    DAVA::List<std::pair<PerfQueryGLES2_t*, GLuint>>::iterator it = pendingQueries.begin();
    while (it != pendingQueries.end())
    {
        uint32 result = 0;
#if defined(__DAVAENGINE_IPHONE__)
        result = 1;
#elif defined(__DAVAENGINE_ANDROID__)
        if (glGetQueryObjectuiv && DeviceCaps().isPerfQuerySupported)
        {
            GL_CALL(glGetQueryObjectuiv(it->second, GL_QUERY_RESULT_AVAILABLE, &result));
        }
        else
        {
            result = 1;
        }
#else
        if (DeviceCaps().isPerfQuerySupported)
        {
            GL_CALL(glGetQueryObjectuiv(it->second, GL_QUERY_RESULT_AVAILABLE, &result));
        }
        else
        {
            result = 1;
        }
#endif

        if (result == GL_TRUE)
        {
            completedQueries.push_back(*it);
            it = pendingQueries.erase(it);
        }
        else
        {
            ++it;
        }
    }

    GLint disjointOccurred = 0;
#ifdef __DAVAENGINE_ANDROID__
    if (DeviceCaps().isPerfQuerySupported && _GLES2_TimeStampQuerySupported)
    {
        GL_CALL(glGetIntegerv(GL_GPU_DISJOINT, &disjointOccurred));
    }
#endif

    for (std::pair<PerfQueryGLES2_t*, GLuint>& p : completedQueries)
    {
        uint64 ts = 0;
        if (!disjointOccurred)
        {
#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
            if (glGetQueryObjectui64v)
            {
                GL_CALL(glGetQueryObjectui64v(p.second, GL_QUERY_RESULT, &ts));
            }
#elif defined(__DAVAENGINE_MACOS__)
            GL_CALL(glGetQueryObjectui64vEXT(p.second, GL_QUERY_RESULT, &ts));
#else
            GL_CALL(glGetQueryObjectui64v(p.second, GL_QUERY_RESULT, &ts));
#endif
            ts /= 1000; //mcs
        }

        if (p.first->isUsed)
        {
            p.first->timestamp = ts;
            p.first->isValid = !disjointOccurred;
            p.first->isReady = true;
        }

        queryObjectPool.push_back(p.second);
    }
}

void IssueQuery(Handle handle)
{
    if (_GLES2_TimeStampQuerySupported)
    {
        IssueTimestampQuery(handle);
    }
    else
    {
        if (!currentTimeElapsedQuery)
            BeginTimeElapsedQuery(handle);
        else
            EndTimeElapsedQuery(handle);
    }
}

void SkipQuery(Handle handle)
{
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        query->timestamp = 0;
        query->isReady = true;
        query->isValid = true;
        query->isUsed = true;
    }
}

void GetCurrentFrameQueries(Handle* query0, Handle* query1)
{
    *query0 = currentFramePerfQuery[0];
    *query1 = currentFramePerfQuery[1];
}

void ReleaseQueryObjectsPool()
{
    if (queryObjectPool.size())
    {
        GLCommand cmd = { GLCommand::DELETE_QUERIES, { uint64(queryObjectPool.size()), uint64(queryObjectPool.data()) } };
        ExecGL(&cmd, 1);

        queryObjectPool.clear();
    }
}
}

//==============================================================================
} // namespace rhi
