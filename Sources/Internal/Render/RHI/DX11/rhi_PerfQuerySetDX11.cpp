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

class
PerfQuerySetDX11_t
{
public:
    struct Desc
    {
    };
    PerfQuerySetDX11_t();
    ~PerfQuerySetDX11_t();

    struct
    timestamp_t
    {
        uint64 value;
        ID3D11Query* query;
        uint32 isUsed : 1;
        uint32 isReady : 1;
    };

    ID3D11Query* freq;
    uint64 freqValue;
    std::vector<timestamp_t> timestamp;

    ID3D11Query* frameT0Query;
    ID3D11Query* frameT1Query;
    uint64 frameT0;
    uint64 frameT1;

    uint32 freqDataRetrieved : 1;
    uint32 freqDataValid : 1;
    uint32 timestampsDataRetrieved : 1;
    uint32 frameTimestampsReady : 1;
};

typedef ResourcePool<PerfQuerySetDX11_t, RESOURCE_PERFQUERY_SET, PerfQuerySetDX11_t::Desc, false> PerfQuerySetDX11Pool;
RHI_IMPL_POOL(PerfQuerySetDX11_t, RESOURCE_PERFQUERY_SET, PerfQuerySetDX11_t::Desc, false);
static Handle DX11_CurFramePerfQuerySet = InvalidHandle;

//==============================================================================

PerfQuerySetDX11_t::PerfQuerySetDX11_t()
    : freq(nullptr)
    , freqDataRetrieved(false)
    , freqDataValid(false)
{
}

//------------------------------------------------------------------------------

PerfQuerySetDX11_t::~PerfQuerySetDX11_t()
{
}

static Handle
dx11_PerfQuerySet_Create(uint32 maxTimestampCount)
{
    Handle handle = PerfQuerySetDX11Pool::Alloc();
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        D3D11_QUERY_DESC desc;

        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;

        if (SUCCEEDED(_D3D11_Device->CreateQuery(&desc, &(set->freq))))
        {
            desc.Query = D3D11_QUERY_TIMESTAMP;
            desc.MiscFlags = 0;

            set->timestamp.resize(maxTimestampCount);
            for (std::vector<PerfQuerySetDX11_t::timestamp_t>::iterator t = set->timestamp.begin(), t_end = set->timestamp.end(); t != t_end; ++t)
            {
                t->isUsed = false;
                t->isReady = false;

                if (SUCCEEDED(_D3D11_Device->CreateQuery(&desc, &(t->query))))
                {
                }
                else
                {
                    t->query = nullptr;
                }
            }

            DVASSERT(SUCCEEDED(_D3D11_Device->CreateQuery(&desc, &(set->frameT0Query))));
            DVASSERT(SUCCEEDED(_D3D11_Device->CreateQuery(&desc, &(set->frameT1Query))));
        }
        else
        {
            PerfQuerySetDX11Pool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

static void
dx11_PerfQuerySet_Delete(Handle handle)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        for (std::vector<PerfQuerySetDX11_t::timestamp_t>::iterator t = set->timestamp.begin(), t_end = set->timestamp.end(); t != t_end; ++t)
        {
            if (t->query)
            {
                t->query->Release();
                t->query = nullptr;
            }
        }
        set->timestamp.clear();

        set->freq->Release();
        set->freq = nullptr;

        set->frameT0Query->Release();
        set->frameT0Query = nullptr;

        set->frameT1Query->Release();
        set->frameT1Query = nullptr;
    }

    PerfQuerySetDX11Pool::Free(handle);
}

static void
dx11_PerfQuerySet_Reset(Handle handle)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        set->freqDataRetrieved = false;
        set->freqDataValid = true;
        set->timestampsDataRetrieved = false;
        set->frameTimestampsReady = false;

        for (std::vector<PerfQuerySetDX11_t::timestamp_t>::iterator t = set->timestamp.begin(), t_end = set->timestamp.end(); t != t_end; ++t)
        {
            t->isUsed = false;
            t->isReady = false;
        }
    }
}

static void
dx11_PerfQuerySet_SetCurrent(Handle handle)
{
    DX11_CurFramePerfQuerySet = handle;
}

static void
dx11_PerfQuerySet_GetStatus(Handle handle, bool* IsReady, bool* isValid)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        *IsReady = set->freqDataRetrieved && set->timestampsDataRetrieved && set->frameTimestampsReady;
        *isValid = set->freqDataValid;
    }
}

static bool
dx11_PerfQuerySet_GetFreq(Handle handle, uint64* freq)
{
    bool retrieved = false;
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        *freq = set->freqValue;
        retrieved = set->freqDataValid;
    }

    return retrieved;
}

static bool
dx11_PerfQuerySet_GetTimestamp(Handle handle, uint32 timestampIndex, uint64* time)
{
    bool ready = false;
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set && timestampIndex < set->timestamp.size())
    {
        ready = set->timestamp[timestampIndex].isReady;
        *time = set->timestamp[timestampIndex].value;
    }

    return ready;
}

static bool
dx11_PerfQuerySet_GetFrameTimestamps(Handle handle, uint64* t0, uint64* t1)
{
    bool ready = false;
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        ready = set->frameTimestampsReady;
        *t0 = set->frameT0;
        *t1 = set->frameT1;
    }

    return ready;
}

namespace PerfQuerySetDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PerfQuerySet_Create = &dx11_PerfQuerySet_Create;
    dispatch->impl_PerfQuerySet_Delete = &dx11_PerfQuerySet_Delete;
    dispatch->impl_PerfQuerySet_Reset = &dx11_PerfQuerySet_Reset;
    dispatch->impl_PerfQuerySet_SetCurrent = &dx11_PerfQuerySet_SetCurrent;
    dispatch->impl_PerfQuerySet_GetStatus = &dx11_PerfQuerySet_GetStatus;
    dispatch->impl_PerfQuerySet_GetFreq = &dx11_PerfQuerySet_GetFreq;
    dispatch->impl_PerfQuerySet_GetTimestamp = &dx11_PerfQuerySet_GetTimestamp;
    dispatch->impl_PerfQuerySet_GetFrameTimestamps = &dx11_PerfQuerySet_GetFrameTimestamps;
}

void BeginFreqMeasurment(Handle handle, ID3D11DeviceContext* context)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        context->Begin(set->freq);
    }
}

void EndFreqMeasurment(Handle handle, ID3D11DeviceContext* context)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        context->End(set->freq);
    }
}

void IssueTimestampQuery(Handle handle, uint32 timestampIndex, ID3D11DeviceContext* context)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set && timestampIndex < set->timestamp.size())
    {
        ID3D11Query* iq = set->timestamp[timestampIndex].query;

        DVASSERT(iq);
        context->End(iq);
        set->timestamp[timestampIndex].isUsed = true;
        set->timestamp[timestampIndex].isReady = false;
    }
}

void IssueFrameBeginQuery(Handle handle, ID3D11DeviceContext* context)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        context->End(set->frameT0Query);
    }
}
void IssueFrameEndQuery(Handle handle, ID3D11DeviceContext* context)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        context->End(set->frameT1Query);
    }
}

Handle Current()
{
    return DX11_CurFramePerfQuerySet;
}

void ObtainResults(Handle handle)
{
    PerfQuerySetDX11_t* set = PerfQuerySetDX11Pool::Get(handle);

    if (set)
    {
        if (!set->freqDataRetrieved)
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data;
            HRESULT hr = _D3D11_ImmediateContext->GetData(set->freq, &data, sizeof(data), 0);

            if (hr == S_OK)
            {
                set->freqDataRetrieved = true;
                set->freqDataValid = !data.Disjoint;
                set->freqValue = data.Frequency;
            }
        }

        if (set->freqDataRetrieved && !set->timestampsDataRetrieved)
        {
            set->timestampsDataRetrieved = true;

            for (std::vector<PerfQuerySetDX11_t::timestamp_t>::iterator t = set->timestamp.begin(), t_end = set->timestamp.end(); t != t_end; ++t)
            {
                if (t->isUsed)
                {
                    if (!t->isReady)
                    {
                        HRESULT hr = _D3D11_ImmediateContext->GetData(t->query, &(t->value), sizeof(UINT64), 0);

                        if (hr == S_OK)
                            t->isReady = true;
                        else
                            set->timestampsDataRetrieved = false;
                    }
                }
            }
        }

        if (!set->frameTimestampsReady)
        {
            HRESULT hr0 = _D3D11_ImmediateContext->GetData(set->frameT0Query, &(set->frameT0), sizeof(UINT64), 0);
            HRESULT hr1 = _D3D11_ImmediateContext->GetData(set->frameT1Query, &(set->frameT1), sizeof(UINT64), 0);

            set->frameTimestampsReady = hr0 == S_OK && hr1 == S_OK;
        }
    }
}
}

//==============================================================================
} // namespace rhi
