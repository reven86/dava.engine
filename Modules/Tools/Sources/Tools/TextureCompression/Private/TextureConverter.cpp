#include "Tools/TextureCompression/TextureConverter.h"
#include "Tools/TextureCompression/DXTConverter.h"
#include "Tools/TextureCompression/PVRConverter.h"

#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Render/GPUFamilyDescriptor.h>
#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>

ENUM_DECLARE(DAVA::TextureConverter::eConvertQuality)
{
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_FAST, "Developer Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_FASTEST, "Lower Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_NORMAL, "Normal Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_HIGH, "Better Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_VERY_HIGH, "Best Quality");
};

namespace DAVA
{
FilePath TextureConverter::ConvertTexture(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, bool updateAfterConversion, eConvertQuality quality)
{
    const TextureDescriptor::Compression* compression = &descriptor.compression[gpuFamily];

    FilePath outputPath;
    auto compressedFormat = GPUFamilyDescriptor::GetCompressedFileFormat(gpuFamily, static_cast<DAVA::PixelFormat>(compression->format));
    if (compressedFormat == IMAGE_FORMAT_PVR)
    {
        if (IMAGE_FORMAT_WEBP == descriptor.dataSettings.sourceFileFormat)
        {
            Logger::Error("Can not to convert from WebP (descriptor.pathname.GetAbsolutePathname().c_str()) to PVR");
            return FilePath();
        }

        Logger::FrameworkDebug("Starting PVR (%s) conversion (%s)...",
                               GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(compression->format), descriptor.pathname.GetAbsolutePathname().c_str());

        if (descriptor.dataSettings.GetIsNormalMap())
        {
            outputPath = PVRConverter::Instance()->ConvertNormalMapToPvr(descriptor, gpuFamily, quality);
        }
        else
        {
            outputPath = PVRConverter::Instance()->ConvertToPvr(descriptor, gpuFamily, quality);
        }
    }
    else if (compressedFormat == IMAGE_FORMAT_DDS)
    {
        DAVA::Logger::FrameworkDebug("Starting DXT(%s) conversion (%s)...",
                                     GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(compression->format), descriptor.pathname.GetAbsolutePathname().c_str());

        if (descriptor.IsCubeMap())
        {
            outputPath = DXTConverter::ConvertCubemapToDxt(descriptor, gpuFamily);
        }
        else
        {
            outputPath = DXTConverter::ConvertToDxt(descriptor, gpuFamily);
        }
    }
    else
    {
        DVASSERT(false);
    }

    if (updateAfterConversion)
    {
        bool wasUpdated = descriptor.UpdateCrcForFormat(gpuFamily);
        if (wasUpdated)
        {
            // Potential problem may occur in case of multithread convertion of
            // one texture: Save() will dump to drive unvalid compression info
            // and final variant of descriptor must be dumped again after finishing
            // of all threads.
            descriptor.Save();
        }
    }

    return outputPath;
}

FilePath TextureConverter::GetOutputPath(const TextureDescriptor& descriptor, eGPUFamily gpuFamily)
{
    return descriptor.CreateMultiMipPathnameForGPU(gpuFamily);
}
};
