#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_RingBuffer.h"
#include "../rhi_ShaderCache.h"
#include "rhi_DX11.h"
#include <D3D11Shader.h>
#include <D3Dcompiler.h>

namespace rhi
{
ID3D11InputLayout* _CreateInputLayout(const VertexLayout& layout, const void* code, uint32 code_sz)
{
    ID3D11InputLayout* vdecl = nullptr;
    D3D11_INPUT_ELEMENT_DESC elem[32];
    DAVA::uint32 elemCount = 0;

    DVASSERT(layout.ElementCount() < countof(elem));
    for (uint32 i = 0; i != layout.ElementCount(); ++i)
    {
        if (layout.ElementSemantics(i) == VS_PAD)
            continue;

        uint32 stream_i = layout.ElementStreamIndex(i);

        elem[elemCount].AlignedByteOffset = (UINT)(layout.ElementOffset(i));
        elem[elemCount].SemanticIndex = layout.ElementSemanticsIndex(i);
        elem[elemCount].InputSlot = stream_i;

        if (layout.StreamFrequency(stream_i) == VDF_PER_INSTANCE)
        {
            elem[elemCount].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            elem[elemCount].InstanceDataStepRate = 1;
        }
        else
        {
            elem[elemCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elem[elemCount].InstanceDataStepRate = 0;
        }

        switch (layout.ElementSemantics(i))
        {
        case VS_POSITION:
        {
            elem[elemCount].SemanticName = "POSITION";
        }
        break;

        case VS_NORMAL:
        {
            elem[elemCount].SemanticName = "NORMAL";
        }
        break;

        case VS_COLOR:
        {
            elem[elemCount].SemanticName = "COLOR";
        }
        break;

        case VS_TEXCOORD:
        {
            elem[elemCount].SemanticName = "TEXCOORD";
        }
        break;

        case VS_TANGENT:
        {
            elem[elemCount].SemanticName = "TANGENT";
        }
        break;

        case VS_BINORMAL:
        {
            elem[elemCount].SemanticName = "BINORMAL";
        }
        break;

        case VS_BLENDWEIGHT:
        {
            elem[elemCount].SemanticName = "BLENDWEIGHT";
        }
        break;

        case VS_BLENDINDEX:
        {
            elem[elemCount].SemanticName = "BLENDINDICES";
        }
        break;
        }

        switch (layout.ElementDataType(i))
        {
        case VDT_FLOAT:
            {
                switch (layout.ElementDataCount(i))
                {
                case 4:
                    elem[elemCount].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;
                case 3:
                    elem[elemCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    break;
                case 2:
                    elem[elemCount].Format = DXGI_FORMAT_R32G32_FLOAT;
                    break;
                case 1:
                    elem[elemCount].Format = DXGI_FORMAT_R32_FLOAT;
                    break;
                }
            }
            break;
        }

        if (layout.ElementSemantics(i) == VS_COLOR)
        {
            elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        ++elemCount;
    }

    DX11DeviceCommand(DX11Command::CREATE_INPUT_LAYOUT, elem, elemCount, code, code_sz, &vdecl);

    return vdecl;
}

ID3D11InputLayout* _CreateCompatibleInputLayout(const VertexLayout& vbLayout, const VertexLayout& vprogLayout, const void* code, uint32 code_sz)
{
    ID3D11InputLayout* vdecl = nullptr;
    D3D11_INPUT_ELEMENT_DESC elem[32];
    DAVA::uint32 elemCount = 0;

    DVASSERT(vbLayout.ElementCount() < countof(elem));
    for (uint32 i = 0; i != vprogLayout.ElementCount(); ++i)
    {
        DVASSERT(vprogLayout.ElementSemantics(i) != VS_PAD);

        uint32 vb_elem_i = DAVA::InvalidIndex;

        for (uint32 k = 0; k != vbLayout.ElementCount(); ++k)
        {
            if (vbLayout.ElementSemantics(k) == vprogLayout.ElementSemantics(i) && vbLayout.ElementSemanticsIndex(k) == vprogLayout.ElementSemanticsIndex(i))
            {
                vb_elem_i = k;
                break;
            }
        }

        if (vb_elem_i != DAVA::InvalidIndex)
        {
            uint32 stream_i = vprogLayout.ElementStreamIndex(i);

            elem[elemCount].AlignedByteOffset = (UINT)(vbLayout.ElementOffset(vb_elem_i));
            elem[elemCount].SemanticIndex = vprogLayout.ElementSemanticsIndex(i);
            elem[elemCount].InputSlot = stream_i;

            if (vprogLayout.StreamFrequency(stream_i) == VDF_PER_INSTANCE)
            {
                elem[elemCount].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                elem[elemCount].InstanceDataStepRate = 1;
            }
            else
            {
                elem[elemCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                elem[elemCount].InstanceDataStepRate = 0;
            }

            switch (vbLayout.ElementSemantics(vb_elem_i))
            {
            case VS_POSITION:
            {
                elem[elemCount].SemanticName = "POSITION";
            }
            break;

            case VS_NORMAL:
            {
                elem[elemCount].SemanticName = "NORMAL";
            }
            break;

            case VS_COLOR:
            {
                elem[elemCount].SemanticName = "COLOR";
            }
            break;

            case VS_TEXCOORD:
            {
                elem[elemCount].SemanticName = "TEXCOORD";
            }
            break;

            case VS_TANGENT:
            {
                elem[elemCount].SemanticName = "TANGENT";
            }
            break;

            case VS_BINORMAL:
            {
                elem[elemCount].SemanticName = "BINORMAL";
            }
            break;

            case VS_BLENDWEIGHT:
            {
                elem[elemCount].SemanticName = "BLENDWEIGHT";
            }
            break;

            case VS_BLENDINDEX:
            {
                elem[elemCount].SemanticName = "BLENDINDICES";
            }
            break;
            }

            switch (vbLayout.ElementDataType(vb_elem_i))
            {
            case VDT_FLOAT:
                {
                    switch (vbLayout.ElementDataCount(vb_elem_i))
                    {
                    case 4:
                        elem[elemCount].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                        break;
                    case 3:
                        elem[elemCount].Format = DXGI_FORMAT_R32G32B32_FLOAT;
                        break;
                    case 2:
                        elem[elemCount].Format = DXGI_FORMAT_R32G32_FLOAT;
                        break;
                    case 1:
                        elem[elemCount].Format = DXGI_FORMAT_R32_FLOAT;
                        break;
                    }
                }
                break;
            }

            if (vbLayout.ElementSemantics(vb_elem_i) == VS_COLOR)
            {
                elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }
        else
        {
            DVASSERT(!"kaboom!");
        }

        ++elemCount;
    }

    if (vprogLayout.Stride() < vbLayout.Stride())
    {
        const uint32 padCnt = vbLayout.Stride() - vprogLayout.Stride();

        DVASSERT(padCnt % 4 == 0);
        for (uint32 p = 0; p != padCnt / 4; ++p)
        {
            elem[elemCount].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; //vprogLayout.Stride() + p;
            elem[elemCount].SemanticIndex = p;
            elem[elemCount].SemanticName = "PAD";
            elem[elemCount].InputSlot = 0;
            elem[elemCount].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elem[elemCount].InstanceDataStepRate = 0;
            elem[elemCount].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            ++elemCount;
        }
    }

    DX11DeviceCommand(DX11Command::CREATE_INPUT_LAYOUT, elem, elemCount, code, code_sz, &vdecl);

    return vdecl;
}

class ConstBufDX11
{
public:
    static RingBuffer defaultRingBuffer;
    static uint32 currentFrame;

    enum : uint32
    {
        REGISTER_SIZE = 4 * sizeof(float)
    };

public:
    struct Desc
    {
    };

    void Construct(ProgType type, uint32 buf_i, uint32 reg_count);
    void Destroy();

    uint32 ConstCount();

    bool SetConst(uint32 const_i, uint32 count, const float* data);
    bool SetConst(uint32 const_i, uint32 const_sub_i, const float* data, uint32 dataCount);
    void SetToRHI(ID3D11DeviceContext* context, ID3D11Buffer** buffer);
    void Invalidate();
    void SetToRHI(const void* instData);

    const void* Instance();

private:
    ProgType progType = PROG_VERTEX;
    ID3D11Buffer* buffer = nullptr;
    float* value = nullptr;
    float* inst = nullptr;
    uint32 frame = 0;
    uint32 buf_i = DAVA::InvalidIndex;
    uint32 regCount = 0;
    bool updatePending = true;
};

RingBuffer ConstBufDX11::defaultRingBuffer;
uint32 ConstBufDX11::currentFrame = 0;

void ConstBufDX11::Construct(ProgType ptype, uint32 bufIndex, uint32 regCnt)
{
    DVASSERT(value == nullptr);
    DVASSERT(bufIndex != DAVA::InvalidIndex);
    DVASSERT(regCnt);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = regCnt * REGISTER_SIZE;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (DX11DeviceCommand(DX11Command::CREATE_BUFFER, &desc, NULL, &buffer))
    {
        value = reinterpret_cast<float*>(calloc(regCnt, REGISTER_SIZE));
        progType = ptype;
        buf_i = bufIndex;
        regCount = regCnt;
        updatePending = true;
    }
}

void ConstBufDX11::Destroy()
{
    DAVA::SafeRelease(buffer);
    if (value)
        ::free(value);

    frame = 0;
    regCount = 0;
    inst = nullptr;
    value = nullptr;
    updatePending = false;
    buf_i = DAVA::InvalidIndex;
}

uint32 ConstBufDX11::ConstCount()
{
    return regCount;
}

bool ConstBufDX11::SetConst(uint32 const_i, uint32 const_count, const float* data)
{
    DVASSERT(const_i + const_count <= regCount);

    memcpy(value + 4 * const_i, data, const_count * REGISTER_SIZE);
    if (_DX11_UseHardwareCommandBuffers)
        updatePending = true;
    else
        inst = nullptr;

    return true;
}

bool ConstBufDX11::SetConst(uint32 const_i, uint32 const_sub_i, const float* data, uint32 dataCount)
{
    DVASSERT(const_i <= regCount && const_sub_i < 4);

    memcpy(value + const_i * 4 + const_sub_i, data, dataCount * sizeof(float));
    if (_DX11_UseHardwareCommandBuffers)
        updatePending = true;
    else
        inst = nullptr;

    return true;
}

void ConstBufDX11::SetToRHI(ID3D11DeviceContext* context, ID3D11Buffer** outBuffer)
{
    if (updatePending)
    {
        DVASSERT(_DX11_UseHardwareCommandBuffers);
        context->UpdateSubresource(buffer, 0, nullptr, value, regCount * REGISTER_SIZE, 0);
        updatePending = false;
    }
    outBuffer[buf_i] = buffer;
}

void ConstBufDX11::SetToRHI(const void* instData)
{
    _D3D11_ImmediateContext->UpdateSubresource(buffer, 0, nullptr, instData, regCount * REGISTER_SIZE, 0);

    if (progType == PROG_VERTEX)
        _D3D11_ImmediateContext->VSSetConstantBuffers(buf_i, 1, &buffer);
    else
        _D3D11_ImmediateContext->PSSetConstantBuffers(buf_i, 1, &buffer);
}

const void* ConstBufDX11::Instance()
{
    if ((inst == nullptr) || (frame != currentFrame))
    {
        inst = defaultRingBuffer.Alloc(regCount * REGISTER_SIZE);
        memcpy(inst, value, regCount * REGISTER_SIZE);
        frame = currentFrame;
    }
    return inst;
}

void ConstBufDX11::Invalidate()
{
    updatePending = true;
}

static void DumpShaderText(const char* code, uint32 code_sz)
{
    if (code_sz >= RHI_SHADER_SOURCE_BUFFER_SIZE)
    {
        DAVA::Logger::Info(code);
        return;
    }

    char src[RHI_SHADER_SOURCE_BUFFER_SIZE] = {};
    char* src_line[1024] = {};
    uint32 line_cnt = 0;
    memcpy(src, code, code_sz);

    src_line[line_cnt++] = src;
    for (char* s = src; *s;)
        {
            if (*s == '\n')
            {
                *s = 0;
                ++s;

                while (*s && (/**s == '\n'  ||  */ *s == '\r'))
                {
                    *s = 0;
                    ++s;
                }

                if (!(*s))
                    break;

                src_line[line_cnt] = s;
                ++line_cnt;
            }
            else if (*s == '\r')
            {
                *s = ' ';
            }
            else
            {
                ++s;
            }
        }

        for (uint32 i = 0; i != line_cnt; ++i)
        {
            DAVA::Logger::Info("%4u |  %s", 1 + i, src_line[i]);
        }
    }

struct PipelineStateDX11_t
{
    struct LayoutInfo
    {
        ID3D11InputLayout* inputLayout = nullptr;
        uint32 layoutUID = 0;
    };

    Handle CreateConstBuffer(ProgType type, uint32 buf_i);

    PipelineState::Descriptor desc;

    ID3D11InputLayout* inputLayout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11BlendState* blendState;
    ID3D10Blob* vpCode;

    uint32 vertexBufCount;
    uint32 vertexBufRegCount[16];
    uint32 fragmentBufCount;
    uint32 fragmentBufRegCount[16];

    VertexLayout vertexLayout;
    DAVA::Vector<LayoutInfo> altLayout;
    DAVA::Vector<uint8> dbgVertexSrc;
    DAVA::Vector<uint8> dbgPixelSrc;
};

typedef ResourcePool<PipelineStateDX11_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false> PipelineStateDX11Pool;
RHI_IMPL_POOL(PipelineStateDX11_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false);

typedef ResourcePool<ConstBufDX11, RESOURCE_CONST_BUFFER, ConstBufDX11::Desc, false> ConstBufDX11Pool;
RHI_IMPL_POOL_SIZE(ConstBufDX11, RESOURCE_CONST_BUFFER, ConstBufDX11::Desc, false, 12 * 1024);

Handle PipelineStateDX11_t::CreateConstBuffer(ProgType type, uint32 buf_i)
{
    Handle handle = ConstBufDX11Pool::Alloc();
    ConstBufDX11* cb = ConstBufDX11Pool::Get(handle);
    cb->Construct(type, buf_i, (type == PROG_VERTEX) ? vertexBufRegCount[buf_i] : fragmentBufRegCount[buf_i]);
    return handle;
}

static Handle dx11_PipelineState_Create(const PipelineState::Descriptor& desc)
{
    bool success = false;
    Handle handle = PipelineStateDX11Pool::Alloc();
    PipelineStateDX11_t* ps = PipelineStateDX11Pool::Get(handle);
    HRESULT hr;
    static std::vector<uint8> vprog_bin;
    static std::vector<uint8> fprog_bin;
    ID3D10Blob* vp_code = nullptr;
    ID3D10Blob* vp_err = nullptr;
    ID3D10Blob* fp_code = nullptr;
    ID3D10Blob* fp_err = nullptr;

#if 0
    DAVA::Logger::Info("create PS");
    DAVA::Logger::Info("  vprog= %s", desc.vprogUid.c_str());
    DAVA::Logger::Info("  fprog= %s", desc.vprogUid.c_str());
    desc.vertexLayout.Dump();
#endif

    rhi::ShaderCache::GetProg(desc.vprogUid, &vprog_bin);
    rhi::ShaderCache::GetProg(desc.fprogUid, &fprog_bin);

#if 0
    DumpShaderText((const char*)(&vprog_bin[0]), (uint32 int)vprog_bin.size());
    DumpShaderText((const char*)(&fprog_bin[0]), (uint32 int)fprog_bin.size());
#endif

    const char* vsFeatureLevel = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_11_0) ? "vs_4_0" : "vs_4_0_level_9_1";
    const char* fsFeatureLevel = (_D3D11_FeatureLevel >= D3D_FEATURE_LEVEL_11_0) ? "ps_4_0" : "ps_4_0_level_9_1";

    hr = D3DCompile((const char*)(&vprog_bin[0]), vprog_bin.size(), "vprog", nullptr, nullptr, "vp_main", vsFeatureLevel,
                    D3DCOMPILE_OPTIMIZATION_LEVEL2, 0, &vp_code, &vp_err);

    if (DX11Check(hr))
    {
        if (DX11DeviceCommand(DX11Command::CREATE_VERTEX_SHADER, vp_code->GetBufferPointer(), vp_code->GetBufferSize(), NULL, &(ps->vertexShader)))
        {
            ID3D11ShaderReflection* reflection = nullptr;
            hr = D3DReflect(vp_code->GetBufferPointer(), vp_code->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
            if (DX11Check(hr))
            {
                D3D11_SHADER_DESC desc = {};
                hr = reflection->GetDesc(&desc);
                if (DX11Check(hr))
                {
                    ps->vertexBufCount = desc.ConstantBuffers;

                    for (uint32 b = 0; b != desc.ConstantBuffers; ++b)
                    {
                        ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(b);
                        if (cb)
                        {
                            D3D11_SHADER_BUFFER_DESC cb_desc = {};
                            hr = cb->GetDesc(&cb_desc);
                            if (DX11Check(hr))
                            {
                                ps->vertexBufRegCount[b] = cb_desc.Size / ConstBufDX11::REGISTER_SIZE;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            ps->vertexShader = nullptr;
        }
    }
    else
    {
        DAVA::Logger::Error("FAILED to compile vertex-shader:");
        if (vp_err)
        {
            DAVA::Logger::Info((const char*)(vp_err->GetBufferPointer()));
        }
        DAVA::Logger::Error("shader-uid : %s", desc.vprogUid.c_str());
        DAVA::Logger::Error("vertex-shader text:\n");
        DumpShaderText((const char*)(&vprog_bin[0]), (uint32)vprog_bin.size());
        ps->vertexShader = nullptr;
        DVASSERT_MSG(ps->vertexShader, desc.vprogUid.c_str());
    }

    hr = D3DCompile((const char*)(&fprog_bin[0]), fprog_bin.size(), "fprog", nullptr, nullptr, "fp_main", fsFeatureLevel,
                    D3DCOMPILE_OPTIMIZATION_LEVEL2, 0, &fp_code, &fp_err);

    if (DX11Check(hr))
    {
        if (DX11DeviceCommand(DX11Command::CREATE_PIXEL_SHADER, fp_code->GetBufferPointer(), fp_code->GetBufferSize(), NULL, &(ps->pixelShader)))
        {
            ID3D11ShaderReflection* reflection = nullptr;
            hr = D3DReflect(fp_code->GetBufferPointer(), fp_code->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
            if (DX11Check(hr))
            {
                D3D11_SHADER_DESC desc = {};
                hr = reflection->GetDesc(&desc);
                if (DX11Check(hr))
                {
                    ps->fragmentBufCount = desc.ConstantBuffers;
                    for (uint32 b = 0; b != desc.ConstantBuffers; ++b)
                    {
                        ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(b);
                        if (cb)
                        {
                            D3D11_SHADER_BUFFER_DESC cb_desc = {};
                            hr = cb->GetDesc(&cb_desc);
                            if (DX11Check(hr))
                            {
                                ps->fragmentBufRegCount[b] = cb_desc.Size / ConstBufDX11::REGISTER_SIZE;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            ps->pixelShader = nullptr;
            DVASSERT_MSG(ps->pixelShader, desc.fprogUid.c_str());
        }
    }
    else
    {
        DAVA::Logger::Error("FAILED to compile pixel-shader:");
        if (fp_err)
        {
            DAVA::Logger::Info((const char*)(fp_err->GetBufferPointer()));
        }
        DAVA::Logger::Error("shader-uid : %s", desc.fprogUid.c_str());
        DAVA::Logger::Error("vertex-shader text:\n");
        DumpShaderText((const char*)(&fprog_bin[0]), (uint32)fprog_bin.size());
        ps->pixelShader = nullptr;
    }

    if (ps->vertexShader && ps->pixelShader)
    {
        ps->vpCode = vp_code;
        ps->inputLayout = _CreateInputLayout(desc.vertexLayout, vp_code->GetBufferPointer(), static_cast<uint32>(vp_code->GetBufferSize()));
        ps->vertexLayout = desc.vertexLayout;
        DVASSERT(ps->inputLayout);

        if (ps->inputLayout)
        {
            ps->dbgVertexSrc = vprog_bin;
            ps->dbgPixelSrc = fprog_bin;

            UINT8 mask = 0;

            if (desc.blending.rtBlend[0].writeMask & COLORMASK_R)
                mask |= D3D11_COLOR_WRITE_ENABLE_RED;
            if (desc.blending.rtBlend[0].writeMask & COLORMASK_G)
                mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
            if (desc.blending.rtBlend[0].writeMask & COLORMASK_B)
                mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
            if (desc.blending.rtBlend[0].writeMask & COLORMASK_A)
                mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

            D3D11_BLEND_DESC bs_desc = {};
            bs_desc.AlphaToCoverageEnable = FALSE;
            bs_desc.IndependentBlendEnable = FALSE;
            bs_desc.RenderTarget[0].BlendEnable = desc.blending.rtBlend[0].blendEnabled;
            bs_desc.RenderTarget[0].RenderTargetWriteMask = mask;
            bs_desc.RenderTarget[0].SrcBlend = DX11_BlendOp(BlendOp(desc.blending.rtBlend[0].colorSrc));
            bs_desc.RenderTarget[0].DestBlend = DX11_BlendOp(BlendOp(desc.blending.rtBlend[0].colorDst));
            bs_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            bs_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            bs_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            bs_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            if (DX11DeviceCommand(DX11Command::CREATE_BLEND_STATE, &bs_desc, &ps->blendState))
            {
                ps->desc = desc;
                success = true;
            }
        }
    }

    if (!success)
    {
        PipelineStateDX11Pool::Free(handle);
        handle = InvalidHandle;
    }

    return handle;
}

static void dx11_PipelineState_Delete(Handle ps)
{
    PipelineStateDX11Pool::Free(ps);
}

static Handle dx11_PipelineState_CreateVertexConstBuffer(Handle ps, uint32 buf_i)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);
    return ps11->CreateConstBuffer(PROG_VERTEX, buf_i);
}

static Handle dx11_PipelineState_CreateFragmentConstBuffer(Handle ps, uint32 buf_i)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);
    return ps11->CreateConstBuffer(PROG_FRAGMENT, buf_i);
}

static bool dx11_ConstBuffer_SetConst(Handle cb, uint32 const_i, uint32 const_count, const float* data)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);
    return cb11->SetConst(const_i, const_count, data);
}

static bool dx11_ConstBuffer_SetConst1fv(Handle cb, uint32 const_i, uint32 const_sub_i, const float* data, uint32 dataCount)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);

    return cb11->SetConst(const_i, const_sub_i, data, dataCount);
}

void dx11_ConstBuffer_Delete(Handle cb)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);
    cb11->Destroy();
    ConstBufDX11Pool::Free(cb);
}

void PipelineStateDX11::SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PipelineState_Create = &dx11_PipelineState_Create;
    dispatch->impl_PipelineState_Delete = &dx11_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer = &dx11_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer = &dx11_PipelineState_CreateFragmentConstBuffer;
}

void PipelineStateDX11::SetToRHI(Handle ps, uint32 layoutUID, ID3D11DeviceContext* context)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);

    ID3D11InputLayout* layout11 = nullptr;
    if (layoutUID == VertexLayout::InvalidUID)
    {
        layout11 = ps11->inputLayout;
    }
    else
    {
        for (const PipelineStateDX11_t::LayoutInfo& l : ps11->altLayout)
        {
            if (l.layoutUID == layoutUID)
            {
                layout11 = l.inputLayout;
                break;
            }
        }

        if (layout11 == nullptr)
        {
            const VertexLayout* vbLayout = VertexLayout::Get(layoutUID);
            PipelineStateDX11_t::LayoutInfo info;
            VertexLayout layout;
            layout11 = _CreateCompatibleInputLayout(*vbLayout, ps11->vertexLayout, ps11->vpCode->GetBufferPointer(), static_cast<uint32>(ps11->vpCode->GetBufferSize()));

            if (layout11)
            {
                info.inputLayout = layout11;
                info.layoutUID = layoutUID;
                ps11->altLayout.push_back(info);
            }
            else
            {
                DAVA::Logger::Error("can't create compatible vertex-layout");
                DAVA::Logger::Info("vprog-layout:");
                ps11->vertexLayout.Dump();
                DAVA::Logger::Info("custom-layout:");
                vbLayout->Dump();
            }
        }
    }

    context->IASetInputLayout(layout11);
    context->VSSetShader(ps11->vertexShader, nullptr, 0);
    context->PSSetShader(ps11->pixelShader, nullptr, 0);
    context->OMSetBlendState(ps11->blendState, nullptr, 0xFFFFFFFF);
}

uint32 PipelineStateDX11::VertexLayoutStride(Handle ps, uint32 stream_i)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);
    return ps11->vertexLayout.Stride(stream_i);
}

uint32 PipelineStateDX11::VertexLayoutStreamCount(Handle ps)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);
    return ps11->vertexLayout.StreamCount();
}

void PipelineStateDX11::GetConstBufferCount(Handle ps, uint32* vertexBufCount, uint32* fragmentBufCount)
{
    PipelineStateDX11_t* ps11 = PipelineStateDX11Pool::Get(ps);
    *vertexBufCount = ps11->vertexBufCount;
    *fragmentBufCount = ps11->fragmentBufCount;
}

void ConstBufferDX11::Init(uint32 maxCount)
{
    ConstBufDX11Pool::Reserve(maxCount);
}

void ConstBufferDX11::SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_ConstBuffer_SetConst = &dx11_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv = &dx11_ConstBuffer_SetConst1fv;
    dispatch->impl_ConstBuffer_Delete = &dx11_ConstBuffer_Delete;
}

void ConstBufferDX11::InvalidateAll()
{
    for (ConstBufDX11Pool::Iterator cb = ConstBufDX11Pool::Begin(), cb_end = ConstBufDX11Pool::End(); cb != cb_end; ++cb)
    {
        cb->Invalidate();
    }
}

void ConstBufferDX11::SetToRHI(Handle cb, ID3D11DeviceContext* context, ID3D11Buffer** buffer)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);
    cb11->SetToRHI(context, buffer);
}

void ConstBufferDX11::SetToRHI(Handle cb, const void* instData)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);
    cb11->SetToRHI(instData);
}

const void* ConstBufferDX11::Instance(Handle cb)
{
    ConstBufDX11* cb11 = ConstBufDX11Pool::Get(cb);
    return cb11->Instance();
}

void ConstBufferDX11::InvalidateAllInstances()
{
    ++ConstBufDX11::currentFrame;
}

void ConstBufferDX11::InitializeRingBuffer(uint32 size)
{
    ConstBufDX11::defaultRingBuffer.Initialize((size) ? size : 4 * 1024 * 1024);
}

} // namespace rhi
