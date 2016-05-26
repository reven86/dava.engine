#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/LibJpegHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/LibPngHelper.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibTgaHelper.h"
#include "Render/Image/LibWebPHelper.h"
#include "Render/Image/LibPSDHelper.h"

#include "Render/Image/ImageConvert.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/RenderBase.h"

#include "Base/ScopedPtr.h"

#include "FileSystem/FileSystem.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"


namespace DAVA
{
namespace ImageSystem
{

const Vector<std::unique_ptr<ImageFormatInterface>>& GetWrappers()
{
    static Vector<std::unique_ptr<ImageFormatInterface>> wrappers;

    if (wrappers.empty())
    {
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibPngHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibDdsHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibPVRHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibJpegHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibTgaHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibWebPHelper()));
        wrappers.emplace_back(std::unique_ptr<ImageFormatInterface>(new LibPSDHelper()));
    }
 
#if defined (__DAVAENGINE_DEBUG__)
    DVASSERT(wrappers.size() == IMAGE_FORMAT_COUNT);
    for (size_type w = 0; w < IMAGE_FORMAT_COUNT; ++w)
    {
        DVASSERT(w == wrappers[w]->GetImageFormat());
        DVASSERT(wrappers[w] != nullptr);
    }
#endif //#if defined (__DAVAENGINE_DEBUG__)

    return wrappers;
}

ImageFormatInterface* GetDecoder(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::FORMAT_PVR2:
    case PixelFormat::FORMAT_PVR4:
    case PixelFormat::FORMAT_ETC1:
    case PixelFormat::FORMAT_PVR2_2:
    case PixelFormat::FORMAT_PVR4_2:
    case PixelFormat::FORMAT_EAC_R11_UNSIGNED:
    case PixelFormat::FORMAT_EAC_R11_SIGNED:
    case PixelFormat::FORMAT_EAC_RG11_UNSIGNED:
    case PixelFormat::FORMAT_EAC_RG11_SIGNED:
    case PixelFormat::FORMAT_ETC2_RGB:
    case PixelFormat::FORMAT_ETC2_RGBA:
    case PixelFormat::FORMAT_ETC2_RGB_A1:
        return GetImageFormatInterface(ImageFormat::IMAGE_FORMAT_PVR);

    case PixelFormat::FORMAT_DXT1:
    case PixelFormat::FORMAT_DXT1A:
    case PixelFormat::FORMAT_DXT3:
    case PixelFormat::FORMAT_DXT5:
    case PixelFormat::FORMAT_DXT5NM:
    case PixelFormat::FORMAT_ATC_RGB:
    case PixelFormat::FORMAT_ATC_RGBA_EXPLICIT_ALPHA:
    case PixelFormat::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA:
        return GetImageFormatInterface(ImageFormat::IMAGE_FORMAT_DDS);

    default:
        break;
    }

    return nullptr;
}

Image* LoadSingleMip(const FilePath& pathname, uint32 mip)
{
    Vector<Image*> images;
    ImageSystem::Load(pathname, images, LoadingParams(0, 0, mip, 0));

    Image* image = nullptr;
    if (images.empty() == false)
    {
        image = SafeRetain(images[0]);
        for (Image *img: images)
        {
            SafeRelease(img);
        }
    }

    return image;
}

ImageFormatInterface* GetImageFormatInterface(const ScopedPtr<File>& file)
{
    for (const std::unique_ptr<ImageFormatInterface>& wrapper : GetWrappers())
    {
        if (wrapper->CanProcessFile(file))
        {
            return wrapper.get();
        }
    }
    DVASSERT(false);

    return nullptr;
}

eErrorCode TryToDecompressImages(Vector<Image*>& imageSet)
{
    DVASSERT(false);

    size_type count = imageSet.size();
    for (size_type i = 0; i < count; ++i)
    {
        Image* encodedImage = imageSet[i];

        const PixelFormatDescriptor& pixelDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(encodedImage->format);
        if (!pixelDescriptor.isHardwareSupported)
        {
            Image* decodedImage = nullptr;
            if (PixelFormatDescriptor::IsCompressedFormat(encodedImage->format))
            {
                ImageFormatInterface* properWrapper = GetDecoder(encodedImage->format);
                DVASSERT(properWrapper != nullptr);

                decodedImage = properWrapper->DecodeToRGBA8888(encodedImage);
                if (decodedImage == nullptr)
                {
                    return eErrorCode::ERROR_DECODE_FAIL;
                }
            }
            else
            {
                decodedImage = Image::Create(encodedImage->width, encodedImage->height, PixelFormat::FORMAT_RGBA8888);
                ImageConvert::ConvertImageDirect(encodedImage, encodedImage);
            }

            imageSet[i] = decodedImage;
            encodedImage->Release();
        }
    }

    return eErrorCode::SUCCESS;
}

Image* EnsurePowerOf2Image(Image* image)
{
    if (IsPowerOf2(image->GetWidth()) && IsPowerOf2(image->GetHeight()))
    {
        return SafeRetain(image);
    }
    Image* newImage = Image::Create(NextPowerOf2(image->GetWidth()),
        NextPowerOf2(image->GetHeight()),
        image->GetPixelFormat());
    newImage->InsertImage(image, 0, 0);
    return newImage;
}

void EnsurePowerOf2Images(Vector<Image*>& images)
{
    Vector<Image*>::iterator end = images.end();
    for (Vector<Image*>::iterator iter = images.begin(); iter != end; ++iter)
    {
        Image* image = (*iter);
        if (!IsPowerOf2(image->GetWidth()) || !IsPowerOf2(image->GetHeight()))
        {
            Image* newImage = Image::Create(NextPowerOf2(image->GetWidth()),
                NextPowerOf2(image->GetHeight()),
                image->GetPixelFormat());
            newImage->InsertImage(image, 0, 0);
            (*iter) = newImage;
            SafeRelease(image);
        }
    }
}


eErrorCode LoadWithoutDecompession(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams)
{
    ScopedPtr<File> fileRead(File::Create(pathname, File::READ | File::OPEN));
    if (!fileRead)
    {
        return eErrorCode::ERROR_FILE_NOTFOUND;
    }

    eErrorCode result = LoadWithoutDecompession(fileRead, imageSet, loadingParams);
    return result;
}

eErrorCode LoadWithoutDecompession(const ScopedPtr<File>& file, Vector<Image*>& imageSet, const LoadingParams& loadingParams)
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(file->GetFilename()); //fast by filename
    if (nullptr == properWrapper)
    {
        // Retry by content.
        properWrapper = GetImageFormatInterface(file); //slow by content
    }

    if (nullptr == properWrapper)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->ReadFile(file, imageSet, loadingParams);
}

eErrorCode Load(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams)
{
    eErrorCode loaded = LoadWithoutDecompession(pathname, imageSet, loadingParams);

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    if (loaded == eErrorCode::SUCCESS && imageSet.empty() == false)
    {
//        loaded = TryToDecompressImages(imageSet);
    }

#endif //defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

    return loaded;
}

eErrorCode Load(const ScopedPtr<File>& file, Vector<Image*>& imageSet, const LoadingParams& loadingParams)
{
    eErrorCode loaded = LoadWithoutDecompession(file, imageSet, loadingParams);

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    if (loaded == eErrorCode::SUCCESS && imageSet.empty() == false)
    {
//        loaded = TryToDecompressImages(imageSet);
    }
#endif //defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

    return loaded;
}



eErrorCode SaveAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality)
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if (nullptr == properWrapper)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->WriteFileAsCubeMap(fileName, imageSet, compressionFormat, quality);
}

eErrorCode Save(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality)
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if (nullptr == properWrapper)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->WriteFile(fileName, imageSet, compressionFormat, quality);
}

eErrorCode Save(const FilePath& fileName, Image* image, PixelFormat compressionFormat, ImageQuality quality)
{
    DVASSERT(image != nullptr);

    Vector<Image*> imageSet;
    imageSet.push_back(image);
    return Save(fileName, imageSet, compressionFormat, quality);
}

ImageFormatInterface* GetImageFormatInterface(const FilePath& pathName)
{
    const String extension = pathName.GetExtension();
    for (const std::unique_ptr<ImageFormatInterface>& wrapper : GetWrappers())
    {
        if (wrapper->IsFileExtensionSupported(extension))
        {
            return wrapper.get();
        }
    }

    return nullptr;
}

ImageFormat GetImageFormatForExtension(const FilePath& pathname)
{
    return GetImageFormatForExtension(pathname.GetExtension());
}

ImageFormat GetImageFormatForExtension(const String& extension)
{
    for (const std::unique_ptr<ImageFormatInterface>& wrapper : GetWrappers())
    {
        if (wrapper->IsFileExtensionSupported(extension))
            return wrapper->GetImageFormat();
    }

    return IMAGE_FORMAT_UNKNOWN;
}

ImageFormat GetImageFormatByName(const String& name)
{
    for (const std::unique_ptr<ImageFormatInterface>& wrapper : GetWrappers())
    {
        if (CompareCaseInsensitive(wrapper->Name(), name) == 0)
            return wrapper->GetImageFormat();
    }

    return IMAGE_FORMAT_UNKNOWN;
}


ImageInfo GetImageInfo(const ScopedPtr<File>& infile)
{
    DVASSERT(infile);

    const ImageFormatInterface* properWrapper = GetImageFormatInterface(infile);
    if (nullptr != properWrapper)
    {
        infile->Seek(0, File::SEEK_FROM_START); //reset file state after GetImageFormatInterface
        return properWrapper->GetImageInfo(infile);
    }

    return ImageInfo();
}

ImageInfo GetImageInfo(const FilePath& pathName)
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(pathName); //fast by pathname
    if (nullptr == properWrapper)
    {
        ScopedPtr<File> infile(File::Create(pathName, File::OPEN | File::READ));
        if (infile)
        {
            return GetImageInfo(infile);
        }

        return ImageInfo();
    }

    return properWrapper->GetImageInfo(pathName);
}


uint32 GetBaseMipmap(const LoadingParams& sourceImageParams, const LoadingParams& loadingParams)
{
    if (sourceImageParams.minimalWidth != 0 || sourceImageParams.minimalHeight != 0)
    {
        uint32 width = sourceImageParams.minimalWidth;
        uint32 height = sourceImageParams.minimalHeight;
        uint32 fromMipMap = sourceImageParams.baseMipmap;

        while ((((width >> fromMipMap) < loadingParams.minimalWidth) || ((height >> fromMipMap) < loadingParams.minimalHeight)) && fromMipMap != 0)
        {
            --fromMipMap;
        }

        return fromMipMap;
    }

    return sourceImageParams.baseMipmap;
}

ImageFormatInterface* GetImageFormatInterface(ImageFormat fileFormat)
{
    DVASSERT(fileFormat < IMAGE_FORMAT_COUNT);
    return GetWrappers()[fileFormat].get();
}

const Vector<String>& GetExtensionsFor(ImageFormat format)
{
    return GetImageFormatInterface(format)->Extensions();
}


}
}

