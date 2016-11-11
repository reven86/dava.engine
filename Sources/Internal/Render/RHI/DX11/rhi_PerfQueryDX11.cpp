#include "../Common/rhi_Private.h"
#include "../rhi_Public.h"
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
    uint32 isUsed : 1;
    uint32 isReady : 1;
    uint32 isFreqValid : 1;
};

namespace PerfQueryDX11
{
struct PerfQueryFrameDX11
{
    ID3D11Query* freqQuery = nullptr;
    DAVA::List<HPerfQuery> perfQueries;
    uint64 freq = 0;
    bool isFreqValid = false;
};

PerfQueryFrameDX11* NextPerfQueryFrame();
void ObtainPerfQueryMeasurment(ID3D11DeviceContext* context);
void ReleasePerfQueryPool();
}

//==============================================================================

typedef ResourcePool<PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false> PerfQueryDX11Pool;
RHI_IMPL_POOL(PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false);

DAVA::List<PerfQueryDX11::PerfQueryFrameDX11*> pendingPerfQueryFrameDX11;
DAVA::Vector<PerfQueryDX11::PerfQueryFrameDX11*> perfQueryFramePoolDX11;
PerfQueryDX11::PerfQueryFrameDX11* currentPerfQueryFrameDX11 = nullptr;

//==============================================================================

static Handle dx11_PerfQuery_Create()
{
    Handle handle = PerfQueryDX11Pool::Alloc();
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        perfQuery->timestamp = 0;
        perfQuery->freq = 0;
        perfQuery->isUsed = 0;
        perfQuery->isReady = 0;
        perfQuery->isFreqValid = 0;

        DVASSERT(perfQuery->query == nullptr);

        HRESULT hr;
        D3D11_QUERY_DESC desc = {};
        desc.Query = D3D11_QUERY_TIMESTAMP;
        DX11_DEVICE_CALL(_D3D11_Device->CreateQuery(&desc, &(perfQuery->query)), hr);

        if (hr != S_OK)
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
        perfQuery->isFreqValid = 0;
        perfQuery->isReady = 0;
        perfQuery->isUsed = 0;
        perfQuery->timestamp = 0;
    }
}

static bool dx11_PerfQuery_IsReady(Handle handle)
{
    bool ret = false;
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);

    if (perfQuery)
    {
        ret = perfQuery->isReady;
    }

    return ret;
}

static uint64 dx11_PerfQuery_Value(Handle handle)
{
    uint64 ret = 0;
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery && perfQuery->isReady)
    {
        if (perfQuery->isFreqValid)
            ret = perfQuery->timestamp / (perfQuery->freq / 1000000); //mcs
        else
            ret = uint64(-1);
    }

    return ret;
}

namespace PerfQueryDX11
{
void IssueTimestampQuery(Handle handle, ID3D11DeviceContext* context)
{
    DVASSERT(currentPerfQueryFrameDX11);

    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery)
    {
        DVASSERT(perfQuery->query);
        DVASSERT(!perfQuery->isUsed);

        context->End(perfQuery->query);

        perfQuery->isUsed = 1;
        perfQuery->isReady = 0;

        currentPerfQueryFrameDX11->perfQueries.push_back(HPerfQuery(handle));
    }
}

void BeginMeasurment(ID3D11DeviceContext* context)
{
    DVASSERT(currentPerfQueryFrameDX11 == nullptr || !DeviceCaps().isPerfQuerySupported);

    currentPerfQueryFrameDX11 = NextPerfQueryFrame();

    if (currentPerfQueryFrameDX11)
        context->Begin(currentPerfQueryFrameDX11->freqQuery);
}

void EndMeasurment(ID3D11DeviceContext* context)
{
    DVASSERT(currentPerfQueryFrameDX11 || !DeviceCaps().isPerfQuerySupported);

    if (currentPerfQueryFrameDX11)
    {
        context->End(currentPerfQueryFrameDX11->freqQuery);

        pendingPerfQueryFrameDX11.push_back(currentPerfQueryFrameDX11);
        currentPerfQueryFrameDX11 = nullptr;
    }
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuery_Create = &dx11_PerfQuery_Create;
    dispatch->impl_PerfQuery_Delete = &dx11_PerfQuery_Delete;
    dispatch->impl_PerfQuery_Reset = &dx11_PerfQuery_Reset;
    dispatch->impl_PerfQuery_IsReady = &dx11_PerfQuery_IsReady;
    dispatch->impl_PerfQuery_Value = &dx11_PerfQuery_Value;
}

//==============================================================================

#if RHI_DX11__USE_DEFERRED_CONTEXTS
void DeferredPerfQueriesIssued(const std::vector<Handle>& queries)
{
    if (!queries.empty())
    {
        DVASSERT(currentPerfQueryFrameDX11);
        currentPerfQueryFrameDX11->perfQueries.insert(currentPerfQueryFrameDX11->perfQueries.end(), queries.begin(), queries.end());
    }
}

//==============================================================================

void IssueTimestampQueryDeferred(Handle handle, ID3D11DeviceContext* context)
{
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery)
    {
        DVASSERT(perfQuery->query);
        DVASSERT(!perfQuery->isUsed);

        context->End(perfQuery->query);

        perfQuery->isUsed = 1;
        perfQuery->isReady = 0;
    }
}
#endif

//==============================================================================

PerfQueryFrameDX11* NextPerfQueryFrame()
{
    PerfQueryFrameDX11* ret = nullptr;

    if (DeviceCaps().isPerfQuerySupported)
    {
        if (perfQueryFramePoolDX11.size())
        {
            ret = perfQueryFramePoolDX11.back();
            perfQueryFramePoolDX11.pop_back();
        }

        if (ret)
        {
            ret->freq = 0;
            ret->isFreqValid = false;
            ret->perfQueries.clear();
        }
        else
        {
            ret = new PerfQueryFrameDX11();

            HRESULT hr;
            D3D11_QUERY_DESC desc = {};
            desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
            DX11_DEVICE_CALL(_D3D11_Device->CreateQuery(&desc, &(ret->freqQuery)), hr);

            DVASSERT(ret->freqQuery);
        }
    }

    return ret;
}

//==============================================================================

void ObtainPerfQueryMeasurment(ID3D11DeviceContext* context)
{
    DAVA::List<PerfQueryFrameDX11*>::iterator fit = pendingPerfQueryFrameDX11.begin();
    while (fit != pendingPerfQueryFrameDX11.end())
    {
        PerfQueryFrameDX11* frame = *fit;

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
                query->isReady = 1;

                qit = frame->perfQueries.erase(qit);
            }
            else
            {
                ++qit;
            }
        }

        ++fit;
    }

    fit = pendingPerfQueryFrameDX11.begin();
    while (fit != pendingPerfQueryFrameDX11.end())
    {
        PerfQueryFrameDX11* frame = *fit;

        if (frame->freq && !frame->perfQueries.size())
        {
            perfQueryFramePoolDX11.push_back(frame);
            fit = pendingPerfQueryFrameDX11.erase(fit);
        }
        else
        {
            ++fit;
        }
    }
}

void ReleasePerfQueryPool()
{
    for (PerfQueryFrameDX11* frame : pendingPerfQueryFrameDX11)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    pendingPerfQueryFrameDX11.clear();

    for (PerfQueryFrameDX11* frame : perfQueryFramePoolDX11)
    {
        frame->freqQuery->Release();
        delete frame;
    }
    perfQueryFramePoolDX11.clear();
}
}

//==============================================================================
} // namespace rhi
