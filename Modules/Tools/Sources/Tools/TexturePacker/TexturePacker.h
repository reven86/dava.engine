#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include "Tools/TextureCompression/TextureConverter.h"
#include "Tools/TexturePacker/Spritesheet.h"
#include "Tools/TexturePacker/DefinitionFile.h"

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>
#include <Math/Math2D.h>

namespace DAVA
{
class DefinitionFile;
class PngImageExt;
class FilePath;

struct SpriteItem
{
    DefinitionFile::Pointer defFile;
    uint32 spriteWeight = 0;
    uint32 frameIndex = 0;
};

class TexturePacker
{
public:
    static const uint32 DEFAULT_TEXTURE_SIZE = 2048;
    static const Set<PixelFormat> PIXEL_FORMATS_WITH_COMPRESSION;
    static const uint32 DEFAULT_MARGIN = 1;

    struct FilterItem
    {
        int8 minFilter;
        int8 magFilter;
        int8 mipFilter;

        FilterItem(int8 minF, int8 magF, int8 mipF)
        {
            minFilter = minF;
            magFilter = magF;
            mipFilter = mipF;
        }
    };

public:
    TexturePacker();

    // pack textures to single texture
    void PackToTextures(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs);
    // page each PSD file to separate texture
    void PackToTexturesSeparate(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs);
    // pack one sprite and use several textures if more than one needed
    void PackToMultipleTextures(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& remainingList, const Vector<eGPUFamily>& forGPUs);

    void SetUseOnlySquareTextures();
    void SetMaxTextureSize(uint32 maxTextureSize);
    void SetConvertQuality(TextureConverter::eConvertQuality quality);
    void SetAlgorithms(const Vector<PackingAlgorithm>& algorithms);
    void SetTexturePostfix(const String& postfix);

    // set visible 1 pixel border for each texture
    void SetTwoSideMargin(bool val = true)
    {
        useTwoSideMargin = val;
    }
    void SetTexturesMargin(uint32 margin)
    {
        texturesMargin = margin;
    }

    const Set<String>& GetErrors() const;

private:
    struct ImageExportKeys
    {
        eGPUFamily forGPU = GPU_ORIGIN;
        ImageFormat imageFormat = IMAGE_FORMAT_UNKNOWN;
        PixelFormat pixelFormat = FORMAT_INVALID;
        ImageQuality imageQuality = DEFAULT_IMAGE_QUALITY;
        bool toComressForGPU = false;
        bool toConvertOrigin = false;
    };

    Vector<ImageExportKeys> GetExportKeys(const Vector<eGPUFamily>& forGPUs);
    void ExportImage(const PngImageExt& image, const Vector<ImageExportKeys>& exportKeys, const FilePath& exportedPathname);

    rhi::TextureAddrMode GetDescriptorWrapMode();
    FilterItem GetDescriptorFilter(bool generateMipMaps = false);

    bool CheckFrameSize(const Size2i& spriteSize, const Size2i& frameSize);

    Vector<SpriteItem> PrepareSpritesVector(const DefinitionFile::Collection& defList);
    Vector<std::unique_ptr<SpritesheetLayout>> PackSprites(Vector<SpriteItem>& spritesToPack, const Vector<ImageExportKeys>& imageExportKeys);
    void SaveResultSheets(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& defList, const Vector<std::unique_ptr<SpritesheetLayout>>& resultSheets, const Vector<ImageExportKeys>& imageExportKeys);

    int32 TryToPack(SpritesheetLayout* sheet, Vector<SpriteItem>& tempSortVector, bool fullPackOnly);

    bool WriteDefinition(const std::unique_ptr<SpritesheetLayout>& sheet, const FilePath& outputPath, const String& textureName, const DefinitionFile& defFile);
    bool WriteMultipleDefinition(const Vector<std::unique_ptr<SpritesheetLayout>>& usedAtlases, const FilePath& outputPath, const char* textureBasename, const DefinitionFile& defFile);
    void WriteDefinitionString(FILE* fp, const Rect2i& writeRect, const Rect2i& originRect, int textureIndex, const String& frameName);

    void DrawToFinalImage(PngImageExt& finalImage, PngImageExt& drawedImage, const SpriteBoundsRect& drawRect, const Rect2i& frameRect);

    String MakeTextureName(const char* basename, uint32 textureIndex) const;

    uint32 maxTextureSize;

    bool onlySquareTextures;
    bool NeedSquareTextureForCompression(const Vector<ImageExportKeys>& keys);

    TextureConverter::eConvertQuality quality;

    bool useTwoSideMargin;
    uint32 texturesMargin;
    Vector<PackingAlgorithm> packAlgorithms;

    String texturePostfix;

    Set<String> errors;
    void AddError(const String& errorMsg);
};
};


#endif // __DAVAENGINE_TEXTURE_PACKER_H__
