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

    static int32 GetPixelFormatSizeInBytes(PixelFormat formatID);
    static int32 GetPixelFormatSizeInBits(PixelFormat formatID);
    static Size2i GetPixelFormatBlockSize(PixelFormat formatID);

    static const char* GetPixelFormatString(const PixelFormat format);
    static PixelFormat GetPixelFormatByName(const FastName& formatName);

    static const PixelFormatDescriptor& GetPixelFormatDescriptor(const PixelFormat formatID);

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
};

inline int32 PixelFormatDescriptor::GetPixelFormatSizeInBits(const PixelFormat formatID)
{
    return GetPixelFormatDescriptor(formatID).pixelSize;
}

inline int32 PixelFormatDescriptor::GetPixelFormatSizeInBytes(const PixelFormat formatID)
{
    int32 bits = GetPixelFormatSizeInBits(formatID);
    if (bits < 8)
    { // To detect wrong situations
        Logger::Warning("[Texture::GetPixelFormatSizeInBytes] format takes less than byte");
    }

    return bits / 8;
}

}
