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


#include "Render/ImageLoader.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/RenderBase.h"
#include "Render/LibPngHelpers.h"
#include "Render/LibPVRHelper.h"
#include "Render/LibDxtHelper.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

namespace DAVA 
{

Vector<Image *> ImageLoader::CreateFromFileByExtension(const FilePath &pathname)
{
    if(pathname.IsEqualToExtension("*.pvr"))
    {
        return ImageLoader::CreateFromPVRFile(pathname);
    }
    else if(pathname.IsEqualToExtension("*.dds"))
    {
        return ImageLoader::CreateFromDDSFile(pathname);
    }
    
    return ImageLoader::CreateFromFileByContent(pathname);
}

    
    
Vector<Image *> ImageLoader::CreateFromFileByContent(const FilePath & pathname)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return Vector<Image *>();
    }
    
    Vector<Image *>imageSet = CreateFromFileByContent(file);
    SafeRelease(file);
	return imageSet;
};
    
    
Vector<Image *> ImageLoader::CreateFromFileByContent(File *file)
{
    if(IsPVRFile(file))
    {
        return CreateFromPVR(file);
    }

    if(IsPNGFile(file))
    {
        return CreateFromPNG(file);
    }

    if(IsDDSFile(file))
    {
        return CreateFromDDS(file);
    }
    
    return Vector<Image *>();
}

    
Vector<Image *> ImageLoader::CreateFromPNGFile(const FilePath & pathname)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromPNGFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return Vector<Image *>();
    }
    
    Vector<Image *>imageSet = CreateFromPNG(file);
    SafeRelease(file);
	return imageSet;
}
    
Vector<Image *> ImageLoader::CreateFromPVRFile(const FilePath & pathname)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromPVRFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return Vector<Image *>();
    }
    
    Vector<Image *>imageSet = CreateFromPVR(file);
    SafeRelease(file);
	return imageSet;
}
    
Vector<Image *> ImageLoader::CreateFromDDSFile(const FilePath & pathname)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromDDSFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return Vector<Image *>();
    }
    
    Vector<Image *>imageSet = CreateFromDDS(file);
    SafeRelease(file);
	return imageSet;
}


bool ImageLoader::IsPNGFile(DAVA::File *file)
{
    bool isPng = LibPngWrapper::IsPngFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isPng;
}
    
bool ImageLoader::IsPVRFile(DAVA::File *file)
{
    bool isPvr = LibPVRHelper::IsPvrFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isPvr;
}

bool ImageLoader::IsDDSFile(DAVA::File *file)
{
    bool isDXT = LibDxtHelper::IsDxtFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isDXT;
}
    
    
Vector<Image *> ImageLoader::CreateFromPNG(DAVA::File *file)
{
    Image *pngImage = new Image();
    if(pngImage)
    {
        int32 retCode = LibPngWrapper::ReadPngFile(file, pngImage);
        if(1 == retCode)
        {
            Vector<Image *>imageSet;
            imageSet.push_back(pngImage);
            return imageSet;
        }
        
        SafeRelease(pngImage);
    }
    
    return Vector<Image *>();
}

Vector<Image *> ImageLoader::CreateFromDDS(DAVA::File *file)
{
    Vector<Image *> retObj;

	bool res = LibDxtHelper::ReadDxtFile(file, retObj);
	if(false == res)
	{
		for_each(retObj.begin(), retObj.end(),SafeRelease<Image>);
		retObj.clear();
	}
	return retObj;
}

Vector<Image *> ImageLoader::CreateFromPVR(DAVA::File *file)
{
//    uint64 loadTime = SystemTimer::Instance()->AbsoluteMS();

    int32 mipMapLevelsCount = LibPVRHelper::GetMipMapLevelsCount(file);
	int32 faceCount = LibPVRHelper::GetCubemapFaceCount(file);
	int32 totalImageCount = mipMapLevelsCount * faceCount;
    if(totalImageCount)
    {
        Vector<Image *> imageSet;
        imageSet.reserve(totalImageCount);
        for(int32 i = 0; i < totalImageCount; ++i)
        {
            Image *image = new Image();
            if(!image)
            {
                Logger::Error("[ImageLoader::CreateFromPVR] Cannot allocate memory");
				for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
                return Vector<Image *>();
            }
            
            imageSet.push_back(image);
        }

        file->Seek(0, File::SEEK_FROM_START);
        bool read = LibPVRHelper::ReadFile(file, imageSet);
        if(!read)
        {
            Logger::Error("[ImageLoader::CreateFromPVR] Cannot read images from PVR file (%s)", file->GetFilename().GetAbsolutePathname().c_str());
			for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
            return Vector<Image *>();
        }
//        loadTime = SystemTimer::Instance()->AbsoluteMS() - loadTime;
//        Logger::Info("Unpack PVR(%s) for %ldms", file->GetFilename().c_str(), loadTime);
        return imageSet;
    }
    return Vector<Image *>();
}

void ImageLoader::Save(DAVA::Image *image, const FilePath &pathname)
{
    DVASSERT(pathname.IsEqualToExtension(".png"));
    
    DVASSERT((FORMAT_RGBA8888 == image->format) || (FORMAT_A8 == image->format) || (FORMAT_A16 == image->format));
    LibPngWrapper::WritePngFile(pathname, image->width, image->height, image->data, image->format);
}
    
    
};
