#include "Tests/SaveImageTest.h"
#include "Infrastructure/GameCore.h"
#include "Infrastructure/TextureUtils.h"

using namespace DAVA;

SaveImageTest::SaveImageTest()
: TestTemplate<SaveImageTest>("SaveImageTest")
{
    imageRGBA8888 = Get8888Image();
    imageRGB888 = Get888Image();
    imageA8 = GetA8Image();
    RegisterFunction(this, &SaveImageTest::PngTest, "PngTest", NULL);
    RegisterFunction(this, &SaveImageTest::JpegTest, "JpegTest", NULL);
    RegisterFunction(this, &SaveImageTest::TgaTest, "TgaTest", NULL);
}

SaveImageTest::~SaveImageTest()
{
    SafeRelease(imageRGBA8888);
    SafeRelease(imageRGB888);
    SafeRelease(imageA8);
}

const float32 LOSSY_ALLOWED_DIFF = 2.f; //in percents
const float32 LOSSELESS_ALLOWED_DIFF = 0.f; //in percents

void SaveImageTest::PngTest(PerfFuncData * data)
{
    SaveLoadCheck(data, imageRGBA8888, "testRGBA8888.png", LOSSELESS_ALLOWED_DIFF);
    //SaveLoadCheck(data, imageRGB888, "testRGB888.png", LOSSELESS_ALLOWED_DIFF); -- RGB888 is not supported by PNG
    SaveLoadCheck(data, imageA8, "testA8.png", LOSSELESS_ALLOWED_DIFF);
}

void SaveImageTest::JpegTest(PerfFuncData * data)
{
    //SaveLoadCheck(data, imageRGBA8888, "testRGBA8888.jpg", LOSSY_ALLOWED_DIFF); -- RGBA8888 is not supported for JPEG
    SaveLoadCheck(data, imageRGB888, "testRGB888.jpg", LOSSY_ALLOWED_DIFF);
    SaveLoadCheck(data, imageA8, "testA8.jpg", LOSSY_ALLOWED_DIFF);
}

void SaveImageTest::TgaTest(PerfFuncData * data)
{
    SaveLoadCheck(data, imageRGBA8888, "testRGBA8888.tga", LOSSELESS_ALLOWED_DIFF);
    SaveLoadCheck(data, imageRGB888, "testRGB888.tga", LOSSELESS_ALLOWED_DIFF);
    SaveLoadCheck(data, imageA8, "testA8.tga", LOSSELESS_ALLOWED_DIFF);
}

void SaveImageTest::SaveLoadCheck(PerfFuncData* data, const Image* img, const String& filename, float32 diffThreshold)
{
    FilePath path = FilePath::FilepathInDocuments(filename);

    DAVA::Vector<DAVA::Image*> imgSet;

    TEST_VERIFY(img->Save(path));

    TEST_VERIFY(DAVA::ImageSystem::Instance()->Load(path, imgSet) == DAVA::SUCCESS);
    TEST_VERIFY(imgSet[0]->dataSize == img->dataSize);

    const TextureUtils::CompareResult cmpRes = TextureUtils::CompareImages(img, imgSet[0], img->format);
    float32 differencePersentage = ((float32)cmpRes.difference / ((float32)cmpRes.bytesCount * 256.f)) * 100.f;
    TEST_VERIFY(differencePersentage <= diffThreshold);
}

Image* SaveImageTest::Get8888Image() const
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
            *_date++ = 0xFF;                // A channel
        }
    }
    return img;
}

Image* SaveImageTest::Get888Image() const
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

Image* SaveImageTest::GetA8Image() const
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