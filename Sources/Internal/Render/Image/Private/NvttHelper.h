#ifndef __DAVAENGINE_NVTT_HELPER_H__
#define __DAVAENGINE_NVTT_HELPER_H__

#include "Render/RenderBase.h"

namespace DAVA
{
namespace NvttHelper
{
bool IsDxtFormat(PixelFormat format);
bool DecompressDxtToRgba(const Image* srcImage, Image* dstImage);
bool CompressRgbaToDxt(const Image* srcImage, Image* dstImage);
}
}

#endif // __DAVAENGINE_NVTT_HELPER_H__
