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
    rhi::Handle frameQuery0 = InvalidHandle;
    rhi::Handle frameQuery1 = InvalidHandle;
    bool isFreqValid = false;
};
}

//==============================================================================

typedef ResourcePool<PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false> PerfQueryDX11Pool;
RHI_IMPL_POOL(PerfQueryDX11_t, RESOURCE_PERFQUERY, PerfQueryDX11_t::Desc, false);

DAVA::Mutex perfQueryMutexDX11;
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

        DVASSERT(perfQuery->query == nullptr)

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        desc.MiscFlags = 0;

        HRESULT hr;
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

static void dx11_PerfQuery_SetCurrent(Handle start, Handle end)
{
    DVASSERT(currentPerfQueryFrameDX11);
    currentPerfQueryFrameDX11->frameQuery0 = start;
    currentPerfQueryFrameDX11->frameQuery1 = end;
}

namespace PerfQueryDX11
{
void IssueTimestamp(PerfQueryFrameDX11* frame, Handle handle, ID3D11DeviceContext* context)
{
    PerfQueryDX11_t* perfQuery = PerfQueryDX11Pool::Get(handle);
    if (perfQuery)
    {
        DVASSERT(perfQuery->query);
        DVASSERT(!perfQuery->isUsed);

        context->End(perfQuery->query);

        perfQuery->isUsed = 1;
        perfQuery->isReady = 0;

        frame->perfQueries.push_back(HPerfQuery(handle));
    }
}

void BeginMeasurment(PerfQueryFrameDX11* frame, ID3D11DeviceContext* context)
{
    context->Begin(frame->freqQuery);
}

void EndMeasurment(PerfQueryFrameDX11* frame, ID3D11DeviceContext* context)
{
    context->End(frame->freqQuery);

    pendingPerfQueryFrameDX11.push_back(frame);
}

void BeginFrame(PerfQueryFrameDX11* frame, ID3D11DeviceContext* context)
{
    if (frame->frameQuery0)
        IssueTimestamp(frame, frame->frameQuery0, context);
}

void EndFrame(PerfQueryFrameDX11* frame, ID3D11DeviceContext* context)
{
    if (frame->frameQuery1)
        IssueTimestamp(frame, frame->frameQuery1, context);
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

PerfQueryFrameDX11* NextPerfQueryFrame()
{
    currentPerfQueryFrameDX11 = nullptr;

    perfQueryMutexDX11.Lock();
    if (perfQueryFramePoolDX11.size())
    {
        currentPerfQueryFrameDX11 = perfQueryFramePoolDX11.back();
        perfQueryFramePoolDX11.pop_back();
    }
    perfQueryMutexDX11.Unlock();

    if (currentPerfQueryFrameDX11)
    {
        currentPerfQueryFrameDX11->freq = 0;
        currentPerfQueryFrameDX11->isFreqValid = false;
        currentPerfQueryFrameDX11->perfQueries.clear();
        currentPerfQueryFrameDX11->frameQuery0 = InvalidHandle;
        currentPerfQueryFrameDX11->frameQuery1 = InvalidHandle;
    }
    else
    {
        currentPerfQueryFrameDX11 = new PerfQueryFrameDX11();

        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;

        HRESULT hr;
        DX11_DEVICE_CALL(_D3D11_Device->CreateQuery(&desc, &(currentPerfQueryFrameDX11->freqQuery)), hr);
    }

    return currentPerfQueryFrameDX11;
}

PerfQueryFrameDX11* CurrentPerfQueryFrame()
{
    return currentPerfQueryFrameDX11;
}

//==============================================================================
//called from render-thread

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

    DAVA::LockGuard<DAVA::Mutex> guard(perfQueryMutexDX11);

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
