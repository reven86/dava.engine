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


#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/RenderBase.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibJpegHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/LibPngHelpers.h"
#include "Render/Image/LibPVRHelper.h"
#include "Render/Image/LibTgaHelper.h"

namespace DAVA 
{

ImageSystem::ImageSystem()
{
    wrappers[IMAGE_FORMAT_PNG] = new LibPngWrapper();
    wrappers[IMAGE_FORMAT_DDS] = new LibDdsHelper();
    wrappers[IMAGE_FORMAT_PVR] = new LibPVRHelper();
    wrappers[IMAGE_FORMAT_JPEG] = new LibJpegWrapper();
    wrappers[IMAGE_FORMAT_TGA] = new LibTgaWrapper();
}
    
ImageSystem::~ImageSystem()
{
    for(auto wrapper : wrappers)
    {
        delete wrapper;
    }
}

eErrorCode ImageSystem::Load(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap) const
{
    File *fileRead = File::Create(pathname, File::READ | File::OPEN);
    if(!fileRead)
    {
        return ERROR_FILE_NOTFOUND;
    }
    
    eErrorCode result = Load(fileRead, imageSet, baseMipmap);
    
    SafeRelease(fileRead);
    
    return result;
}

eErrorCode ImageSystem::Load(File *file, Vector<Image *> & imageSet, int32 baseMipmap) const
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(file->GetFilename());
    if (!properWrapper)
    {
        // Retry by content.
        properWrapper = GetImageFormatInterface(file);
    }
    
    if (NULL == properWrapper || !properWrapper->IsImage(file))
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }

    return properWrapper->ReadFile(file, imageSet, baseMipmap);
}
    
Image* ImageSystem::EnsurePowerOf2Image(Image* image) const
{
    if (IsPowerOf2(image->GetWidth() && IsPowerOf2(image->GetHeight())))
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

eErrorCode ImageSystem::SaveAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if(!properWrapper)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }
    
    return properWrapper->WriteFileAsCubeMap(fileName, imageSet, compressionFormat);
}
    
eErrorCode ImageSystem::Save(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const
{
    ImageFormatInterface* properWrapper = GetImageFormatInterface(fileName);
    if(!properWrapper)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }
    
    return properWrapper->WriteFile(fileName, imageSet, compressionFormat);
}

eErrorCode ImageSystem::Save(const FilePath & fileName, Image *image, PixelFormat compressionFormat) const
{
    if (NULL == image)
    {
        return ERROR_WRITE_FAIL;
    }
    Vector<Image*> imageSet;
    imageSet.push_back(image);
    return Save(fileName, imageSet, compressionFormat);
}
    
ImageFormatInterface* ImageSystem::GetImageFormatInterface(const FilePath & pathname) const
{
    String extension = pathname.GetExtension();
    for(int32 i = 0; i < IMAGE_FORMAT_COUNT ; ++i)
    {
        if(wrappers[i]->IsFileExtensionSupported(extension))
        {
            return wrappers[i];
        }
    }
    
    return NULL;
}

ImageFormatInterface* ImageSystem::GetImageFormatInterface(File *file) const
{
    for(int32 i = 0; i < IMAGE_FORMAT_COUNT; ++i)
    {
        if( wrappers[i]->IsImage(file))
        {
            return  wrappers[i];
        }
    }
    DVASSERT(0);
    
    return NULL;
}
    
    
ImageFormat ImageSystem::GetImageFormatForExtension(const String &extension) const
{
    for(auto i = 0; i < IMAGE_FORMAT_COUNT; ++i)
    {
        auto supported = wrappers[i]->IsFileExtensionSupported(extension);
        if(supported)
        {
            return static_cast<ImageFormat>(i);
        }
    }

    return IMAGE_FORMAT_UNKNOWN;
}
    
    
};
