#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Utils/Utils.h"
#include "zip/zip.h"

#if defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
AssetsManager::AssetsManager()
    : packageName("")
    , applicationPackage(NULL)
{
}

AssetsManager::~AssetsManager()
{
    if (applicationPackage)
    {
        zip_close(applicationPackage);
        applicationPackage = NULL;
    }
}

void AssetsManager::Init(const String& packageName)
{
    DVASSERT_MSG(applicationPackage == NULL, "[AssetsManager::Init] Package should be initialized only once.");

    applicationPackage = zip_open(packageName.c_str(), 0, NULL);
    if (applicationPackage == NULL)
    {
        DVASSERT_MSG(false, "[CorePlatformAndroid::InitApplicationPackage] Could not initialize application package.");
        applicationPackage = NULL;
    }
}
}

#endif // #if defined(__DAVAENGINE_ANDROID__)
