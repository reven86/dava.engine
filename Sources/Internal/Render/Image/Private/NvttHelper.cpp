#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/Private/NvttHelper.h"
#include "Base/GlobalEnum.h"

#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

namespace DAVA
{
namespace NvttHelper
{
namespace Internal
{
nvtt::Format GetNVTTFormatByPixelFormat(PixelFormat pixelFormat)
{
    switch (pixelFormat)
    {
    case FORMAT_DXT1:
        return nvtt::Format_DXT1;
    case FORMAT_DXT1A:
        return nvtt::Format_DXT1a;
    case FORMAT_DXT3:
        return nvtt::Format_DXT3;
    case FORMAT_DXT5:
        return nvtt::Format_DXT5;
    case FORMAT_DXT5NM:
        return nvtt::Format_DXT5n;
    case FORMAT_ATC_RGB:
        return nvtt::Format_ATC_RGB;
    case FORMAT_ATC_RGBA_EXPLICIT_ALPHA:
        return nvtt::Format_ATC_RGBA_EXPLICIT_ALPHA;
    case FORMAT_ATC_RGBA_INTERPOLATED_ALPHA:
        return nvtt::Format_ATC_RGBA_INTERPOLATED_ALPHA;
    case FORMAT_RGBA8888:
        return nvtt::Format_RGBA;
    default:
        DVASSERT(false, "Unsupported pixel format");
        return nvtt::Format_COUNT;
    }
}

struct BufferOutputHandler : public nvtt::OutputHandler
{
    explicit BufferOutputHandler(Vector<uint8>& buf)
        : buffer(buf)
    {
        buffer.clear();
    }

    void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
    {
        Logger::FrameworkDebug("Compressing image: size %d [%dx%d] depth %d, face %d, mip %d", size, width, height, depth, face, miplevel);
        writingImage = true;
    }

    bool writeData(const void* data, int size) override
    {
        if (writingImage)
        {
            auto prevSize = buffer.size();
            buffer.resize(prevSize + size);
            Memcpy(&buffer[prevSize], data, size);
        }
        return true;
    }

    bool writingImage = false;
    Vector<uint8>& buffer;
};

struct ImageOutputHandler : public nvtt::OutputHandler
{
    explicit ImageOutputHandler(Image* img)
        : image(img)
        , ptr(image->data)
        , bytesWritten(0)
    {
    }

    void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
    {
        writingImage = true;
        Logger::FrameworkDebug("Compressing image: size %d [%dx%d] depth %d, face %d, mip %d", size, width, height, depth, face, miplevel);
    }

    bool writeData(const void* data, int size) override
    {
        if (writingImage)
        {
            if (bytesWritten + size > image->dataSize)
            {
                Logger::Error("Compressed data size is larger than expected");
                return false;
            }
            else
            {
                Memcpy(ptr, data, size);
                ptr += size;
                bytesWritten += size;
                return true;
            }
        }
        return true;
    }

    bool writingImage = false;
    Image* image = nullptr;
    uint8* ptr = nullptr;
    uint32 bytesWritten = 0;
};

} // namespace Internal

bool IsDxtFormat(PixelFormat format)
{
    return (format >= FORMAT_DXT1 && format <= FORMAT_DXT5NM);
}

bool DecompressDxtToRgba(const Image* srcImage, Image* dstImage)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

    DVASSERT(false, "No need to decompress on mobile platforms");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else
    DVASSERT(srcImage);
    DVASSERT(dstImage);
    DVASSERT(IsDxtFormat(srcImage->format));
    DVASSERT(dstImage->format == FORMAT_RGBA8888);

    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, srcImage->width, srcImage->height);
    inputOptions.setMipmapGeneration(false);

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(Internal::GetNVTTFormatByPixelFormat(srcImage->format));
    if (FORMAT_DXT5NM == srcImage->format)
    {
        inputOptions.setNormalMap(true);
    }

    uint32 headerSize = DECOMPRESSOR_MIN_HEADER_SIZE;
    Vector<uint8> imageBuffer(headerSize + srcImage->dataSize);

    uint32 realHeaderSize = nvtt::Decompressor::getHeader(imageBuffer.data(), headerSize, inputOptions, compressionOptions);
    if (realHeaderSize > DECOMPRESSOR_MIN_HEADER_SIZE)
    {
        Logger::Error("[NvttHelper::DecompressDxt] Header size (%d) is bigger than maximum expected", realHeaderSize);
        return false;
    }

    nvtt::Decompressor dec;

    Memcpy(imageBuffer.data() + realHeaderSize, srcImage->data, srcImage->dataSize);

    if (!dec.initWithDDSFile(imageBuffer.data(), realHeaderSize + srcImage->dataSize))
    {
        Logger::Error("[InitDecompressor] Wrong buffer data");
        return false;
    }

    const PixelFormat outFormat = FORMAT_RGBA8888;
    ScopedPtr<Image> newImage(Image::Create(srcImage->width, srcImage->height, outFormat));
    const uint32 mip = 0;
    bool decompressedOk = dec.process(newImage->data, newImage->dataSize, mip);
    if (decompressedOk)
    {
        // nvtt decompresses into BGRA8888, thus we need to swap channels to obtain RGBA8888
        ImageConvert::SwapRedBlueChannels(newImage, dstImage);

        newImage->mipmapLevel = srcImage->mipmapLevel;
        newImage->cubeFaceID = srcImage->cubeFaceID;

        return true;
    }

    return false;
#endif
}

bool CompressRgbaToDxt(const Image* srcImage, Image* dstImage)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

    DVASSERT(false, "No need to compress on mobile platforms");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else
    DVASSERT(srcImage);
    DVASSERT(dstImage);
    DVASSERT(srcImage->format == FORMAT_RGBA8888, "Source image format is not rgba8888");
    DVASSERT(IsDxtFormat(dstImage->format), "Specified format is not a dxt compressed");

    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, srcImage->width, srcImage->height);

    ScopedPtr<Image> bgraImage(Image::Create(srcImage->width, srcImage->height, FORMAT_RGBA8888));
    ImageConvert::SwapRedBlueChannels(srcImage, bgraImage);
    inputOptions.setMipmapData(bgraImage->data, bgraImage->width, bgraImage->height, 1, 0, 0);
    inputOptions.setMipmapGeneration(false);

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(Internal::GetNVTTFormatByPixelFormat(dstImage->format));
    if (FORMAT_DXT5NM == dstImage->format)
    {
        inputOptions.setNormalMap(true);
    }

    nvtt::OutputOptions outputOptions;
    Internal::ImageOutputHandler outputHandler(dstImage);
    outputOptions.setOutputHandler(&outputHandler);

    nvtt::Compressor compressor;
    bool compressedOk = compressor.process(inputOptions, compressionOptions, outputOptions);
    if (compressedOk)
    {
        if (outputHandler.bytesWritten == dstImage->dataSize)
        {
            return true;
        }
        else
        {
            Logger::Error("[NvttHelper::CompressRGBAtoDxt] dxt compress size %d is not equal to expected %d", outputHandler.bytesWritten, dstImage->dataSize);
            return false;
        }
    }
    else
    {
        Logger::Error("[NvttHelper::CompressRGBAtoDxt] dxt compress error");
        return false;
    }
#endif
}
}
}
