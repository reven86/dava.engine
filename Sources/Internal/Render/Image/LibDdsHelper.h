#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/CRCAdditionInterface.h"

namespace DAVA
{
class Image;
class File;

class LibDdsHelper : public ImageFormatInterface, public CRCAdditionInterface
{
public:
    LibDdsHelper();

    ImageFormat GetImageFormat() const override;
    bool CanProcessFile(File* infile) const override;

    eErrorCode ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;

    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(File* infile) const override;

    bool AddCRCIntoMetaData(const FilePath& filePathname) const override;
    uint32 GetCRCFromFile(const FilePath& filePathname) const override;

    static eErrorCode ReadFile(File* file, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams, bool forceSoftwareConvertation = false);
    static bool DecompressImageToRGBA(const DAVA::Image& image, Vector<DAVA::Image*>& imageSet, bool forceSoftwareConvertation = false);

private:
    static bool WriteDxtFile(const FilePath& fileNameOriginal, const Vector<Vector<Image*>>& imageSets, PixelFormat compressionFormat, bool isCubemap);
    static bool WriteAtcFile(const FilePath& fileNameOriginal, const Vector<Image*>& imageSet, PixelFormat compressionFormat);
    static bool WriteAtcFileAsCubemap(const FilePath& fileNameOriginal, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat);
};

inline ImageFormat LibDdsHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_DDS;
}
};

#endif // __DAVAENGINE_DXT_HELPER_H__