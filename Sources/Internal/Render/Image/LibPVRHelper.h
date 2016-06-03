#pragma once

#include "Base/Platform.h"
#include "Base/BaseTypes.h"

#include "Render/Image/ImageFormatInterface.h"

namespace DAVA
{
class LibPVRHelper : public ImageFormatInterface
{
public:
    LibPVRHelper();

    static bool AddCRCIntoMetaData(const FilePath& filePathname);
    static uint32 GetCRCFromMetaData(const FilePath& filePathname);

    //ImageFormatInterface
    eErrorCode Load(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const;
    eErrorCode Save(const FilePath& fileName, const Vector<Image*>& imageSet) const;
    eErrorCode SaveCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet) const;

    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;

    static bool CanCompressTo(PixelFormat format);
    static bool CompressFromRGBA(const Image* image, Image* dstImage);

    static bool CanDecompressFrom(PixelFormat format);
    static bool DecompressToRGBA(const Image* encodedImage, Image* decodedImage);

protected:
    bool CanProcessFileInternal(const ScopedPtr<File>& infile) const override;
};
}
