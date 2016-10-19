#include "VisibilityQueryResults.h"
#include "Debug/DVAssert.h"
#include "RHI/rhi_Public.h"

namespace DAVA
{
namespace VisibilityQueryResultsDetails
{
Vector<rhi::HQueryBuffer> queryBufferPool;
Vector<rhi::HQueryBuffer> pendingQueryBuffer;
rhi::HQueryBuffer currentQueryBuffer;

uint32 frameVisibilityResults[VisibilityQueryResults::QUERY_INDEX_COUNT] = {};

} //ns Details

namespace VisibilityQueryResults
{
using namespace VisibilityQueryResultsDetails;

rhi::HQueryBuffer GetCurrentQueryBuffer()
{
#ifdef __DAVAENGINE_RENDERSTATS__
    if (!currentQueryBuffer.IsValid())
    {
        if (queryBufferPool.empty())
        {
            currentQueryBuffer = rhi::CreateQueryBuffer(QUERY_INDEX_COUNT);
        }
        else
        {
            currentQueryBuffer = queryBufferPool.back();
            queryBufferPool.pop_back();
        }
    }
#endif

    return currentQueryBuffer;
}

uint32 GetResult(eQueryIndex index)
{
    return frameVisibilityResults[index];
}

FastName GetQueryIndexName(eQueryIndex index)
{
    static const FastName queryIndexNames[QUERY_INDEX_COUNT] =
    {
      FastName("OpaqueRenderLayer"),
      FastName("AfterOpaqueRenderLayer"),
      FastName("AlphaTestLayer"),
      FastName("WaterLayer"),
      FastName("TransclucentRenderLayer"),
      FastName("AfterTransclucentRenderLayer"),
      FastName("ShadowVolumeRenderLayer"),
      FastName("VegetationRenderLayer"),
      FastName("DebugRenderLayer"),
      FastName("UI"),
    };

    return queryIndexNames[index];
}

void EndFrame()
{
    DVASSERT(pendingQueryBuffer.size() < 128);

    while (!pendingQueryBuffer.empty() && rhi::QueryBufferIsReady(pendingQueryBuffer.front()))
    {
        for (uint32 i = 0; i < uint32(QUERY_INDEX_COUNT); ++i)
            frameVisibilityResults[i] = rhi::QueryValue(pendingQueryBuffer.front(), i);

        rhi::ResetQueryBuffer(pendingQueryBuffer.front());
        queryBufferPool.push_back(pendingQueryBuffer.front());
        pendingQueryBuffer.erase(pendingQueryBuffer.begin());
    }

    if (currentQueryBuffer.IsValid())
    {
        pendingQueryBuffer.push_back(currentQueryBuffer);
        currentQueryBuffer = rhi::HQueryBuffer();
    }
}

void Cleanup()
{
    queryBufferPool.insert(queryBufferPool.end(), pendingQueryBuffer.begin(), pendingQueryBuffer.end());
    queryBufferPool.push_back(currentQueryBuffer);

    for (rhi::HQueryBuffer h : queryBufferPool)
    {
        if (h.IsValid())
        {
            rhi::DeleteQueryBuffer(h);
        }
    }
}

} //ns VisibilityResults
} //ns DAVA