#ifndef __DAVAENGINE_QUALCOMM_HELPER_H__
#define __DAVAENGINE_QUALCOMM_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/Image/Image.h"

namespace DAVA
{
namespace QualcommHelper
{
bool IsAtcFormat(PixelFormat format);
bool DecompressAtcToRgba(const Image* image, Image* dstImage);
bool CompressRgbaToAtc(const Image* image, Image* dstImage);
}
}

#endif // __DAVAENGINE_QUALCOMM_HELPER_H__