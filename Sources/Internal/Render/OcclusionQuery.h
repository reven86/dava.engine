#ifndef __DAVAENGINE_OCCLUSION_QUERY__
#define __DAVAENGINE_OCCLUSION_QUERY__


#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Base/Observer.h"

#if defined(__DAVAENGINE_OPENGL__)

namespace DAVA
{
class ApplicationCore;

class OcclusionQuery
{
public:
    OcclusionQuery();
    ~OcclusionQuery();

    enum eQueryResult
    {
        WAIT = 0,
        RESULT = 1,
    };
    void Init();
    void Release();
    void BeginQuery();
    void EndQuery();

    bool IsResultAvailable();
    void GetQuery(uint32* resultValue);
    uint32 GetId() const
    {
        return id;
    };

private:
    uint32 id;
};

template <uint32 N, uint32 M>
class SmartHandle
{
public:
    inline SmartHandle()
    {
    }
    inline SmartHandle(uint32 _index, uint32 _salt)
        : index(_index)
        , salt(_salt)
    {
    }
    uint32 index : N;
    uint32 salt : M;
};

using OcclusionQueryPoolHandle = SmartHandle<32, 32>;

class OcclusionQueryPool
{
public:
    static const uint32 INVALID_INDEX = 0xFFFFFFFF;

    OcclusionQueryPool(uint32 occlusionQueryCount);
    ~OcclusionQueryPool();

    OcclusionQueryPoolHandle CreateQueryObject();
    OcclusionQuery& Get(OcclusionQueryPoolHandle handle);
    void ReleaseQueryObject(OcclusionQueryPoolHandle handle);

private:
    uint32 createdCounter;
    uint32 occlusionQueryCount;
    uint32 nextFree;
    struct OcclusionQueryItem
    {
        OcclusionQuery query;
        uint32 next;
        uint16 salt;
    };
    Vector<OcclusionQueryItem> queries;
};

inline OcclusionQuery& OcclusionQueryPool::Get(OcclusionQueryPoolHandle handle)
{
    return queries[handle.index].query;
}

#define FRAME_QUERY_POOL_SIZE 20

class FrameOcclusionQueryManager : public Singleton<FrameOcclusionQueryManager>
{
    struct FrameQuery
    {
        OcclusionQueryPool queryPool;
        Vector<OcclusionQueryPoolHandle> activeQueries;
        uint32 drawedFrameStats;
        FastName queryName;
        bool isQueryOpen;

        FrameQuery(const FastName& name)
            :
            queryPool(FRAME_QUERY_POOL_SIZE)
            ,
            drawedFrameStats(0)
            ,
            queryName(name)
            ,
            isQueryOpen(false)
        {
        }
    };

public:
    enum eRetrieveBehavior
    {
        BEHAVIOR_WAIT,
        BEHAVIOR_SKIP,
        BEHAVIOR_RETRIEVE_ON_NEXT_FRAME
    };

    FrameOcclusionQueryManager();
    virtual ~FrameOcclusionQueryManager();

    void BeginQuery(const FastName& queryName);
    void EndQuery(const FastName& queryName);
    bool IsQueryOpen(const FastName& queryName);
    uint32 GetFrameStats(const FastName& queryName) const;
    void GetQueriesNames(Vector<FastName>& names) const;

    void SetRetrieveBehavior(eRetrieveBehavior _behavior)
    {
        behavior = _behavior;
    };

private:
    void ResetFrameStats();
    void ProccesRenderedFrame();
    FrameOcclusionQueryManager::FrameQuery* GetQuery(const FastName& queryName) const;

    Vector<FrameQuery*> frameQueries;

    eRetrieveBehavior behavior;
    bool frameBegan;

    friend class ApplicationCore;
};

/*
    id queryId = occlusionQuery->CreateQueryObject();
    occlusionQuery->BeginQuery(queryID);
    
    occlusionQuery->EndQuery(queryID);
 
    occlusionQuery->GetQuery(
 
 */
};

#endif // #if defined(__DAVAENGINE_OPENGL__)

#endif //__DAVAENGINE_OCCLUSION_QUERY__
