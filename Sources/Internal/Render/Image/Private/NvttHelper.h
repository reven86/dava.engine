#pragma once

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
