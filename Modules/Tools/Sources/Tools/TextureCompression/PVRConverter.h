#ifndef __DAVAENGINE_PVR_CONVERTER_H__
#define __DAVAENGINE_PVR_CONVERTER_H__

#include "Tools/TextureCompression/TextureConverter.h"

#include <Base/StaticSingleton.h>
#include <Base/BaseTypes.h>
#include <Render/RenderBase.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class TextureDescriptor;
class PVRConverter : public StaticSingleton<PVRConverter>
{
public:
    PVRConverter();
    virtual ~PVRConverter();

    FilePath ConvertToPvr(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, bool addCRC = true);
    FilePath ConvertNormalMapToPvr(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality);

    void SetPVRTexTool(const FilePath& textToolPathname);

    FilePath GetConvertedTexturePath(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);

protected:
    FilePath PrepareCubeMapForPvrConvert(const TextureDescriptor& descriptor);
    void CleanupCubemapAfterConversion(const TextureDescriptor& descriptor);

    void GetToolCommandLine(const TextureDescriptor& descriptor, const FilePath& fileToConvert, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, Vector<String>& args);

    String GenerateInputName(const TextureDescriptor& descriptor, const FilePath& fileToConvert);

protected:
    Map<PixelFormat, String> pixelFormatToPVRFormat;

    FilePath pvrTexToolPathname;

    Vector<String> pvrToolSuffixes;
    Vector<String> cubemapSuffixes;
};
};


#endif // __DAVAENGINE_PVR_CONVERTER_H__
