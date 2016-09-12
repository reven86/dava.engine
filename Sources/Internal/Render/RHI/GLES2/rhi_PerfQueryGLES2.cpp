#include "../Common/rhi_Private.h"
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

//==============================================================================

static Handle gles2_PerfQuery_Create()
{
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
#else
        GL_CALL(glGetQueryObjectuiv(it->second, GL_QUERY_RESULT_AVAILABLE, &result));
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
    GL_CALL(glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjointOccurred));
#endif

    for (std::pair<PerfQueryGLES2_t*, GLuint>& p : completedQueries)
    {
        uint64 ts = 0;
        if (!disjointOccurred)
        {
#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
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

void IssueTimestampQuery(Handle handle)
{
    PerfQueryGLES2_t* query = PerfQueryGLES2Pool::Get(handle);
    if (query)
    {
        DVASSERT(!query->isUsed);

        GLuint queryObject = 0;
        if (queryObjectPool.size())
        {
            queryObject = queryObjectPool.back();
            queryObjectPool.pop_back();
        }

        if (!queryObject)
        {
#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
#else
            GL_CALL(glGenQueries(1, &queryObject));
#endif
        }

#if defined(__DAVAENGINE_IPHONE__)
#elif defined(__DAVAENGINE_ANDROID__)
#else
        GL_CALL(glQueryCounter(queryObject, GL_TIMESTAMP));
#endif

        query->isUsed = true;
        pendingQueries.push_back(std::pair<PerfQueryGLES2_t*, GLuint>(query, queryObject));
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
