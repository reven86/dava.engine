#ifndef __LIGHTMAPS_PACKER_H__
#define __LIGHTMAPS_PACKER_H__

#include "DAVAEngine.h"

#include "LightmapAtlasingData.h"
#include "SpriteResourcesPacker.h"

class LightmapsPacker : public SpriteResourcesPacker
{
public:
    void CreateDescriptors();
    void ParseSpriteDescriptors();
    DAVA::Vector<LightmapAtlasingData>* GetAtlasingData();

private:
    DAVA::Vector2 GetTextureSize(const DAVA::FilePath& filePath);

private:
    DAVA::Vector<LightmapAtlasingData> atlasingData;
};

#endif //__LIGHTMAPS_PACKER_H__
