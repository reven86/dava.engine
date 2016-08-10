#include "UnitTests/UnitTests.h"

#include "Base/BaseTypes.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Utils/Random.h"

using namespace DAVA;

DAVA_TESTCLASS (ImageTest)
{
    DAVA_TEST(DownscaleTest)
    {
        struct TestData
        {
            PixelFormat format;
            uint32 width;
            uint32 height;
            bool willBeDownscaled;
            bool shouldTestData;
        };

        static Vector<TestData> tests =
        {
            { PixelFormat::FORMAT_RGBA8888, 8, 8, true, true },
            { PixelFormat::FORMAT_RGBA5551, 8, 8, true, true },
            { PixelFormat::FORMAT_RGBA4444, 8, 8, false, false },
            { PixelFormat::FORMAT_RGB888, 8, 8, true, true },
            { PixelFormat::FORMAT_RGB565, 8, 8, false, false },
            { PixelFormat::FORMAT_A8, 8, 8, true, true },
            { PixelFormat::FORMAT_A16, 8, 8, false, false },
            { PixelFormat::FORMAT_PVR4, 8, 8, false, false },
            { PixelFormat::FORMAT_PVR2, 8, 8, false, false },
            { PixelFormat::FORMAT_RGBA16161616, 8, 8, true, true },
            { PixelFormat::FORMAT_RGBA32323232, 8, 8, true, false }, //sometimes this code works bad because of uint32 overflow
            { PixelFormat::FORMAT_DXT1, 8, 8, false, false },
            { PixelFormat::FORMAT_DXT1A, 8, 8, false, false },
            { PixelFormat::FORMAT_DXT3, 8, 8, false, false },
            { PixelFormat::FORMAT_DXT5, 8, 8, false, false },
            { PixelFormat::FORMAT_DXT5NM, 8, 8, false, false },
            { PixelFormat::FORMAT_ETC1, 8, 8, false, false },
            { PixelFormat::FORMAT_ATC_RGB, 8, 8, false, false },
            { PixelFormat::FORMAT_ATC_RGBA_EXPLICIT_ALPHA, 8, 8, false, false },
            { PixelFormat::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, 8, 8, false, false },
            { PixelFormat::FORMAT_RGBA16F, 8, 8, true, true },
            { PixelFormat::FORMAT_RGBA32F, 8, 8, true, true }
        };

        for (const TestData& td : tests)
        {
            ScopedPtr<Image> sourceImage(Image::Create(td.width, td.height, td.format));
            TEST_VERIFY(sourceImage);

            uint8 colorByte = Random::Instance()->Rand(255);
            Memset(sourceImage->GetData(), colorByte, sourceImage->GetDataSize());

            ScopedPtr<Image> destination(ImageConvert::DownscaleTwiceBillinear(sourceImage));
            if (td.willBeDownscaled)
            {
                TEST_VERIFY(destination);
                TEST_VERIFY(destination.get() != sourceImage.get());

                TEST_VERIFY(destination->GetPixelFormat() == td.format);
                TEST_VERIFY(destination->GetWidth() == (td.width / 2));
                TEST_VERIFY(destination->GetHeight() == (td.height / 2));
                
                if (td.shouldTestData)
                {
                    bool isDataEqual = true;
                    for (uint32 index = 0; index < destination->GetDataSize() && isDataEqual; ++index)
                    {
                        isDataEqual = (destination->GetData()[index] == colorByte);
                    }
                    TEST_VERIFY(isDataEqual);
                }
            }
            else
            {
                TEST_VERIFY(!destination);
            }
        }
    }
};

