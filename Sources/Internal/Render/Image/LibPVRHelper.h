#pragma once

#include "Base/Platform.h"
#include "Base/BaseTypes.h"

#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/Private/CRCAdditionInterface.h"

namespace DAVA
{
class LibPVRHelper : public ImageFormatInterface, public CRCAdditionInterface
{
public:
    LibPVRHelper();

    // CRCAdditionInterface
    bool AddCRCIntoMetaData(const FilePath& filePathname) const override;
    uint32 GetCRCFromFile(const FilePath& filePathname) const override;

    //ImageFormatInterface
    eErrorCode Load(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const;
    eErrorCode Save(const FilePath& fileName, const Vector<Image*>& imageSet) const;
    eErrorCode SaveCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet) const;

    eErrorCode ReadFile(const ScopedPtr<File>& infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const override;
    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    Image* DecodeToRGBA8888(Image* encodedImage) const override;
    ImageInfo GetImageInfo(const ScopedPtr<File>& infile) const override;

    static bool CanCompressAndDecompress(PixelFormat format);
    static bool DecompressToRGBA(const Image* image, Image* dstImage);
    static bool CompressFromRGBA(const Image* image, Image* dstImage);

protected:
    bool CanProcessFileInternal(const ScopedPtr<File>& infile) const override;

};
}

