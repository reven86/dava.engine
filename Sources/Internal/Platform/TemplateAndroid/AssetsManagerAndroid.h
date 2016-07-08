#ifndef __DAVAENGINE_ASSETS_MANAGER_ANDROID_H__
#define __DAVAENGINE_ASSETS_MANAGER_ANDROID_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

#if defined(__DAVAENGINE_ANDROID__)

class zip;

namespace DAVA
{
class AssetsManager : public Singleton<AssetsManager>
{
public:
    AssetsManager();
    virtual ~AssetsManager();

    void Init(const String& packageName);

    const String& GetPackageName() const
    {
        return packageName;
    };
    zip* GetApplicationPackage() const
    {
        return applicationPackage;
    };

private:
    String packageName;

    zip* applicationPackage;
};
};

#endif // #if defined(__DAVAENGINE_ANDROID__)

#endif // __DAVAENGINE_ASSETS_MANAGER_ANDROID_H__
