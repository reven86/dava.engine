#ifndef __LIGHTMAPS_PACKER_H__
#define __LIGHTMAPS_PACKER_H__

#include "DAVAEngine.h"

#include "LightmapAtlasingData.h"
#include "SpriteResourcesPacker.h"

class LightmapsPacker : public SpriteResourcesPacker
{
public:
    LightmapsPacker();

    void CreateDescriptors();
    void ParseSpriteDescriptors();
    DAVA::Vector<LightmapAtlasingData>* GetAtlasingData();

private:
    DAVA::Vector<LightmapAtlasingData> atlasingData;

    DAVA::Vector2 GetTextureSize(const DAVA::FilePath& filePath);
};

#endif //__LIGHTMAPS_PACKER_H__
