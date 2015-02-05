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

#include <array>
#include <memory>

#include "Render/Image/ImageFormatInterface.h"

namespace DAVA 
{

class LibTgaWrapper: public ImageFormatInterface
{
public:
    
    LibTgaWrapper();
    
    bool IsImage(File *file) const override;

    eErrorCode ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap = 0) const override;

    eErrorCode WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const override;
    eErrorCode WriteFileAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat) const override;
    
    uint32 GetDataSize(File *infile) const override;
	Size2i GetImageSize(File *infile) const override;

private:
    union TgaHeader
    {
        enum IMAGE_TYPE{
            TRUECOLOR = 2,
            GRAYSCALE = 3,
            COMPRESSED_TRUECOLOR = 10,
            COMPRESSED_GRAYSCALE = 11
        };
        enum ORIGIN_CORNER {
            BOTTOM_LEFT = 0,
            BOTTOM_RIGHT = 1,
            TOP_LEFT = 2,
            TOP_RIGHT = 3
        };
        struct {
            uint8 idlength;             // should be 0
            uint8 colorMapType;         // should be 0
            uint8 imageType;            // can be 2,3,10,11
            uint8 colorMapData[5];      // should be 0,0,0,0,0
            uint16 originX;             // should be 0
            uint16 originY;             // should be 0
            uint16 width;               // image width in pixels
            uint16 height;              // image height in pixels
            uint8 bpp;                  // can be 8,16,24,32,64,128
            uint8 :2, orig:2, alpha:4;  // image origin corner and number of alpha bits
        };
        std::array<uint8, 18> fields;
    };

    struct DataIterator
    {
        explicit DataIterator(Image* image, uint8 _pixSize);
        bool AtEnd() const { return isAtEnd; }

        virtual void Write(uint8* pixel) = 0;
        virtual void SetAtBegin() = 0;

    protected:
        uint8* data;
        uint16 width;
        uint16 height;
        uint8 pixelSize;
        uint8* ptr;
        uint16 x,y;
        bool isAtEnd;
    };

    struct BottomLeftIterator : public DataIterator
    {
        explicit BottomLeftIterator(Image* image, uint8 pixSize) : DataIterator(image, pixSize) {}
        void Write(uint8* pixel) override;
        void SetAtBegin() override;
    };

    struct BottomRightIterator : public DataIterator
    {
        explicit BottomRightIterator(Image* image, uint8 pixSize) : DataIterator(image, pixSize) {}
        void Write(uint8* pixel) override;
        void SetAtBegin() override;
    };

    struct TopLeftIterator : public DataIterator
    {
        explicit TopLeftIterator(Image* image, uint8 pixSize) : DataIterator(image, pixSize) {}
        void Write(uint8* pixel) override;
        void SetAtBegin() override;
    };

    struct TopRightIterator : public DataIterator
    {
        explicit TopRightIterator(Image* image, uint8 pixSize) : DataIterator(image, pixSize) {}
        void Write(uint8* pixel) override;
        void SetAtBegin() override;
    };

    DataIterator* CreateDataIterator(Image* image, uint8 pixSize, TgaHeader::ORIGIN_CORNER origin) const;

    eErrorCode ReadTgaHeader(File *infile, TgaHeader& tgaHeader, PixelFormat& pixelFormat) const;
    eErrorCode ReadCompressedTga(File *infile, const TgaHeader& tgaHeader, ScopedPtr<Image>& image) const;
    eErrorCode ReadUncompressedTga(File *infile, const TgaHeader& tgaHeader, ScopedPtr<Image>& image) const;
    PixelFormat DefinePixelFormat(const TgaHeader& tgaHeader) const;
};

};

#endif // __DAVAENGINE_TGA_HELPER_H__