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


#include "ImageSystem.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/RenderBase.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

#include "Render/Image/LibJpegHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/LibPngHelper.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibTgaHelper.h"
#include "Render/Image/LibWebPHelper.h"
#include "Render/Image/LibPSDHelper.h"

#include "Base/ScopedPtr.h"

namespace DAVA
{
namespace ImageSystem
{
static UnorderedMap<ImageFormat, std::shared_ptr<ImageFormatInterface>, std::hash<uint8>> wrappers = {
    { IMAGE_FORMAT_PNG, std::shared_ptr<ImageFormatInterface>(new LibPngHelper) },
    { IMAGE_FORMAT_DDS, std::shared_ptr<ImageFormatInterface>(new LibDdsHelper) },
    { IMAGE_FORMAT_PVR, std::shared_ptr<ImageFormatInterface>(new LibPVRHelper) },
    { IMAGE_FORMAT_JPEG, std::shared_ptr<ImageFormatInterface>(new LibJpegHelper) },
    { IMAGE_FORMAT_TGA, std::shared_ptr<ImageFormatInterface>(new LibTgaHelper) },
    { IMAGE_FORMAT_WEBP, std::shared_ptr<ImageFormatInterface>(new LibWebPHelper) }
};

eErrorCode Load(const FilePath& pathname, Vector<Image*>& imageSet, uint32 baseMipmap)
{
    ScopedPtr<File> fileRead(File::Create(pathname, File::READ | File::OPEN));
    if (!fileRead)
    {
        return eErrorCode::ERROR_FILE_NOTFOUND;
    }

    eErrorCode result = Load(fileRead, imageSet, baseMipmap);
    return result;
}

eErrorCode Load(const ScopedPtr<File>& file, Vector<Image*>& imageSet, uint32 baseMipmap)
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

    return properWrapper->ReadFile(file, imageSet, baseMipmap);
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
    for (const auto& wrapper : wrappers)
    {
        if (wrapper.second->IsFileExtensionSupported(extension))
        {
            return wrapper.second.get();
        }
    }

    return nullptr;
}

ImageFormatInterface* GetImageFormatInterface(const ScopedPtr<File>& file)
{
    for (const auto& wrapper : wrappers)
    {
        if (wrapper.second->CanProcessFile(file))
        {
            return wrapper.second.get();
        }
    }
    DVASSERT_MSG(false, "Can't get image format wrapper for passed file");

    return nullptr;
}

ImageFormat GetImageFormatForExtension(const FilePath& pathname)
{
    return GetImageFormatForExtension(pathname.GetExtension());
}

ImageFormat GetImageFormatForExtension(const String& extension)
{
    for (const auto& wrapper : wrappers)
    {
        if (wrapper.second->IsFileExtensionSupported(extension))
            return wrapper.first;
    }

    return IMAGE_FORMAT_UNKNOWN;
}

ImageFormat GetImageFormatByName(const String& name)
{
    for (const auto& wrapper : wrappers)
    {
        if (CompareCaseInsensitive(wrapper.second->GetFormatName(), name) == 0)
            return wrapper.first;
    }

    return IMAGE_FORMAT_UNKNOWN;
}

ImageInfo GetImageInfo(const FilePath& pathName)
{
    ScopedPtr<File> infile(File::Create(pathName, File::OPEN | File::READ));
    if (infile)
    {
        ImageFormatInterface* properWrapper = GetImageFormatInterface(pathName);
        if (!properWrapper)
        {
            properWrapper = GetImageFormatInterface(infile);
        }

        if (properWrapper)
        {
            return properWrapper->GetImageInfo(infile);
        }
    }
    return ImageInfo();
}

ImageFormatInterface* GetImageFormatInterface(ImageFormat fileFormat)
{
    DVASSERT(fileFormat < IMAGE_FORMAT_COUNT);
    return wrappers[fileFormat].get();
}

const Vector<String>& GetExtensionsFor(ImageFormat format)
{
    return GetImageFormatInterface(format)->GetExtensions();
}

} // namespace ImageSystem
} // namespace DAVA
