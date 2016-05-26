#pragma once 

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
/**
    \ingroup render
    \brief Class that represents pixel format for internal using in our SDK. 
 */

class PixelFormatDescriptor
{
public:
    static void SetHardwareSupportedFormats();

    static int32 GetPixelFormatSizeInBytes(PixelFormat format);
    static int32 GetPixelFormatSizeInBits(PixelFormat format);
    static Size2i GetPixelFormatBlockSize(PixelFormat format);

    static const char* GetPixelFormatString(const PixelFormat format);
    static PixelFormat GetPixelFormatByName(const FastName& formatName);

    static const PixelFormatDescriptor& GetPixelFormatDescriptor(const PixelFormat format);

    static bool IsCompressedFormat(PixelFormat format);

private:
    static UnorderedMap<PixelFormat, PixelFormatDescriptor, std::hash<uint8>> pixelDescriptors;

public:
    static rhi::TextureFormat TEXTURE_FORMAT_INVALID;

    PixelFormat formatID;
    FastName name;
    uint8 pixelSize;
    rhi::TextureFormat format;
    bool isHardwareSupported;
    bool isCompressed;
    Size2i blockSize;
};

inline int32 PixelFormatDescriptor::GetPixelFormatSizeInBits(const PixelFormat format)
{
    return GetPixelFormatDescriptor(format).pixelSize;
}

inline int32 PixelFormatDescriptor::GetPixelFormatSizeInBytes(const PixelFormat format)
{
    int32 bits = GetPixelFormatSizeInBits(format);
    if (bits < 8)
    { // To detect wrong situations
        Logger::Warning("[Texture::GetPixelFormatSizeInBytes] format takes less than byte");
    }

    return bits / 8;
}

}
