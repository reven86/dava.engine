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



#ifndef __DAVAENGINE_TGA_HELPER_H__
#define __DAVAENGINE_TGA_HELPER_H__

#include <memory>

#include "Render/Image/ImageFormatInterface.h"

namespace DAVA 
{

class LibTgaHelper: public ImageFormatInterface
{
public:
    
    LibTgaHelper();

    ImageFormat GetImageFormat() const override;

    bool CanProcessFile(File* file) const override;

    eErrorCode ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap = 0) const override;

    eErrorCode WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat, ImageQuality quality) const override;
    
    ImageInfo GetImageInfo(File *infile) const override;

    struct TgaInfo
    {
        enum IMAGE_TYPE : uint8
        {
            TRUECOLOR = 2,
            GRAYSCALE = 3,
            COMPRESSED_TRUECOLOR = 10,
            COMPRESSED_GRAYSCALE = 11
        };
        enum ORIGIN_CORNER : uint8
        {
            BOTTOM_LEFT = 0,
            BOTTOM_RIGHT = 1,
            TOP_LEFT = 2,
            TOP_RIGHT = 3
        };

        uint16 width;
        uint16 height;
        uint8 bytesPerPixel;
        uint8 alphaBits;
        IMAGE_TYPE imageType;
        ORIGIN_CORNER origin_corner;
        PixelFormat pixelFormat;
    };

    eErrorCode ReadTgaHeader(const FilePath& filepath, TgaInfo& tgaHeader) const;

private:

    eErrorCode ReadTgaHeader(File *infile, TgaInfo& tgaHeader) const;

    eErrorCode ReadCompressedTga(File *infile, const TgaInfo& tgaHeader, ScopedPtr<Image>& image) const;
    eErrorCode ReadUncompressedTga(File *infile, const TgaInfo& tgaHeader, ScopedPtr<Image>& image) const;
    PixelFormat DefinePixelFormat(const TgaInfo& tgaHeader) const;

    eErrorCode WriteTgaHeader(File *outfile, const TgaInfo& tgaHeader) const;
    eErrorCode WriteUncompressedTga(File *infile, const TgaInfo& tgaHeader, const uint8* data) const;

    struct ImageDataWriter
    {
        ImageDataWriter(Image* image, const LibTgaHelper::TgaInfo& tgaInfo);
        inline bool AtEnd() const { return isAtEnd; }
        void Write(uint8* pixel);

    private:
        void ResetPtr(const Image* image, const LibTgaHelper::TgaInfo& tgaInfo);
        void IncrementPtr();

    public:
        uint8* ptr;
        ptrdiff_t ptrNextLineJump;
        ptrdiff_t ptrInc;

    private:
        const TgaInfo& tgaInfo;
        uint16 linesRemaining;
        uint16 linePixelsRemaining;
        bool isAtEnd;
    };
};

inline ImageFormat LibTgaHelper::GetImageFormat() const
{
    return IMAGE_FORMAT_TGA;
}

};

#endif // __DAVAENGINE_TGA_HELPER_H__