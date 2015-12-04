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

#include "Render/Image/DDS/QualcommHelper.h"
#include "Render/Image/DDS/NvttHelper.h"
#include "Render/Image/Image.h"
#include "Render/PixelFormatDescriptor.h"

#include <libdxt/nvtt_extra.h>

#if defined(__DAVAENGINE_WIN_UAP__)

//disabling of warning 
#pragma warning(push)
#pragma warning(disable : 4091)
#include <libatc/TextureConverter.h>
#pragma warning(pop)

#else
#include <libatc/TextureConverter.h>
#endif

namespace DAVA
{
namespace QualcommHelper
{
const Map<int32, PixelFormat> formatsMap =
{
  { Q_FORMAT_ATC_RGB, FORMAT_ATC_RGB },
  { Q_FORMAT_ATC_RGBA_EXPLICIT_ALPHA, FORMAT_ATC_RGBA_EXPLICIT_ALPHA },
  { Q_FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, FORMAT_ATC_RGBA_INTERPOLATED_ALPHA },
  { Q_FORMAT_RGBA_8UI, FORMAT_RGBA8888 },
  { Q_FORMAT_RGB_8UI, FORMAT_RGB888 },
  { Q_FORMAT_RGB5_A1UI, FORMAT_RGBA5551 },
  { Q_FORMAT_RGBA_4444, FORMAT_RGBA4444 },
  { Q_FORMAT_RGB_565, FORMAT_RGB565 },
  { Q_FORMAT_ALPHA_8, FORMAT_A8 }
};

int32 GetQualcommFromDava(PixelFormat format)
{
    for (const auto& pair : formatsMap)
    {
        if (pair.second == format)
        {
            return pair.first;
        }
    }

    Logger::Error("Wrong pixel format (%d).", GlobalEnumMap<PixelFormat>::Instance()->ToString(format));
    return -1;
}

PixelFormat GetDavaFromQualcomm(int32 format)
{
    const auto& pairFound = formatsMap.find(format);
    if (pairFound == formatsMap.end())
    {
        Logger::Error("Wrong qualcomm format (%d).", format);
        return FORMAT_INVALID;
    }
    else
    {
        return pairFound->second;
    }
}

ImagePtr DecompressAtcToRGBA(const Image* image)
{
#if defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
#if defined(__DAVAENGINE_MACOS__)
    if (image->format == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("Decompressing FORMAT_ATC_RGBA_INTERPOLATED_ALPHA disabled on OSX platform, because of bug in qualcomm library");
        return ImagePtr(nullptr);
    }
#endif

    DVASSERT(image);

    const int32 qualcommFormat = GetQualcommFromDava(image->format);

    TQonvertImage srcImg = { 0 };
    TQonvertImage dstImg = { 0 };

    srcImg.nWidth = image->width;
    srcImg.nHeight = image->height;
    srcImg.nFormat = qualcommFormat;
    srcImg.nDataSize = image->dataSize;
    srcImg.pData = image->data;

    dstImg.nWidth = image->width;
    dstImg.nHeight = image->height;
    dstImg.nFormat = Q_FORMAT_RGBA_8UI;
    dstImg.nDataSize = 0;
    dstImg.pData = nullptr;

    if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS ||
        dstImg.nDataSize == 0)
    {
        Logger::Error("[DecompressAtcToRGBA] Failed to get dst data size");
        return ImagePtr(nullptr);
    }

    Vector<unsigned char> dstData(dstImg.nDataSize);
    dstImg.pData = dstData.data();
    if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS)
    {
        Logger::Error("[DecompressAtc] Failed to convert data");
        return ImagePtr(nullptr);
    }

    ImagePtr convertedImage(Image::CreateFromData(image->width, image->height, FORMAT_RGBA8888, dstImg.pData));
    convertedImage->mipmapLevel = image->mipmapLevel;
    convertedImage->cubeFaceID = image->cubeFaceID;

    return convertedImage;
#else
    return ImagePtr(nullptr);
#endif //defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
}

bool WriteAtcFile(const FilePath& outFileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    DVASSERT_MSG(false, "Qualcomm doesn't provide texture converter library for ios/android");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else

    if (compressionFormat != FORMAT_ATC_RGB &&
        compressionFormat != FORMAT_ATC_RGBA_EXPLICIT_ALPHA &&
        compressionFormat != FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Wrong copression format (%d).", compressionFormat);
        return false;
    }

    if (imageSet.empty())
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Empty income image vector.");
        return false;
    }

    uint32 compressedDataSize = 0;
    uint32 mipCount = imageSet.size();
    Vector<TQonvertImage> srcImages(mipCount);
    Vector<TQonvertImage> dstImages(mipCount);

    auto pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(imageSet[0]->format);
    auto atcSrcFormat = GetQualcommFromDava(imageSet[0]->format);
    auto atcDstFormat = GetQualcommFromDava(compressionFormat);

    for (uint32 i = 0; i < mipCount; ++i)
    {
        TQonvertImage& srcImg = srcImages[i];
        TQonvertImage& dstImg = dstImages[i];

        srcImg = { 0 };
        srcImg.nWidth = imageSet[i]->width;
        srcImg.nHeight = imageSet[i]->height;
        srcImg.nFormat = atcSrcFormat;
        srcImg.nDataSize = imageSet[i]->width * imageSet[i]->height * pixelSize;
        srcImg.pData = imageSet[i]->data;

        dstImg = { 0 };
        dstImg.nWidth = imageSet[i]->width;
        dstImg.nHeight = imageSet[i]->height;
        dstImg.nFormat = atcDstFormat;
        dstImg.nDataSize = 0;
        dstImg.pData = nullptr;

        if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
        {
            Logger::Error("[QualcommHelper::WriteAtcFile] Error converting (%s).", outFileName.GetAbsolutePathname().c_str());
            return false;
        }
        compressedDataSize += dstImg.nDataSize;
    }

    Vector<unsigned char> compressedData(compressedDataSize);
    unsigned char* imageData = compressedData.data();
    for (uint32 i = 0; i < mipCount; ++i)
    {
        TQonvertImage& srcImg = srcImages[i];
        TQonvertImage& dstImg = dstImages[i];

        dstImg.pData = imageData;
        imageData += dstImg.nDataSize;

        if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
        {
            Logger::Error("[QualcommHelper::WriteAtcFile] Error converting (%s).", outFileName.GetAbsolutePathname().c_str());
            return false;
        }
    }

    return NvttHelper::WriteDdsFile(outFileName, compressionFormat, compressedData, imageSet[0]->width, imageSet[0]->height, mipCount, false);
#endif
}

bool WriteAtcFileAsCubemap(const DAVA::FilePath& outFileName, const Vector<Vector<DAVA::Image*>>& imageSets, DAVA::PixelFormat compressionFormat)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    DVASSERT_MSG(false, "Qualcomm doesn't provide texture converter library for ios/android");
    return false;

#elif defined(__DAVAENGINE_WIN_UAP__)
    __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    return false;

#else

    DVASSERT(imageSets.empty() == false);
    DVASSERT(imageSets[0].empty() == false);

    if (compressionFormat != FORMAT_ATC_RGB &&
        compressionFormat != FORMAT_ATC_RGBA_EXPLICIT_ALPHA &&
        compressionFormat != FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Wrong copression format (%d).", compressionFormat);
        return false;
    }
    if (imageSets.empty())
    {
        Logger::Error("[LibDdsHelper::WriteAtcFile] Empty income image vector.");
        return false;
    }

    uint32 facesCount = imageSets.size();
    uint32 mipCount = imageSets[0].size();

    uint32 compressedDataSize = 0;

    Vector<Vector<TQonvertImage>> srcImages(mipCount);
    Vector<Vector<TQonvertImage>> dstImages(mipCount);

    auto pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(imageSets[0][0]->format);
    auto atcSrcFormat = QualcommHelper::GetQualcommFromDava(imageSets[0][0]->format);
    auto atcDstFormat = QualcommHelper::GetQualcommFromDava(compressionFormat);

    for (uint32 f = 0; f < facesCount; ++f)
    {
        for (int32 m = 0; m < mipCount; ++m)
        {
            Image* image = imageSets[f][m];

            TQonvertImage& srcImg = srcImages[f][m];
            TQonvertImage& dstImg = dstImages[f][m];

            srcImg.nWidth = image->width;
            srcImg.nHeight = image->height;
            srcImg.nFormat = atcSrcFormat;
            srcImg.nDataSize = image->width * image->height * pixelSize;
            srcImg.pData = image->data;

            dstImg.nWidth = image->width;
            dstImg.nHeight = image->height;
            dstImg.nFormat = atcDstFormat;
            dstImg.nDataSize = 0;
            dstImg.pData = nullptr;

            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
            {
                Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", outFileName.GetAbsolutePathname().c_str());
                return false;
            }

            compressedDataSize += dstImg.nDataSize;
        }
    }

    Vector<unsigned char> compressedData(compressedDataSize);
    unsigned char* imageData = compressedData.data();
    for (uint32 f = 0; f < facesCount; ++f)
    {
        for (uint32 m = 0; m < mipCount; ++m)
        {
            TQonvertImage& srcImg = srcImages[f][m];
            TQonvertImage& dstImg = dstImages[f][m];

            dstImg.pData = imageData;
            imageData += dstImg.nDataSize;

            if (Qonvert(&srcImg, &dstImg) != Q_SUCCESS || dstImg.nDataSize == 0)
            {
                Logger::Error("[LibDdsHelper::WriteAtcFile] Error converting (%s).", outFileName.GetAbsolutePathname().c_str());
                return false;
            }
        }
    }

    return NvttHelper::WriteDdsFile(outFileName, compressionFormat, compressedData, imageSets[0][0]->width, imageSets[0][0]->height, mipCount, true);
#endif
}
}
}