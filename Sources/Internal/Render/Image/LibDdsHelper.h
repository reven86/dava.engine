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


#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/CRCAdditionInterface.h"

namespace DAVA 
{
class Image;
class File;

class LibDdsHelper: public ImageFormatInterface, public CRCAdditionInterface
{
public:
    LibDdsHelper();

    ImageFormat GetImageFormat() const override;
    bool IsMyImage(File* infile) const override;

    eErrorCode ReadFile(File* infile, Vector<Image*>& imageSet, int32 baseMipMap = 0) const override;

    eErrorCode WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;

    ImageInfo GetImageInfo(File* infile) const override;

    bool AddCRCIntoMetaData(const FilePath& filePathname) const override;
    uint32 GetCRCFromFile(const FilePath& filePathname) const override;

    static eErrorCode ReadFile(File *file, Vector<Image*> &imageSet, int32 baseMipMap = 0, bool forceSoftwareConvertation = false);
    static bool DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forceSoftwareConvertation = false);

private:

    static bool WriteDxtFile(const FilePath & fileNameOriginal, const Vector<Vector<Image *>> &imageSets, PixelFormat compressionFormat, bool isCubemap);
    static bool WriteAtcFile(const FilePath & fileNameOriginal, const Vector<Image *> &imageSet, PixelFormat compressionFormat);
    static bool WriteAtcFileAsCubemap(const FilePath & fileNameOriginal, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat);
};

inline ImageFormat LibDdsHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_DDS;
}
};

#endif // __DAVAENGINE_DXT_HELPER_H__