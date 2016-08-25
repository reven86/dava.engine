#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_FormatConversion.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_dx11.h"

namespace rhi
{
//==============================================================================

struct RenderTargetViewDX11_t
{
    ID3D11RenderTargetView* view = nullptr;
    uint32 level = 0;
    TextureFace face = TEXTURE_FACE_NEGATIVE_X;

    RenderTargetViewDX11_t(ID3D11RenderTargetView* v, uint32 l, TextureFace f)
        : view(v)
        , level(l)
        , face(f)
    {
    }
};

struct TextureDX11_t
{
    TextureFormat format = TextureFormat::TEXTURE_FORMAT_R8G8B8A8;
    uint32 width = 0;
    uint32 height = 0;
    uint32 arraySize = 1;
    uint32 mipLevelCount = 0;
    uint32 lastUnit = DAVA::InvalidIndex;
    uint32 mappedLevel = 0;
    uint32 samples = 1;
    TextureFace mappedFace = TextureFace(-1);
    ID3D11Texture2D* tex2d = nullptr;
    ID3D11ShaderResourceView* tex2d_srv = nullptr;
    ID3D11DepthStencilView* tex2d_dsv = nullptr;
    ID3D11Texture2D* tex2d_copy = nullptr;
    void* mappedData = nullptr;
    std::vector<RenderTargetViewDX11_t> rt_view;

    bool isMapped = false;
    bool cpuAccessRead = false;

    ID3D11RenderTargetView* getRenderTargetView(uint32 level, TextureFace face);
};

ID3D11RenderTargetView* TextureDX11_t::getRenderTargetView(uint32 level, TextureFace face)
{
    ID3D11RenderTargetView* rtv = nullptr;

    for (const RenderTargetViewDX11_t& v : rt_view)
    {
        if (v.level == level && v.face == face)
        {
            rtv = v.view;
            break;
        }
    }

    if (rtv == nullptr)
    {
        D3D11_RENDER_TARGET_VIEW_DESC desc = {};

        desc.Format = DX11_TextureFormat(format);

        if (arraySize == 6)
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = level;
            desc.Texture2DArray.ArraySize = 1;

            switch (face)
            {
            case TEXTURE_FACE_POSITIVE_X:
                desc.Texture2DArray.FirstArraySlice = 0;
                break;
            case TEXTURE_FACE_NEGATIVE_X:
                desc.Texture2DArray.FirstArraySlice = 1;
                break;
            case TEXTURE_FACE_POSITIVE_Y:
                desc.Texture2DArray.FirstArraySlice = 2;
                break;
            case TEXTURE_FACE_NEGATIVE_Y:
                desc.Texture2DArray.FirstArraySlice = 3;
                break;
            case TEXTURE_FACE_POSITIVE_Z:
                desc.Texture2DArray.FirstArraySlice = 4;
                break;
            case TEXTURE_FACE_NEGATIVE_Z:
                desc.Texture2DArray.FirstArraySlice = 5;
                break;
            }
        }
        else
        {
            desc.ViewDimension = (samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
            if (samples == 1)
            {
                desc.Texture2D.MipSlice = level;
            }
        }

        HRESULT hr = _D3D11_Device->CreateRenderTargetView(tex2d, &desc, &rtv);
        CHECK_HR(hr)

        if (SUCCEEDED(hr))
        {
            rt_view.emplace_back(rtv, level, face);
        }
    }

    return rtv;
}

typedef ResourcePool<TextureDX11_t, RESOURCE_TEXTURE, Texture::Descriptor, true> TextureDX11Pool;
RHI_IMPL_POOL(TextureDX11_t, RESOURCE_TEXTURE, Texture::Descriptor, true);

//------------------------------------------------------------------------------

static Handle dx11_Texture_Create(const Texture::Descriptor& desc)
{
    DVASSERT(desc.levelCount);

    bool need_srv = true;
    bool need_dsv = false;
    bool need_copy = false;

    D3D11_TEXTURE2D_DESC desc2d = {};
    desc2d.Width = desc.width;
    desc2d.Height = desc.height;
    desc2d.MipLevels = desc.levelCount;
    desc2d.ArraySize = 1;
    desc2d.Format = DX11_TextureFormat(desc.format);
    desc2d.SampleDesc.Count = desc.samples;
    desc2d.SampleDesc.Quality = 0;
    desc2d.Usage = D3D11_USAGE_DEFAULT;
    desc2d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc2d.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
    desc2d.MiscFlags = 0;

    DVASSERT(desc2d.Format != DXGI_FORMAT_UNKNOWN);

    if (desc.type == TEXTURE_TYPE_CUBE)
    {
        DVASSERT(desc.samples == 1);
        desc2d.ArraySize = 6;
        desc2d.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    if (desc.autoGenMipmaps)
    {
        desc2d.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    if (desc.isRenderTarget)
    {
        desc2d.BindFlags |= D3D11_BIND_RENDER_TARGET;
        desc2d.MipLevels = 1;
    }

    if (desc.cpuAccessRead)
    {
        DVASSERT(desc.type == TEXTURE_TYPE_2D);
        DVASSERT(!desc.cpuAccessWrite);
        need_copy = true;
    }

    if (desc.format == TEXTURE_FORMAT_D16 || desc.format == TEXTURE_FORMAT_D24S8)
    {
        desc2d.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        need_srv = false;
        need_dsv = true;
    }

    bool useInitialData = false;
    D3D11_SUBRESOURCE_DATA data[128] = {};
    DVASSERT(countof(data) <= countof(desc.initialData));

    for (unsigned s = 0; s != desc2d.ArraySize; ++s)
    {
        for (unsigned m = 0; m != desc.levelCount; ++m)
        {
            uint32 di = s * desc.levelCount + m;
            if (desc.initialData[di])
            {
                data[di].pSysMem = desc.initialData[di];
                data[di].SysMemPitch = TextureStride(desc.format, Size2i(desc.width, desc.height), m);

                if (desc.format == TEXTURE_FORMAT_R8G8B8A8)
                {
                    _SwapRB8(desc.initialData[m], desc.initialData[m], TextureSize(desc.format, desc.width, desc.height, m));
                }
                else if (desc.format == TEXTURE_FORMAT_R4G4B4A4)
                {
                    _SwapRB4(desc.initialData[m], desc.initialData[m], TextureSize(desc.format, desc.width, desc.height, m));
                }
                else if (desc.format == TEXTURE_FORMAT_R5G5B5A1)
                {
                    _SwapRB5551(desc.initialData[m], desc.initialData[m], TextureSize(desc.format, desc.width, desc.height, m));
                }

                useInitialData = true;
            }
            else
            {
                break;
            }
        }
    }

    ID3D11Texture2D* tex2d = nullptr;
    HRESULT hr = _D3D11_Device->CreateTexture2D(&desc2d, (useInitialData) ? data : nullptr, &tex2d);
    CHECK_HR(hr)

    Handle handle = InvalidHandle;
    if (SUCCEEDED(hr))
    {
        handle = TextureDX11Pool::Alloc();
        TextureDX11_t* tex = TextureDX11Pool::Get(handle);

        tex->tex2d = tex2d;
        tex->format = desc.format;
        tex->width = desc.width;
        tex->height = desc.height;
        tex->arraySize = desc2d.ArraySize;
        tex->mipLevelCount = desc2d.MipLevels;
        tex->mappedData = nullptr;
        tex->cpuAccessRead = desc.cpuAccessRead;
        tex->isMapped = false;
        tex->samples = desc.samples;

        if (need_srv)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};

            srv_desc.Format = desc2d.Format;

            if (desc.type == TEXTURE_TYPE_CUBE)
            {
                DVASSERT(desc.samples == 1);
                srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                srv_desc.TextureCube.MipLevels = desc2d.MipLevels;
            }
            else if (desc.samples > 1)
            {
                srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MipLevels = desc2d.MipLevels;
            }

            ID3D11ShaderResourceView* srv = nullptr;
            hr = _D3D11_Device->CreateShaderResourceView(tex2d, &srv_desc, &srv);
            CHECK_HR(hr)

            if (SUCCEEDED(hr))
            {
                tex->tex2d_srv = srv;
            }
        }

        if (need_copy)
        {
            ID3D11Texture2D* copy = nullptr;

            desc2d.Usage = D3D11_USAGE_STAGING;
            desc2d.BindFlags = 0;
            desc2d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            hr = _D3D11_Device->CreateTexture2D(&desc2d, NULL, &copy);
            CHECK_HR(hr)

            if (SUCCEEDED(hr))
            {
                tex->tex2d_copy = copy;
            }
        }

        if (need_dsv)
        {
            DVASSERT(desc.type == TEXTURE_TYPE_2D);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
            dsv_desc.Format = desc2d.Format;
            dsv_desc.ViewDimension = (desc.samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

            ID3D11DepthStencilView* dsv = nullptr;
            hr = _D3D11_Device->CreateDepthStencilView(tex2d, &dsv_desc, &dsv);
            CHECK_HR(hr)

            if (SUCCEEDED(hr))
            {
                tex->tex2d_dsv = dsv;
            }
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

static void dx11_Texture_Delete(Handle tex)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);

    if (self->tex2d_srv)
    {
        self->tex2d_srv->Release();
        self->tex2d_srv = nullptr;
    }

    for (const RenderTargetViewDX11_t& v : self->rt_view)
    {
        v.view->Release();
    }
    self->rt_view.clear();

    if (self->tex2d_dsv)
    {
        self->tex2d_dsv->Release();
        self->tex2d_dsv = nullptr;
    }

    self->tex2d->Release();
    self->tex2d = nullptr;

    if (self->tex2d_copy)
    {
        self->tex2d_copy->Release();
        self->tex2d_copy = nullptr;
    }

    if (self->mappedData)
    {
        ::free(self->mappedData);
        self->mappedData = nullptr;
    }

    TextureDX11Pool::Free(tex);
}

//------------------------------------------------------------------------------

static void*
dx11_Texture_Map(Handle tex, unsigned level, TextureFace face)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);

    DVASSERT(!self->isMapped);

    if (self->cpuAccessRead)
    {
        DVASSERT(self->tex2d_copy);

        D3D11_MAPPED_SUBRESOURCE res = { 0 };
        DX11Command cmd[] =
        {
          { DX11Command::COPY_RESOURCE, { uint64(self->tex2d_copy), uint64(self->tex2d) } },
          { DX11Command::MAP, { uint64(self->tex2d_copy), 0, D3D11_MAP_READ, 0, uint64(&res) } }
        };
        ExecDX11(cmd, countof(cmd));
        CHECK_HR(cmd[1].retval);

        self->mappedData = res.pData;
        self->mappedLevel = level;
        self->isMapped = true;
    }
    else
    {
        self->mappedData = ::realloc(self->mappedData, TextureSize(self->format, self->width, self->height, level));
        self->mappedLevel = level;
        self->mappedFace = face;
        self->isMapped = true;
    }

    if (self->format == TEXTURE_FORMAT_R8G8B8A8)
    {
        _SwapRB8(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }
    else if (self->format == TEXTURE_FORMAT_R4G4B4A4)
    {
        _SwapRB4(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }
    else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
    {
        _SwapRB5551(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
    }

    return self->mappedData;
}

//------------------------------------------------------------------------------

static void
dx11_Texture_Unmap(Handle tex)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);

    DVASSERT(self->isMapped);

    if (self->cpuAccessRead)
    {
        DVASSERT(self->tex2d_copy);

        D3D11_MAPPED_SUBRESOURCE res = { 0 };
        DX11Command cmd[] =
        {
          { DX11Command::UNMAP, { uint64(self->tex2d_copy), 0 } }
        };
        ExecDX11(cmd, countof(cmd));

        self->isMapped = false;
        self->mappedData = nullptr;
    }
    else
    {
        if (self->format == TEXTURE_FORMAT_R8G8B8A8)
        {
            _SwapRB8(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
        }
        else if (self->format == TEXTURE_FORMAT_R4G4B4A4)
        {
            _SwapRB4(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
        }
        else if (self->format == TEXTURE_FORMAT_R5G5B5A1)
        {
            _SwapRB5551(self->mappedData, self->mappedData, TextureSize(self->format, self->width, self->height, self->mappedLevel));
        }

        uint32 rc_i = 0;
        uint32 face = 0;

        if (self->arraySize == 6)
        {
            switch (self->mappedFace)
            {
            case TEXTURE_FACE_POSITIVE_X:
                face = 0;
                break;
            case TEXTURE_FACE_NEGATIVE_X:
                face = 1;
                break;
            case TEXTURE_FACE_POSITIVE_Y:
                face = 2;
                break;
            case TEXTURE_FACE_NEGATIVE_Y:
                face = 3;
                break;
            case TEXTURE_FACE_POSITIVE_Z:
                face = 4;
                break;
            case TEXTURE_FACE_NEGATIVE_Z:
                face = 5;
                break;
            }

            rc_i = self->mappedLevel + (face * self->mipLevelCount);
        }
        else
        {
            rc_i = self->mappedLevel;
        }

        DX11Command cmd = { DX11Command::UPDATE_SUBRESOURCE, { uint64(self->tex2d), rc_i, NULL, uint64(self->mappedData), TextureStride(self->format, Size2i(self->width, self->height), self->mappedLevel), 0 } };
        ExecDX11(&cmd, 1);
        self->isMapped = false;

        ::free(self->mappedData);
        self->mappedData = nullptr;
    }
}

//------------------------------------------------------------------------------

void dx11_Texture_Update(Handle tex, const void* data, uint32 level, TextureFace face)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);
    void* dst = dx11_Texture_Map(tex, level, face);
    uint32 sz = TextureSize(self->format, self->width, self->height, level);

    memcpy(dst, data, sz);
    dx11_Texture_Unmap(tex);
}

TextureFormat dx11_Texture_GetFormat(Handle tex)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);
    return self->format;
}

//==============================================================================

namespace TextureDX11
{
void Init(uint32 maxCount)
{
    TextureDX11Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Texture_Create = &dx11_Texture_Create;
    dispatch->impl_Texture_Delete = &dx11_Texture_Delete;
    dispatch->impl_Texture_Map = &dx11_Texture_Map;
    dispatch->impl_Texture_Unmap = &dx11_Texture_Unmap;
    dispatch->impl_Texture_Update = &dx11_Texture_Update;
    dispatch->impl_Texture_GetFormat = &dx11_Texture_GetFormat;
}

void SetToRHIFragment(Handle tex, uint32 unit_i, ID3D11DeviceContext* context)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);

    context->PSSetShaderResources(unit_i, 1, &(self->tex2d_srv));
    self->lastUnit = unit_i;
}

void SetToRHIVertex(Handle tex, uint32 unit_i, ID3D11DeviceContext* context)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);

    context->VSSetShaderResources(unit_i, 1, &(self->tex2d_srv));
}

void SetRenderTarget(Handle color, Handle depthstencil, uint32 level, TextureFace face, ID3D11DeviceContext* context)
{
    bool hasDepthStencil = depthstencil != InvalidHandle;
    bool usesOwnDepthStencil = depthstencil != DefaultDepthBuffer;

    TextureDX11_t* rt = TextureDX11Pool::Get(color);
    TextureDX11_t* ds = (hasDepthStencil && usesOwnDepthStencil) ? TextureDX11Pool::Get(depthstencil) : nullptr;

    if (rt->lastUnit != DAVA::InvalidIndex)
    {
        ID3D11ShaderResourceView* srv[1] = {};
        context->PSSetShaderResources(rt->lastUnit, 1, srv);
        rt->lastUnit = DAVA::InvalidIndex;
    }

    ID3D11RenderTargetView* rtv = rt->getRenderTargetView(level, face);
    context->OMSetRenderTargets(1, &rtv, (ds) ? ds->tex2d_dsv : (usesOwnDepthStencil ? nullptr : _D3D11_DepthStencilView));
}

void SetAsDepthStencil(Handle tex)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);
}

Size2i Size(Handle tex)
{
    TextureDX11_t* self = TextureDX11Pool::Get(tex);
    return Size2i(self->width, self->height);
}

void ResolveMultisampling(Handle from, Handle to, ID3D11DeviceContext* context)
{
    TextureDX11_t* fromTexture = TextureDX11Pool::Get(from);
    DVASSERT(fromTexture != nullptr);

    ID3D11Resource* fromResource = nullptr;
    fromTexture->tex2d_srv->GetResource(&fromResource);

    DXGI_FORMAT fromFormat = DX11_TextureFormat(fromTexture->format);

    ID3D11Resource* toResource = nullptr;
    if (to == InvalidHandle)
    {
        _D3D11_RenderTargetView->GetResource(&toResource);

        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        _D3D11_RenderTargetView->GetDesc(&desc);
        DVASSERT(desc.Format = fromFormat);
    }
    else
    {
        TextureDX11_t* toTexture = TextureDX11Pool::Get(to);
        toTexture->tex2d_srv->GetResource(&toResource);

        DVASSERT(fromFormat == DX11_TextureFormat(toTexture->format));
    }
    DVASSERT(toResource != nullptr);

    UINT toIndex = D3D11CalcSubresource(0, 0, 1);
    UINT fromIndex = D3D11CalcSubresource(0, 0, 1);
    context->ResolveSubresource(toResource, toIndex, fromResource, fromIndex, fromFormat);

    fromResource->Release();
    toResource->Release();
}
}

} // namespace rhi
