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
DAVA::List<PerfQueryDX11::FreqPerfQueryDX11*> pendingFreqPerfQuery;
DAVA::Vector<PerfQueryDX11::FreqPerfQueryDX11*> freqPerfQueryPool;
PerfQueryDX11::FreqPerfQueryDX11* currentFreqPerfQuery = nullptr;

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
    DVASSERT(currentFreqPerfQuery);
    currentFreqPerfQuery->frameQuery0 = start;
    currentFreqPerfQuery->frameQuery1 = end;
}

namespace PerfQueryDX11
{
void FreqPerfQueryDX11::IssueTimestamp(Handle handle, ID3D11DeviceContext* context)
{
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery)
    {
        DVASSERT(perfQuery->query);
        DVASSERT(!perfQuery->isUsed);

        context->End(perfQuery->query);

        perfQuery->isUsed = true;
        perfQuery->isReady = false;

        perfQueries.push_back(HPerfQuery(handle));
    }
}

void FreqPerfQueryDX11::BeginMeasurment(ID3D11DeviceContext* context)
{
    context->Begin(freqQuery);

    if (frameQuery0)
        IssueTimestamp(frameQuery0, context);
}

void FreqPerfQueryDX11::EndMeasurment(ID3D11DeviceContext* context)
{
    if (frameQuery1)
        IssueTimestamp(frameQuery1, context);

    context->End(freqQuery);

    pendingFreqPerfQuery.push_back(this);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuery_Create = &dx11_PerfQuery_Create;
    dispatch->impl_PerfQuery_Delete = &dx11_PerfQuery_Delete;
    dispatch->impl_PerfQuery_Reset = &dx11_PerfQuery_Reset;
    dispatch->impl_PerfQuery_IsReady = &dx11_PerfQuery_IsReady;
    dispatch->impl_PerfQuery_Value = &dx11_PerfQuery_Value;
    dispatch->impl_PerfQuery_SetCurrent = &dx11_PerfQuery_SetCurrent;
}

//==============================================================================
//called from main-thread

FreqPerfQueryDX11* NextFreqPerfQuery()
{
    currentFreqPerfQuery = nullptr;

    perfQueryMutex.Lock();
    if (freqPerfQueryPool.size())
    {
        currentFreqPerfQuery = freqPerfQueryPool.back();
        freqPerfQueryPool.pop_back();
    }
    perfQueryMutex.Unlock();

    if (currentFreqPerfQuery)
    {
        currentFreqPerfQuery->freq = 0;
        currentFreqPerfQuery->isFreqValid = false;
        currentFreqPerfQuery->perfQueries.clear();
        currentFreqPerfQuery->frameQuery0 = InvalidHandle;
        currentFreqPerfQuery->frameQuery1 = InvalidHandle;
    }
    else
    {
        currentFreqPerfQuery = new FreqPerfQueryDX11();

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;
        HRESULT hr = _D3D11_Device->CreateQuery(&desc, &(currentFreqPerfQuery->freqQuery));
        CHECK_HR(hr);
    }

    return currentFreqPerfQuery;
}

void IssueTimestampQuery(Handle handle, ID3D11DeviceContext* context)
{
    DVASSERT(currentFreqPerfQuery);
    currentFreqPerfQuery->IssueTimestamp(handle, context);
}

//==============================================================================
//called from render-thread

void ObtainPerfQueryMeasurment(ID3D11DeviceContext* context)
{
    DAVA::List<FreqPerfQueryDX11*>::iterator fit = pendingFreqPerfQuery.begin();
    while (fit != pendingFreqPerfQuery.end())
    {
        FreqPerfQueryDX11* frame = *fit;

        if (!frame->freq)
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
            HRESULT hr = context->GetData(frame->freqQuery, &data, sizeof(data), 0);
            CHECK_HR(hr);

            if (hr == S_OK)
            {
                frame->isFreqValid = !data.Disjoint;
                frame->freq = data.Frequency;
            }
        }

        DAVA::List<HPerfQuery>::iterator qit = frame->perfQueries.begin();
        while (qit != frame->perfQueries.end())
        {
            PerfQueryDX11_t* query = PerfQueryDX11Pool::Get(*qit);

            DVASSERT(query->isUsed);

            if (!query->timestamp)
            {
                HRESULT hr = context->GetData(query->query, &(query->timestamp), sizeof(uint64), 0);
                CHECK_HR(hr);
            }

            if (frame->freq && query->timestamp)
            {
                query->freq = frame->freq;
                query->isFreqValid = frame->isFreqValid;
                query->isReady = true;

                qit = frame->perfQueries.erase(qit);
            }
            else
            {
                ++qit;
            }
        }

        ++fit;
    }

    DAVA::LockGuard<DAVA::Mutex> guard(perfQueryMutex);

    fit = pendingFreqPerfQuery.begin();
    while (fit != pendingFreqPerfQuery.end())
    {
        FreqPerfQueryDX11* frame = *fit;

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

void ReleasePerfQueryPool()
{
    for (FreqPerfQueryDX11* frame : pendingFreqPerfQuery)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    pendingFreqPerfQuery.clear();

    for (FreqPerfQueryDX11* frame : freqPerfQueryPool)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    freqPerfQueryPool.clear();
}
}

//==============================================================================
} // namespace rhi
