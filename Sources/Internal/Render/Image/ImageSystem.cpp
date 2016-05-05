/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibJpegHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/LibPngHelper.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibTgaHelper.h"
#include "Render/Image/LibWebPHelper.h"
#include "Render/Image/LibPSDHelper.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/RenderBase.h"

#include "Base/ScopedPtr.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

namespace DAVA
{
ImageSystem::ImageSystem()
{
    wrappers[IMAGE_FORMAT_PNG].reset(new LibPngHelper());
    wrappers[IMAGE_FORMAT_DDS].reset(new LibDdsHelper());
    wrappers[IMAGE_FORMAT_PVR].reset(new LibPVRHelper());
    wrappers[IMAGE_FORMAT_JPEG].reset(new LibJpegHelper());
    wrappers[IMAGE_FORMAT_TGA].reset(new LibTgaHelper());
    wrappers[IMAGE_FORMAT_WEBP].reset(new LibWebPHelper());
    wrappers[IMAGE_FORMAT_PSD].reset(new LibPSDHelper());
}

eErrorCode ImageSystem::LoadWithoutDecompession(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams) const
{
    ScopedPtr<File> fileRead(File::Create(pathname, File::READ | File::OPEN));
    if (!fileRead)
    {
        return eErrorCode::ERROR_FILE_NOTFOUND;
    }

    eErrorCode result = LoadWithoutDecompession(fileRead, imageSet, loadingParams);
    return result;
}

eErrorCode ImageSystem::LoadWithoutDecompession(File* file, Vector<Image*>& imageSet, const LoadingParams& loadingParams) const
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

eErrorCode ImageSystem::Load(const FilePath& pathname, Vector<Image*>& imageSet, const LoadingParams& loadingParams) const
{
    eErrorCode loaded = LoadWithoutDecompession(pathname, imageSet, loadingParams);
    
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    if (loaded == eErrorCode::SUCCESS && imageSet.empty() == false)
    {
        loaded = TryToDecompressImages(imageSet);
    }
    
#endif //defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

    return loaded;
}

eErrorCode ImageSystem::Load(File* file, Vector<Image*>& imageSet, const LoadingParams& loadingParams) const
{
    eErrorCode loaded = LoadWithoutDecompession(file, imageSet, loadingParams);
    
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    if (loaded == eErrorCode::SUCCESS && imageSet.empty() == false)
    {
        loaded = TryToDecompressImages(imageSet);
    }
#endif //defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

    return loaded;
}

eErrorCode ImageSystem::TryToDecompressImages(Vector<Image*>& imageSet) const
{
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

Image* ImageSystem::EnsurePowerOf2Image(Image* image) const
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

void ImageSystem::EnsurePowerOf2Images(Vector<Image*>& images) const
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

eErrorCode ImageSystem::SaveAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if (nullptr == properWrapper)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->WriteFileAsCubeMap(fileName, imageSet, compressionFormat, quality);
}

eErrorCode ImageSystem::Save(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if (nullptr == properWrapper)
    {
        return eErrorCode::ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->WriteFile(fileName, imageSet, compressionFormat, quality);
}

eErrorCode ImageSystem::Save(const FilePath& fileName, Image* image, PixelFormat compressionFormat, ImageQuality quality) const
{
    DVASSERT(image != nullptr);

    Vector<Image*> imageSet;
    imageSet.push_back(image);
    return Save(fileName, imageSet, compressionFormat, quality);
}

ImageFormatInterface* ImageSystem::GetImageFormatInterface(const FilePath& pathName) const
{
    const String extension = pathName.GetExtension();
    for (auto& wrapper : wrappers)
    {
        if (wrapper && wrapper->IsFileExtensionSupported(extension))
        {
            return wrapper.get();
        }
    }

    return nullptr;
}

ImageFormatInterface* ImageSystem::GetImageFormatInterface(File* file) const
{
    for (auto& wrapper : wrappers)
    {
        if (wrapper->CanProcessFile(file))
        {
            return wrapper.get();
        }
    }
    DVASSERT(false);

    return nullptr;
}

ImageFormatInterface* ImageSystem::GetDecoder(PixelFormat format) const
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

ImageFormat ImageSystem::GetImageFormatForExtension(const FilePath& pathname) const
{
    return GetImageFormatForExtension(pathname.GetExtension());
}

ImageFormat ImageSystem::GetImageFormatForExtension(const String& extension) const
{
    for (auto& wrapper : wrappers)
    {
        if (wrapper && wrapper->IsFileExtensionSupported(extension))
            return wrapper->GetImageFormat();
    }

    return IMAGE_FORMAT_UNKNOWN;
}

ImageFormat ImageSystem::GetImageFormatByName(const String& name) const
{
    for (auto& wrapper : wrappers)
    {
        if (CompareCaseInsensitive(wrapper->Name(), name) == 0)
            return wrapper->GetImageFormat();
    }

    return IMAGE_FORMAT_UNKNOWN;
}

ImageInfo ImageSystem::GetImageInfo(const FilePath& pathName) const
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

ImageInfo ImageSystem::GetImageInfo(File* infile) const
{
    DVASSERT(infile != nullptr);

    const ImageFormatInterface* properWrapper = GetImageFormatInterface(infile);
    if (nullptr != properWrapper)
    {
        infile->Seek(0, File::SEEK_FROM_START); //reset file state after GetImageFormatInterface
        return properWrapper->GetImageInfo(infile);
    }

    return ImageInfo();
}

uint32 ImageSystem::GetBaseMipmap(const LoadingParams& sourceImageParams, const LoadingParams& loadingParams)
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

ImageFormatInterface* ImageSystem::GetImageFormatInterface(ImageFormat fileFormat) const
{
    DVASSERT(fileFormat < IMAGE_FORMAT_COUNT);
    return wrappers[fileFormat].get();
}

const Vector<String>& ImageSystem::GetExtensionsFor(ImageFormat format) const
{
    return GetImageFormatInterface(format)->Extensions();
}
};
