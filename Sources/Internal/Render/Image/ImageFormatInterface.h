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


#ifndef __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__
#define __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/File.h"
#include "Utils/Utils.h"
#include "Render/PixelFormatDescriptor.h"

namespace DAVA 
{
class Image;

struct ImageInfo
{
    bool isEmpty()
    {
        return (0 == width || 0 == height);
    }

    Size2i GetImageSize() const
    {
        return Size2i(width, height);
    }

    bool operator==(const ImageInfo& another)
    {
        return (
            width == another.width && 
            height == another.height && 
            format == another.format);
    }

    uint32 width = 0;
    uint32 height = 0;
    PixelFormat format = FORMAT_INVALID;
    uint32 dataSize = 0;
    uint32 mipmapsCount = 0;
};


class ImageFormatInterface
{
public:
    virtual ~ImageFormatInterface() = default;

    virtual ImageFormat GetImageFormat() const = 0;
    virtual bool CanProcessFile(File* file) const = 0;

    virtual eErrorCode ReadFile(File *infile, Vector<Image *> &imageSet, int32 fromMipmap) const = 0;

    virtual eErrorCode WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;
    virtual eErrorCode WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat, ImageQuality quality) const = 0;

    virtual ImageInfo GetImageInfo(File *infile) const = 0;
    inline ImageInfo GetImageInfo(const FilePath &path) const;

    inline bool IsFormatSupported(PixelFormat format) const;
    inline bool IsFileExtensionSupported(const String& extension) const;

    inline const Vector<String>& Extensions() const;
    inline const char* Name() const;
    
protected:
    Vector<PixelFormat> supportedFormats;
    Vector<String> supportedExtensions;
    String name;
};

ImageInfo ImageFormatInterface::GetImageInfo(const FilePath &path) const
{
    File *infile = File::Create(path, File::OPEN | File::READ);
    if (nullptr == infile)
    {
        return ImageInfo();
    }
    ImageInfo info = GetImageInfo(infile);
    infile->Release();
    return info;
}

inline bool ImageFormatInterface::IsFormatSupported(PixelFormat format) const
{
    return (std::find(supportedFormats.begin(), supportedFormats.end(), format) != supportedFormats.end());
}

inline bool ImageFormatInterface::IsFileExtensionSupported(const String& extension) const
{
    for (Vector<String>::const_iterator it = supportedExtensions.begin(); it != supportedExtensions.end(); ++it)
    {
        const bool isEqual = (CompareCaseInsensitive(*it, extension) == 0);
        if (isEqual)
        {
            return true;
        }
    }

    return false;
}

inline const Vector<String>& ImageFormatInterface::Extensions() const
{
    return supportedExtensions;
}

inline const char* ImageFormatInterface::Name() const
{
    return name.c_str();
}

};

#endif // __DAVAENGINE_IMAGE_FORMAT_INTERFACE_H__