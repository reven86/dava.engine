#include "rhi_Utils.h"

#include "Concurrency/Spinlock.h"

static DAVA::Spinlock _TraceSync;
static char _TraceBuf[4096];

void Trace(const char* format, ...)
{
#if 0
    _TraceSync.Lock();

    va_list  arglist;

    va_start(arglist, format);
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    _vsnprintf(_TraceBuf, countof(_TraceBuf), format, arglist);
#else
    vsnprintf(_TraceBuf, countof(_TraceBuf), format, arglist);
#endif
    va_end(arglist);

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    ::OutputDebugStringA(_TraceBuf);
#else
    puts(_TraceBuf);
#endif

    _TraceSync.Unlock();
#endif
}

namespace rhi
{
uint32 TextureStride(TextureFormat format, Size2i size, uint32 level)
{
    uint32 stride = 0;
    uint32 width = TextureExtents(size, level).dx;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    {
        stride = width * sizeof(uint32);
    }
    break;

    case TEXTURE_FORMAT_R8G8B8:
    {
        stride = width * 3 * sizeof(uint8);
    }
    break;

    case TEXTURE_FORMAT_R4G4B4A4:
    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
    case TEXTURE_FORMAT_R16:
    case TEXTURE_FORMAT_D16:
    {
        stride = width * sizeof(uint16);
    }
    break;

    case TEXTURE_FORMAT_R8:
    {
        stride = width * sizeof(uint8);
    }
    break;

    case TEXTURE_FORMAT_D24S8:
    {
        stride = width * sizeof(uint32);
    }
    break;

    case TEXTURE_FORMAT_DXT1:
    {
        stride = (width * 8) / 4;
    }
    break;

    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
    {
        stride = (width * 16) / 4;
    }
    break;

    default:
    {
    }
    }

    return stride;
}

//------------------------------------------------------------------------------

Size2i TextureExtents(Size2i size, uint32 level)
{
    Size2i sz(size.dx >> level, size.dy >> level);

    if (sz.dx == 0)
        sz.dx = 1;
    if (sz.dy == 0)
        sz.dy = 1;

    return sz;
}

//------------------------------------------------------------------------------

uint32 TextureSize(TextureFormat format, uint32 width, uint32 height, uint32 level)
{
    Size2i ext = TextureExtents(Size2i(width, height), level);
    uint32 sz = 0;

    switch (format)
    {
    case TEXTURE_FORMAT_R8G8B8A8:
    case TEXTURE_FORMAT_R8G8B8X8:
        sz = ext.dx * ext.dy * sizeof(uint32);
        break;

    case TEXTURE_FORMAT_R8G8B8:
        sz = ext.dx * ext.dy * 3 * sizeof(uint8);
        break;

    case TEXTURE_FORMAT_R5G5B5A1:
    case TEXTURE_FORMAT_R5G6B5:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_R4G4B4A4:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_A16R16G16B16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_A32R32G32B32:
        sz = ext.dx * ext.dy * sizeof(float32);
        break;

    case TEXTURE_FORMAT_R8:
        sz = ext.dx * ext.dy * sizeof(uint8);
        break;

    case TEXTURE_FORMAT_R16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_DXT1:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_DXT3:
    case TEXTURE_FORMAT_DXT5:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_PVRTC_4BPP_RGBA:
    {
        uint32 block_h = 8;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * (sizeof(uint64) * 4);
    }
    break;

    case TEXTURE_FORMAT_PVRTC_2BPP_RGBA:
    {
        uint32 block_h = 16;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * (sizeof(uint64) * 4);
    }
    break;

    case TEXTURE_FORMAT_PVRTC2_4BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA:
    {
        uint32 block_h = 4;
        uint32 block_w = 4;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * sizeof(uint64);
    }
    break;

    case TEXTURE_FORMAT_PVRTC2_2BPP_RGB:
    case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA:
    {
        uint32 block_h = 4;
        uint32 block_w = 8;

        sz = ((height + block_h - 1) / block_h) * ((width + block_w - 1) / block_w) * sizeof(uint64);
    }
    break;

    case TEXTURE_FORMAT_ATC_RGB:
        sz = ((ext.dx + 3) / 4) * ((ext.dy + 3) / 4) * 8;
        break;

    case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT:
    case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED:
        sz = ((ext.dx + 3) / 4) * ((ext.dy + 3) / 4) * 16;
        break;

    case TEXTURE_FORMAT_ETC1:
    case TEXTURE_FORMAT_ETC2_R8G8B8:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_ETC2_R8G8B8A8:
    case TEXTURE_FORMAT_ETC2_R8G8B8A1:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_EAC_R11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11_SIGNED:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 3;
    }
    break;

    case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED:
    case TEXTURE_FORMAT_EAC_R11G11_SIGNED:
    {
        int ww = ext.dx >> 2;
        int hh = ext.dy >> 2;

        if (!ww)
            ww = 1;
        if (!hh)
            hh = 1;

        sz = (ww * hh) << 4;
    }
    break;

    case TEXTURE_FORMAT_D16:
        sz = ext.dx * ext.dy * sizeof(uint16);
        break;

    case TEXTURE_FORMAT_D24S8:
        sz = ext.dx * ext.dy * sizeof(uint32);
        break;

    default:
        break;
    }

    return sz;
}
}
