#ifndef __TEXTURE_DESCRIPTOR_UTILS_H__
#define __TEXTURE_DESCRIPTOR_UTILS_H__

#include "DAVAEngine.h"

class TextureDescriptorUtils
{
public:
    static bool CreateDescriptorIfNeed(const DAVA::FilePath& pngPathname);
};



#endif // __TEXTURE_DESCRIPTOR_UTILS_H__