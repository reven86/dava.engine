#ifndef __DAVAENGINE_DXT_CONVERTER_H__
#define __DAVAENGINE_DXT_CONVERTER_H__

#include "Render/RenderBase.h"

namespace DAVA
{
class FilePath;
class TextureDescriptor;

class DXTConverter
{
public:
    static FilePath ConvertToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);
    static FilePath ConvertCubemapToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);
    static FilePath GetDXTOutput(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);
};
};


#endif // __DAVAENGINE_DXT_CONVERTER_H__