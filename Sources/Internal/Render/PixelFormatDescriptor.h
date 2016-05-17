#ifndef __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__
#define __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents pixel format for internal using in our SDK. 
 */

#if defined(__DAVAENGINE_OPENGL__)

class PixelFormatDescriptor
{
public:
    static void SetHardwareSupportedFormats();

    static int32 GetPixelFormatSizeInBytes(const PixelFormat formatID);
    static int32 GetPixelFormatSizeInBits(const PixelFormat formatID);

    static const char* GetPixelFormatString(const PixelFormat format);
    static PixelFormat GetPixelFormatByName(const FastName& formatName);

    static const PixelFormatDescriptor& GetPixelFormatDescriptor(const PixelFormat formatID);

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

#endif //#if defined (__DAVAENGINE_OPENGL__)
};
#endif // __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__
