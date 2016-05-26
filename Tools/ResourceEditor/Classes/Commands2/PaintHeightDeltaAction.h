#ifndef __PAINT_HEIGHT_DELTA_ACTION_H__
#define __PAINT_HEIGHT_DELTA_ACTION_H__

#include "Commands2/Base/CommandAction.h"
#include "DAVAEngine.h"

class PaintHeightDeltaAction : public CommandAction
{
public:
    PaintHeightDeltaAction(const DAVA::FilePath& targetImagePath,
                           DAVA::float32 refDelta,
                           DAVA::Heightmap* srcHeightmap,
                           DAVA::uint32 targetImageWidth,
                           DAVA::uint32 targetImageHeight,
                           DAVA::float32 targetTerrainHeight,
                           const DAVA::Vector<DAVA::Color>& pixelColors);

    ~PaintHeightDeltaAction();

    virtual void Redo();

protected:
    DAVA::Image* CreateHeightDeltaImage(DAVA::uint32 width, DAVA::uint32 height);
    void CalculateHeightmapToDeltaImageMapping(DAVA::Image* heightmapImage,
                                               DAVA::Image* deltaImage,
                                               /*out*/ DAVA::float32& widthPixelRatio,
                                               /*out*/ DAVA::float32& heightPixelRatio);
    DAVA::float32 SampleHeight(DAVA::uint32 x, DAVA::uint32 y, DAVA::Image* heightmapImage);

    void PrepareDeltaImage(DAVA::Image* heightmapImage,
                           DAVA::Image* deltaImage);

    void MarkDeltaRegion(DAVA::uint32 x,
                         DAVA::uint32 y,
                         DAVA::float32 widthPixelRatio,
                         DAVA::float32 heightPixelRatio,
                         const DAVA::Color& markColor,
                         DAVA::Image* deltaImage);

    void SaveDeltaImage(const DAVA::FilePath& targetPath, DAVA::Image* deltaImage);

private:
    DAVA::FilePath imagePath;
    DAVA::float32 heightDelta;
    DAVA::Heightmap* heightmap;
    DAVA::uint32 imageWidth;
    DAVA::uint32 imageHeight;
    DAVA::float32 terrainHeight;
    DAVA::Vector<DAVA::Color> colors;
};


#endif
