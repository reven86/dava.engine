#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Infrastructure/TextureUtils.h"

using namespace DAVA;

DAVA_TESTCLASS (GPUFamilyTest)
{
    DAVA_TEST (TestFunction)
    {
        TEST_VERIFY(Texture::GetDefaultGPU() == DeviceInfo::GetGPUFamily());
        
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_WIN_UAP__)
        TEST_VERIFY(Texture::GetDefaultGPU() == GPU_DX11);
#elif defined(__DAVAENGINE_IPHONE__)
        TEST_VERIFY(Texture::GetDefaultGPU() == GPU_POWERVR_IOS);
#elif defined(__DAVAENGINE_ANDROID__)
        auto gpu = Texture::GetDefaultGPU();

        TEST_VERIFY(
        gpu == GPU_POWERVR_ANDROID
        || gpu == GPU_TEGRA
        || gpu == GPU_MALI
        || gpu == GPU_ADRENO
        );
#else
        TEST_VERIFY(false);
#endif
    }
};
