#ifndef __RESOURCEEDITORQT__NOTPASSABLETERRAINPROXY__
#define __RESOURCEEDITORQT__NOTPASSABLETERRAINPROXY__

#include "DAVAEngine.h"

class NotPassableTerrainProxy
{
public:
    NotPassableTerrainProxy(DAVA::int32 heightmapSize);
    virtual ~NotPassableTerrainProxy();

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    DAVA::Texture* GetTexture();
    void UpdateTexture(DAVA::Heightmap* heightmap,
                       const DAVA::AABBox3& landscapeBoundingBox,
                       const DAVA::Rect2i& forRect);

private:
    static const DAVA::int32 NOT_PASSABLE_ANGLE = 23;

    struct TerrainColor
    {
        DAVA::Color color;
        DAVA::Vector2 angleRange;

        TerrainColor(const DAVA::Vector2& angle, const DAVA::Color& color)
        {
            this->color = color;
            this->angleRange = angle;
        }
    };

    bool enabled;
    DAVA::Texture* notPassableTexture;
    DAVA::float32 notPassableAngleTan;
    DAVA::Vector<TerrainColor> angleColor;

    DAVA::Vector<rhi::HVertexBuffer> gridBuffers;

    void LoadColorsArray();
    bool PickColor(DAVA::float32 tan, DAVA::Color& color) const;
};

#endif /* defined(__RESOURCEEDITORQT__NOTPASSABLETERRAINPROXY__) */
