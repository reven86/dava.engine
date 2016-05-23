#ifndef __DAVAENGINE_TOOLS_TEXTURE_CONVERTER_H__
#define __DAVAENGINE_TOOLS_TEXTURE_CONVERTER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class TextureDescriptor;
class TextureConverter
{
public:
    enum eConvertQuality : uint32
    {
        ECQ_FASTEST = 0,
        ECQ_FAST,
        ECQ_NORMAL,
        ECQ_HIGH,
        ECQ_VERY_HIGH,

        ECQ_COUNT,
        ECQ_DEFAULT = ECQ_VERY_HIGH
    };

    static FilePath ConvertTexture(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, bool updateAfterConversion, eConvertQuality quality);
    static FilePath GetOutputPath(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);
};
}

#endif /* defined(__DAVAENGINE_TEXTURE_CONVERTER_H__) */
