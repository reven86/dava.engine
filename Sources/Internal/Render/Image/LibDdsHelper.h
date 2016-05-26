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
class File;
class Image;

class LibDdsHelper : public ImageFormatInterface, public CRCAdditionInterface
{
public:
    LibDdsHelper();

    // ImageFormatInterface
    bool CanProcessFile(const ScopedPtr<File>& infile) const override;
    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;

    // CRCAdditionInterface
    bool AddCRCIntoMetaData(const FilePath& filePathname) const override;
    uint32 GetCRCFromFile(const FilePath& filePathname) const override;

    static bool CanCompressAndDecompress(PixelFormat format);
    static bool DecompressToRGBA(const Image* image, Image* dstImage);
    static bool CompressFromRGBA(const Image* image, Image* dstImage);

private:
    eErrorCode WriteFileInternal(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const;
};
};

#endif // __DAVAENGINE_DXT_HELPER_H__