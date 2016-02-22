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

#include "Render/Image/ImageConvert.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/Image.h"


namespace DAVA {
namespace ImageConvert {

bool Normalize(PixelFormat format, const void* inData, uint32 width, uint32 height, uint32 pitch, void* outData)
{
    if (format == FORMAT_RGBA8888)
    {
        ConvertDirect<uint32, uint32, NormalizeRGBA8888> convert;
        convert(inData, width, height, pitch, outData, width, height, pitch);

        return true;
    }

    Logger::Error("Normalize function not implemented for %s", PixelFormatDescriptor::GetPixelFormatString(format));
    return false;
}

bool ConvertImage(const Image *srcImage, Image *dstImage)
{
    DVASSERT(srcImage);
    DVASSERT(dstImage);
    DVASSERT(srcImage->format != dstImage->format);
    DVASSERT(srcImage->width == dstImage->width);
    DVASSERT(srcImage->height == dstImage->height);

    PixelFormat srcFormat = srcImage->format;
    PixelFormat dstFormat = dstImage->format;

    bool srcCompressed = LibDdsHelper::IsSupportedCompressedFormat(srcFormat);
    bool dstCompressed = LibDdsHelper::IsSupportedCompressedFormat(dstFormat);

    if (!srcCompressed && !dstCompressed)
    {
        return ConvertImageDirect(srcImage, dstImage);
    }
    else if (srcCompressed && dstCompressed)
    {
        ScopedPtr<Image> rgba(Image::Create(srcImage->width, srcImage->height, FORMAT_RGBA8888));
        return (LibDdsHelper::DecompressToRGBA(srcImage, rgba) && LibDdsHelper::CompressFromRGBA(rgba, dstImage));
    }
    else if (srcCompressed)
    {
        if (dstFormat == FORMAT_RGBA8888)
        {
            return LibDdsHelper::DecompressToRGBA(srcImage, dstImage);
        }
        else
        {
            ScopedPtr<Image> rgba(Image::Create(srcImage->width, srcImage->height, FORMAT_RGBA8888));
            return (LibDdsHelper::DecompressToRGBA(srcImage, rgba) && ConvertImageDirect(rgba, dstImage));
        }

    }
    else
    {
        if (srcFormat == FORMAT_RGBA8888)
        {
            return LibDdsHelper::CompressFromRGBA(srcImage, dstImage);
        }
        else
        {
            ScopedPtr<Image> rgba(Image::Create(srcImage->width, srcImage->height, FORMAT_RGBA8888));
            return (ConvertImageDirect(srcImage, rgba) && LibDdsHelper::CompressFromRGBA(rgba, dstImage));
        }
    }
}

bool ConvertImageDirect(const Image* srcImage, Image* dstImage)
{
    return ConvertImageDirect(srcImage->format, dstImage->format,
        srcImage->data, srcImage->width, srcImage->height,
        srcImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(srcImage->format),
        dstImage->data, dstImage->width, dstImage->height,
        dstImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(dstImage->format));
}

bool ConvertImageDirect(PixelFormat inFormat, PixelFormat outFormat,
    const void* inData, uint32 inWidth, uint32 inHeight, uint32 inPitch,
    void* outData, uint32 outWidth, uint32 outHeight, uint32 outPitch)
{
    if (inFormat == FORMAT_RGBA5551 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<uint16, uint32, ConvertRGBA5551toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGBA4444 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<uint16, uint32, ConvertRGBA4444toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGB888 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<RGB888, uint32, ConvertRGB888toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGB565 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<uint16, uint32, ConvertRGB565toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_A8 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<uint8, uint32, ConvertA8toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_A16 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<uint16, uint32, ConvertA16toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_BGR888 && outFormat == FORMAT_RGB888)
    {
        ConvertDirect<BGR888, RGB888, ConvertBGR888toRGB888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_BGR888 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<BGR888, uint32, ConvertBGR888toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_BGRA8888 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<BGRA8888, RGBA8888, ConvertBGRA8888toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGBA8888 && outFormat == FORMAT_RGB888)
    {
        ConvertDirect<uint32, RGB888, ConvertRGBA8888toRGB888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGBA16161616 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<RGBA16161616, uint32, ConvertRGBA16161616toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else if (inFormat == FORMAT_RGBA32323232 && outFormat == FORMAT_RGBA8888)
    {
        ConvertDirect<RGBA32323232, uint32, ConvertRGBA32323232toRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        return true;
    }
    else
    {
        Logger::FrameworkDebug("Unsupported image conversion from format %d to %d", inFormat, outFormat);
        return false;
    }
}

bool CanConvertDirect(PixelFormat inFormat, PixelFormat outFormat)
{
    return ConvertImageDirect(inFormat, outFormat, nullptr, 0, 0, 0, nullptr, 0, 0, 0);
}

bool CanConvertFromTo(PixelFormat inFormat, PixelFormat outFormat)
{
    bool inCompressed = LibDdsHelper::IsSupportedCompressedFormat(inFormat);
    bool outCompressed = LibDdsHelper::IsSupportedCompressedFormat(outFormat);

    if (!inCompressed && !outCompressed)
    {
        return CanConvertDirect(inFormat, outFormat);
    }
    else if (inCompressed && outCompressed)
    {
        return true;
    }
    else if (inCompressed)
    {
        return (outFormat == FORMAT_RGBA8888 ? true : CanConvertDirect(FORMAT_RGBA8888, outFormat));
    }
    else
    {
        return (inFormat == FORMAT_RGBA8888 ? true : CanConvertDirect(outFormat, FORMAT_RGBA8888));
    }
}

void SwapRedBlueChannels(const Image* srcImage, const Image* dstImage/* = nullptr*/)
{
    DVASSERT(srcImage);

    auto srcPixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(srcImage->format);
    if (dstImage)
    {
        DVASSERT(PixelFormatDescriptor::GetPixelFormatSizeInBytes(dstImage->format) == srcPixelSize);
        DVASSERT(srcImage->width == dstImage->width);
        DVASSERT(srcImage->height == dstImage->height);
    }

    SwapRedBlueChannels(
        srcImage->format, srcImage->data, srcImage->width, srcImage->height,
        srcImage->width * srcPixelSize,
        dstImage ? dstImage->data : nullptr);
}

void SwapRedBlueChannels(PixelFormat format, void* srcData, uint32 width, uint32 height, uint32 pitch, void* dstData/* = nullptr*/)
{
    if (!dstData)
        dstData = srcData;

    switch (format)
    {
    case FORMAT_RGB888:
    {
        ConvertDirect<BGR888, RGB888, ConvertBGR888toRGB888> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_RGBA8888:
    {
        ConvertDirect<BGRA8888, RGBA8888, ConvertBGRA8888toRGBA8888> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_RGBA4444:
    {
        ConvertDirect<uint16, uint16, ConvertBGRA4444toRGBA4444> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_RGB565:
    {
        ConvertDirect<uint16, uint16, ConvertBGR565toRGB565> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_RGBA16161616:
    {
        ConvertDirect<RGBA16161616, RGBA16161616, ConvertBGRA16161616toRGBA16161616> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_RGBA32323232:
    {
        ConvertDirect<RGBA32323232, RGBA32323232, ConvertBGRA32323232toRGBA32323232> swap;
        swap(srcData, width, height, pitch, dstData);
        return;
    }
    case FORMAT_A8:
    case FORMAT_A16:
    {
        // do nothing for grayscale images
        return;
    }
    default:
    {
        Logger::Error("Image color exchanging is not supported for format %d", format);
        return;
    }
    }
}

void DownscaleTwiceBillinear(PixelFormat inFormat, PixelFormat outFormat,
    const void* inData, uint32 inWidth, uint32 inHeight, uint32 inPitch,
    void* outData, uint32 outWidth, uint32 outHeight, uint32 outPitch, bool normalize)
{
    if ((inFormat == FORMAT_RGBA8888) && (outFormat == FORMAT_RGBA8888))
    {
        if (normalize)
        {
            ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackNormalizedRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
        else
        {
            ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackRGBA8888> convert;
            convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
        }
    }
    else if ((inFormat == FORMAT_RGBA8888) && (outFormat == FORMAT_RGBA4444))
    {
        ConvertDownscaleTwiceBillinear<uint32, uint16, UnpackRGBA8888, PackRGBA4444> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_RGBA4444) && (outFormat == FORMAT_RGBA8888))
    {
        ConvertDownscaleTwiceBillinear<uint16, uint32, UnpackRGBA4444, PackRGBA8888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_A8) && (outFormat == FORMAT_A8))
    {
        ConvertDownscaleTwiceBillinear<uint8, uint8, UnpackA8, PackA8> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_RGB888) && (outFormat == FORMAT_RGB888))
    {
        ConvertDownscaleTwiceBillinear<RGB888, RGB888, UnpackRGB888, PackRGB888> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_RGBA5551) && (outFormat == FORMAT_RGBA5551))
    {
        ConvertDownscaleTwiceBillinear<uint16, uint16, UnpackRGBA5551, PackRGBA5551> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_RGBA16161616) && (outFormat == FORMAT_RGBA16161616))
    {
        ConvertDownscaleTwiceBillinear<RGBA16161616, RGBA16161616, UnpackRGBA16161616, PackRGBA16161616> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else if ((inFormat == FORMAT_RGBA32323232) && (outFormat == FORMAT_RGBA32323232))
    {
        ConvertDownscaleTwiceBillinear<RGBA32323232, RGBA32323232, UnpackRGBA32323232, PackRGBA32323232> convert;
        convert(inData, inWidth, inHeight, inPitch, outData, outWidth, outHeight, outPitch);
    }
    else
    {
        Logger::Debug("Convert function not implemented for %s or %s", PixelFormatDescriptor::GetPixelFormatString(inFormat), PixelFormatDescriptor::GetPixelFormatString(outFormat));
    }
}

Image* DownscaleTwiceBillinear(const Image* source)
{
    if (source->GetPixelFormat() == FORMAT_RGBA8888)
    {
        Image* destination = Image::Create(source->GetWidth() / 2, source->GetHeight() / 2, source->GetPixelFormat());
        if (destination)
        {
            ConvertDownscaleTwiceBillinear<uint32, uint32, UnpackRGBA8888, PackRGBA8888> convertFunc;
            convertFunc(source->GetData(), source->GetWidth(), source->GetHeight(), source->GetWidth() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(source->GetPixelFormat()),
                destination->GetData(), destination->GetWidth(), destination->GetHeight(), destination->GetWidth() * PixelFormatDescriptor::GetPixelFormatSizeInBytes(destination->GetPixelFormat()));
        }
        return destination;
    }
    return 0;
}

void ResizeRGBA8Billinear(const uint32* inPixels, uint32 w, uint32 h, uint32* outPixels, uint32 w2, uint32 h2)
{
    int32 a, b, c, d, x, y, index;
    float32 x_ratio = ((float32)(w - 1)) / w2;
    float32 y_ratio = ((float32)(h - 1)) / h2;
    float32 x_diff, y_diff, blue, red, green, alpha;
    uint32 offset = 0;
    for (uint32 i = 0; i < h2; i++)
    {
        for (uint32 j = 0; j < w2; j++)
        {
            x = (int32)(x_ratio * j);
            y = (int32)(y_ratio * i);
            x_diff = (x_ratio * j) - x;
            y_diff = (y_ratio * i) - y;
            index = (y * w + x);
            a = inPixels[index];
            b = inPixels[index + 1];
            c = inPixels[index + w];
            d = inPixels[index + w + 1];

            blue = (a & 0xff) * (1 - x_diff) * (1 - y_diff) + (b & 0xff) * (x_diff)* (1 - y_diff) +
                (c & 0xff) * (y_diff)* (1 - x_diff) + (d & 0xff) * (x_diff * y_diff);

            green = ((a >> 8) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 8) & 0xff) * (x_diff)* (1 - y_diff) +
                ((c >> 8) & 0xff) * (y_diff)* (1 - x_diff) + ((d >> 8) & 0xff) * (x_diff * y_diff);

            red = ((a >> 16) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 16) & 0xff) * (x_diff)* (1 - y_diff) +
                ((c >> 16) & 0xff) * (y_diff)* (1 - x_diff) + ((d >> 16) & 0xff) * (x_diff * y_diff);

            alpha = ((a >> 24) & 0xff) * (1 - x_diff) * (1 - y_diff) + ((b >> 24) & 0xff) * (x_diff)* (1 - y_diff) +
                ((c >> 24) & 0xff) * (y_diff)* (1 - x_diff) + ((d >> 24) & 0xff) * (x_diff * y_diff);

            outPixels[offset++] =
                ((((uint32)alpha) << 24) & 0xff000000) |
                ((((uint32)red) << 16) & 0xff0000) |
                ((((uint32)green) << 8) & 0xff00) |
                ((uint32)blue);
        }
    }
}

} // namespace ImageConvert
} // namespace DAVA
