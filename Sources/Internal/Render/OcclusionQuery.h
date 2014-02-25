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
	
namespace DAVA
{

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
    GLuint GetId() const { return id; };
private:
    GLuint id;
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
    
typedef SmartHandle<32, 32> OcclusionQueryManagerHandle;
    
class OcclusionQueryManager
{
public:
    static const uint32 INVALID_INDEX = 0xFFFFFFFF;
    
    OcclusionQueryManager(uint32 occlusionQueryCount);
    ~OcclusionQueryManager();
    
    OcclusionQueryManagerHandle CreateQueryObject();
    OcclusionQuery & Get(OcclusionQueryManagerHandle handle);
    void ReleaseQueryObject(OcclusionQueryManagerHandle handle);
    
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
    
inline OcclusionQuery & OcclusionQueryManager::Get(OcclusionQueryManagerHandle handle)
{
    return queries[handle.index].query;
}

    
/*
    id queryId = occlusionQuery->CreateQueryObject();
    occlusionQuery->BeginQuery(queryID);
    
    occlusionQuery->EndQuery(queryID);
 
    occlusionQuery->GetQuery(
 
 */
    


};

#endif //__DAVAENGINE_OCCLUSION_QUERY__
