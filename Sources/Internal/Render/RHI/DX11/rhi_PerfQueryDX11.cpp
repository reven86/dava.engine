#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_dx11.h"

namespace rhi
{
//==============================================================================

struct FreqPerfQuery
{
    ID3D11Query* freqQuery = nullptr;
    DAVA::List<HPerfQuery> perfQueries;
    uint64 freq = 0;
    bool isFreqValid = false;
};

class PerfQueryDX11_t
{
public:
    struct Desc
    {
    };
    PerfQueryDX11_t() = default;
    ~PerfQueryDX11_t() = default;

    ID3D11Query* query = nullptr;
    uint64 timestamp = 0;
    uint64 freq = 0;
    bool isUsed = false;
    bool isReady = false;
    bool isFreqValid = false;
};

//==============================================================================

typedef ResourcePool<PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false> PerfQueryDX11Pool;
RHI_IMPL_POOL(PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false);

DAVA::Mutex perfQueryMutex;
DAVA::List<FreqPerfQuery*> pendingFreqPerfQuery;
DAVA::Vector<FreqPerfQuery*> freqPerfQueryPool;
FreqPerfQuery* currentFreqPerfQuery = nullptr;
Handle currentFramePerfQuery0 = InvalidHandle;
Handle currentFramePerfQuery1 = InvalidHandle;

//==============================================================================

static Handle dx11_PerfQuery_Create()
{
    Handle handle = PerfQueryDX11Pool::Alloc();
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        perfQuery->timestamp = 0;
        perfQuery->freq = 0;
        perfQuery->isUsed = false;
        perfQuery->isReady = false;
        perfQuery->isFreqValid = false;

        DVASSERT(perfQuery->query == nullptr)

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        desc.MiscFlags = 0;
        HRESULT hr = _D3D11_Device->CreateQuery(&desc, &(perfQuery->query));
        CHECK_HR(hr);

        if (hr == S_OK)
        {
        }
        else
        {
            PerfQueryDX11Pool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

static void dx11_PerfQuery_Delete(Handle handle)
{
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        perfQuery->query->Release();
        perfQuery->query = nullptr;
    }

    PerfQueryDX11Pool::Free(handle);
}

static void dx11_PerfQuery_Reset(Handle handle)
{
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        perfQuery->freq = 0;
        perfQuery->isFreqValid = false;
        perfQuery->isReady = false;
        perfQuery->isUsed = false;
        perfQuery->timestamp = 0;
    }
}

static bool dx11_PerfQuery_IsReady(Handle handle)
{
    DAVA::LockGuard<Mutex> guard(perfQueryMutex);

    bool ret = false;
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        ret = perfQuery->isReady && perfQuery->freq;
    }

    return ret;
}

static uint64 dx11_PerfQuery_Value(Handle handle)
{
    DAVA::LockGuard<Mutex> guard(perfQueryMutex);

    uint64 ret = 0;
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery)
    {
        if (perfQuery->isReady)
        {
            if (perfQuery->isFreqValid)
                ret = perfQuery->timestamp / (perfQuery->freq / 1000000); //mcs
            else
                ret = uint64(-1);
        }
    }

    return ret;
}

static void dx11_PerfQuery_SetCurrent(Handle start, Handle end)
{
    currentFramePerfQuery0 = start;
    currentFramePerfQuery1 = end;
}

namespace PerfQueryDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuery_Create = &dx11_PerfQuery_Create;
    dispatch->impl_PerfQuery_Delete = &dx11_PerfQuery_Delete;
    dispatch->impl_PerfQuery_Reset = &dx11_PerfQuery_Reset;
    dispatch->impl_PerfQuery_IsReady = &dx11_PerfQuery_IsReady;
    dispatch->impl_PerfQuery_Value = &dx11_PerfQuery_Value;
    dispatch->impl_PerfQuery_SetCurrent = &dx11_PerfQuery_SetCurrent;
}

void GetCurrentFrameQueries(Handle& start, Handle& end)
{
    start = currentFramePerfQuery0;
    end = currentFramePerfQuery1;
}

void IssueTimestampQuery(Handle handle, ID3D11DeviceContext* context)
{
    DVASSERT(currentFreqPerfQuery);
    if (currentFreqPerfQuery)
    {
        PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
        if (perfQuery)
        {
            DVASSERT(perfQuery->query);
            DVASSERT(!perfQuery->isUsed);

            context->End(perfQuery->query);

            perfQuery->isUsed = true;
            perfQuery->isReady = false;

            currentFreqPerfQuery->perfQueries.push_back(HPerfQuery(handle));
        }
    }
}

void BeginPerfQueryMeasurment(Handle frameQuery0, ID3D11DeviceContext* context)
{
    DVASSERT(!currentFreqPerfQuery);

    if (!freqPerfQueryPool.size())
    {
        currentFreqPerfQuery = new FreqPerfQuery();

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;
        HRESULT hr = _D3D11_Device->CreateQuery(&desc, &(currentFreqPerfQuery->freqQuery));
        CHECK_HR(hr);

        if (hr != S_OK)
        {
            DAVA::SafeDelete(currentFreqPerfQuery);
        }
    }
    else
    {
        currentFreqPerfQuery = freqPerfQueryPool.back();
        freqPerfQueryPool.pop_back();
    }

    if (currentFreqPerfQuery)
    {
        pendingFreqPerfQuery.push_back(currentFreqPerfQuery);

        context->Begin(currentFreqPerfQuery->freqQuery);

        if (frameQuery0)
            IssueTimestampQuery(frameQuery0, context);
    }
}

void ObtainPerfQueryMeasurment()
{
    DAVA::List<FreqPerfQuery*>::iterator fit = pendingFreqPerfQuery.begin();
    while (fit != pendingFreqPerfQuery.end())
    {
        FreqPerfQuery* frame = *fit;

        if (!frame->freq)
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
            HRESULT hr = _D3D11_ImmediateContext->GetData(frame->freqQuery, &data, sizeof(data), 0);
            CHECK_HR(hr);

            if (hr == S_OK)
            {
                frame->isFreqValid = !data.Disjoint;
                frame->freq = data.Frequency;
            }
        }

        perfQueryMutex.Lock();
        DAVA::List<HPerfQuery>::iterator qit = frame->perfQueries.begin();
        while (qit != frame->perfQueries.end())
        {
            PerfQueryDX11_t* query = PerfQueryDX11Pool::Get(*qit);

            DVASSERT(query->isUsed);

            if (!query->isReady)
            {
                HRESULT hr = _D3D11_ImmediateContext->GetData(query->query, &(query->timestamp), sizeof(uint64), 0);
                query->isReady = (hr == S_OK);
            }

            if (frame->freq && query->isReady)
            {
                query->freq = frame->freq;
                query->isFreqValid = frame->isFreqValid;

                qit = frame->perfQueries.erase(qit);
            }
            else
            {
                ++qit;
            }
        }
        perfQueryMutex.Unlock();

        if (frame->freq && !frame->perfQueries.size())
        {
            freqPerfQueryPool.push_back(frame);
            fit = pendingFreqPerfQuery.erase(fit);
        }
        else
        {
            ++fit;
        }
    }
}

void EndPerfQueryMeasurment(Handle frameQuery1, ID3D11DeviceContext* context)
{
    if (currentFreqPerfQuery)
    {
        if (frameQuery1)
            IssueTimestampQuery(frameQuery1, context);

        context->End(currentFreqPerfQuery->freqQuery);
        currentFreqPerfQuery = nullptr;
    }
}

void ReleasePerfQueryPool()
{
    for (FreqPerfQuery* frame : pendingFreqPerfQuery)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    pendingFreqPerfQuery.clear();

    for (FreqPerfQuery* frame : freqPerfQueryPool)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    freqPerfQueryPool.clear();
}
}

//==============================================================================
} // namespace rhi
