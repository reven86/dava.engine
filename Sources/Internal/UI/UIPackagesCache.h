#ifndef __DAVAENGINE_UI_PACKAGES_CACHE_H__
#define __DAVAENGINE_UI_PACKAGES_CACHE_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class UIPackage;

class UIPackagesCache : public BaseObject
{
public:
    UIPackagesCache(UIPackagesCache* _parent = nullptr);

protected:
    ~UIPackagesCache();

public:
    void PutPackage(const String& name, UIPackage* package);
    UIPackage* GetPackage(const String& name) const;

private:
    UIPackagesCache* parent;

    Map<String, UIPackage*> packages;
};
};
#endif // __DAVAENGINE_UI_PACKAGES_CACHE_H__
