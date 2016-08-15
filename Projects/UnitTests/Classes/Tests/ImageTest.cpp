#include "UnitTests/UnitTests.h"

#include "Base/BaseTypes.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Utils/Random.h"

using namespace DAVA;

DAVA_TESTCLASS (ImageTest)
{
    DAVA_TEST (DownscaleTest)
    {
        struct TestData
        {
            PixelFormat format;
            uint32 width;
            uint32 height;
            bool willBeDownscaled;
        };

        static Vector<TestData> tests =
        {
          { PixelFormat::FORMAT_RGBA8888, 8, 8, true },
          { PixelFormat::FORMAT_RGBA5551, 8, 8, true },
          { PixelFormat::FORMAT_RGBA4444, 8, 8, false },
          { PixelFormat::FORMAT_RGB888, 8, 8, true },
          { PixelFormat::FORMAT_RGB565, 8, 8, false },
          { PixelFormat::FORMAT_A8, 8, 8, true },
          { PixelFormat::FORMAT_A16, 8, 8, false },
          { PixelFormat::FORMAT_PVR4, 8, 8, false },
          { PixelFormat::FORMAT_PVR2, 8, 8, false },
          { PixelFormat::FORMAT_RGBA16161616, 8, 8, true },
          { PixelFormat::FORMAT_RGBA32323232, 8, 8, true },
          { PixelFormat::FORMAT_DXT1, 8, 8, false },
          { PixelFormat::FORMAT_DXT1A, 8, 8, false },
          { PixelFormat::FORMAT_DXT3, 8, 8, false },
          { PixelFormat::FORMAT_DXT5, 8, 8, false },
          { PixelFormat::FORMAT_DXT5NM, 8, 8, false },
          { PixelFormat::FORMAT_ETC1, 8, 8, false },
          { PixelFormat::FORMAT_ATC_RGB, 8, 8, false },
          { PixelFormat::FORMAT_ATC_RGBA_EXPLICIT_ALPHA, 8, 8, false },
          { PixelFormat::FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, 8, 8, false },
#if (defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__))
          { PixelFormat::FORMAT_RGBA16F, 8, 8, true },
          { PixelFormat::FORMAT_RGBA32F, 8, 8, true },
#endif //#if (defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__))
        };

        for (const TestData& td : tests)
        {
            ScopedPtr<Image> source(Image::Create(td.width, td.height, td.format));
            TEST_VERIFY(source);

            uint8 colorByte = Random::Instance()->Rand(255);
            colorByte = 255;
            Memset(source->GetData(), colorByte, source->GetDataSize());

            ScopedPtr<Image> destination(ImageConvert::DownscaleTwiceBillinear(source));
            if (td.willBeDownscaled)
            {
                TEST_VERIFY(destination);
                TEST_VERIFY(destination.get() != source.get());

                TEST_VERIFY(destination->GetPixelFormat() == td.format);
                TEST_VERIFY(destination->GetWidth() == (td.width / 2));
                TEST_VERIFY(destination->GetHeight() == (td.height / 2));

                TEST_VERIFY(Memcmp(source->GetData(), destination->GetData(), destination->GetDataSize()) == 0);
            }
            else
            {
                TEST_VERIFY(!destination);
            }
        }
    }
};
