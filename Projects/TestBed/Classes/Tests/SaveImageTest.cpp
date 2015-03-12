#include "SaveImageTest.h"
#include "GameCore.h"

using namespace DAVA;

SaveImageTest::SaveImageTest()
: TestTemplate<SaveImageTest>("SaveImageTest")
{
    RegisterFunction(this, &SaveImageTest::PngTest, "PngTest", NULL);
    RegisterFunction(this, &SaveImageTest::JpegTest, "JpegTest", NULL);
}

void SaveImageTest::PngTest(PerfFuncData * data)
{
    Image* img = GetImage();
    FilePath path = FilePath::FilepathInDocuments("testImage.png");
    
    TEST_VERIFY(img->Save(path));
    
    SafeRelease(img);
}

void SaveImageTest::JpegTest(PerfFuncData * data)
{
    Image* img = GetImage();
    FilePath path = FilePath::FilepathInDocuments("testImage.jpeg");
    
    TEST_VERIFY(img->Save(path));
    
    SafeRelease(img);
}

Image* SaveImageTest::GetImage() const
{
    uint32 size = 512;
    Image* img = Image::Create(size, size, FORMAT_RGBA8888);
    uint8* _date = img->data;
    for (uint32 i1 = 0; i1 < size; ++i1)
    {
        for (uint32 i2 = 0; i2 < size; ++i2)
        {
            *_date++ = 0xFF;    // R
            *_date++ = 0x0;    // G
            *_date++ = 0x0;    // B
            *_date++ = 0xFF;    // A
        }
    }
    return img;
}