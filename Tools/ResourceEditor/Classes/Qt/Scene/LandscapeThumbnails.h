#ifndef __LANDSCAPE_THUMBNAIL_CREATOR__
#define __LANDSCAPE_THUMBNAIL_CREATOR__

#include "Functional/Function.h"

namespace DAVA
{
class Landscape;
class Texture;
}

namespace LandscapeThumbnails
{
using Callback = DAVA::Function<void(DAVA::Landscape*, DAVA::Texture*)>;
void Create(DAVA::Landscape*, Callback callback);
};

#endif // __LANDSCAPE_THUMBNAIL_CREATOR__
