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
    void GetQuery(uint32 * resultValue);
    uint32 GetId() const
    {
        return id;
    };

private:
    uint32 id;
};

template<uint32 N, uint32 M>
class SmartHandle
{
public:
    inline SmartHandle() {}
    inline SmartHandle(uint32 _index, uint32 _salt)
    : index(_index)
    , salt(_salt)
    {
    }
    uint32 index: N;
    uint32 salt: M;
};
    
using OcclusionQueryPoolHandle = SmartHandle<32, 32>;
    
class OcclusionQueryPool
{
public:
    static const uint32 INVALID_INDEX = 0xFFFFFFFF;
    
    OcclusionQueryPool(uint32 occlusionQueryCount);
    ~OcclusionQueryPool();
    
    OcclusionQueryPoolHandle CreateQueryObject();
    OcclusionQuery & Get(OcclusionQueryPoolHandle handle);
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
    
inline OcclusionQuery & OcclusionQueryPool::Get(OcclusionQueryPoolHandle handle)
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

        FrameQuery(const FastName & name) : 
            queryPool(FRAME_QUERY_POOL_SIZE),
            drawedFrameStats(0),
            queryName(name),
            isQueryOpen(false)
            {}
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

    void BeginQuery(const FastName & queryName);
    void EndQuery(const FastName & queryName);
	bool IsQueryOpen(const FastName & queryName);
    uint32 GetFrameStats(const FastName & queryName) const;
    void GetQueriesNames(Vector<FastName> & names) const;

    void SetRetrieveBehavior(eRetrieveBehavior _behavior) { behavior = _behavior; };

private:
    void ResetFrameStats();
    void ProccesRenderedFrame();
    FrameOcclusionQueryManager::FrameQuery * GetQuery(const FastName & queryName) const;

    Vector<FrameQuery *> frameQueries;

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
