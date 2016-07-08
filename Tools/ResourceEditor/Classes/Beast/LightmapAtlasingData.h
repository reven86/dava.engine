#ifndef __LIGHTMAP_ATLASING_DATA__
#define __LIGHTMAP_ATLASING_DATA__

#include "DAVAEngine.h"

struct LightmapAtlasingData
{
    DAVA::String meshInstanceName;
    DAVA::FilePath textureName;
    DAVA::Vector2 uvOffset;
    DAVA::Vector2 uvScale;
};

#endif //__LIGHTMAP_ATLASING_DATA__
