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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Infrastructure/TextureUtils.h"

using namespace DAVA;

const float32 LOSSY_ALLOWED_DIFF = 2.f; //in percents
const float32 LOSSLESS_ALLOWED_DIFF = 0.f; //in percents

DAVA_TESTCLASS(SaveImageTest)
{
    Image* imageRGBA8888 = nullptr;
    Image* imageRGB888 = nullptr;
    Image* imageA8 = nullptr;

    SaveImageTest()
    {
        imageRGBA8888 = Create8888Image();
        imageRGB888 = Create888Image();
        imageA8 = CreateA8Image();
    }
    ~SaveImageTest()
    {
        SafeRelease(imageRGBA8888);
        SafeRelease(imageRGB888);
        SafeRelease(imageA8);
    }

    DAVA_TEST(PngTest)
    {
        SaveLoadCheck(imageRGBA8888, "testRGBA8888.png", LOSSLESS_ALLOWED_DIFF);
        //SaveLoadCheck(imageRGB888, "testRGB888.png", LOSSELESS_ALLOWED_DIFF); -- RGB888 is not supported by PNG
        SaveLoadCheck(imageA8, "testA8.png", LOSSLESS_ALLOWED_DIFF);
    }

    DAVA_TEST(JpegTest)
    {
        //SaveLoadCheck(imageRGBA8888, "testRGBA8888.jpg", LOSSY_ALLOWED_DIFF); -- RGBA8888 is not supported for JPEG
        SaveLoadCheck(imageRGB888, "testRGB888.jpg", LOSSY_ALLOWED_DIFF);
        SaveLoadCheck(imageA8, "testA8.jpg", LOSSY_ALLOWED_DIFF);
    }

    DAVA_TEST(TgaTest)
    {
        SaveLoadCheck(imageRGBA8888, "testRGBA8888.tga", LOSSLESS_ALLOWED_DIFF);
        SaveLoadCheck(imageRGB888, "testRGB888.tga", LOSSLESS_ALLOWED_DIFF);
        SaveLoadCheck(imageA8, "testA8.tga", LOSSLESS_ALLOWED_DIFF);
    }

    DAVA_TEST(WebPTest)
    {
        SaveLoadCheck(imageRGB888, "testRGB888.webp", LOSSY_ALLOWED_DIFF);
        SaveLoadCheck(imageRGBA8888, "testRGBA8888.webp", LOSSY_ALLOWED_DIFF);
    }

    void SaveLoadCheck(const Image* inImage, const String& filename, float32 diffThreshold)
    {
        FilePath path = FilePath::FilepathInDocuments(filename);

        DAVA::Vector<DAVA::Image*> imgSet;

        TEST_VERIFY(inImage->Save(path));

        TEST_VERIFY(DAVA::ImageSystem::Instance()->Load(path, imgSet) == DAVA::eErrorCode::SUCCESS);
        TEST_VERIFY(imgSet[0]->dataSize == inImage->dataSize);

        const TextureUtils::CompareResult cmpRes = TextureUtils::CompareImages(inImage, imgSet[0], inImage->format);
        float32 differencePersentage = ((float32)cmpRes.difference / ((float32)cmpRes.bytesCount * 256.f)) * 100.f;
        TEST_VERIFY(differencePersentage <= diffThreshold);

        for (auto img : imgSet)
        {
            img->Release();
        }
    }

    Image* Create8888Image() const
    {
        uint32 size = 512;
        Image* img = Image::Create(size, size, FORMAT_RGBA8888);
        uint8* _date = img->data;
        for (uint32 i1 = 0; i1 < size; ++i1)
        {
            uint8 blue = 0xFF * i1 / size;
            for (uint32 i2 = 0; i2 < size; ++i2)
            {
                *_date++ = 0xFF * i2 / size;    // R channel, 0 to FF horizontally
                *_date++ = 0x00;                // G channel
                *_date++ = blue;                // B channel, 0 to FF vertically
                *_date++ = 0xFA;                // A channel
            }
        }
        return img;
    }

    Image* Create888Image() const
    {
        uint32 size = 512;
        Image* img = Image::Create(size, size, FORMAT_RGB888);
        uint8* _date = img->data;
        for (uint32 i1 = 0; i1 < size; ++i1)
        {
            uint8 blue = 0xFF * i1 / size;
            for (uint32 i2 = 0; i2 < size; ++i2)
            {
                *_date++ = 0xFF * i2 / size;    // R channel, 0 to FF horizontally
                *_date++ = 0x00;                // G channel
                *_date++ = blue;                // B channel, 0 to FF vertically
            }
        }
        return img;
    }

    Image* CreateA8Image() const
    {
        uint32 size = 512;
        Image* img = Image::Create(size, size, FORMAT_A8);
        uint8* _date = img->data;
        for (uint32 i1 = 0; i1 < size; ++i1)
        {
            uint8 p0 = (0xFF / 2) * i1 / size;
            for (uint32 i2 = 0; i2 < size; ++i2)
            {
                *_date++ = ((0xFF - p0) * i2 / size) + p0; // gradient from black to white horizontally, from black to gray vertically
            }
        }
        return img;
    }
};
