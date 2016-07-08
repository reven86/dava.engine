#include "UIPackagesCache.h"

#include "UIPackage.h"

namespace DAVA
{
UIPackagesCache::UIPackagesCache(UIPackagesCache* _parent)
    : parent(SafeRetain(_parent))
{
}

UIPackagesCache::~UIPackagesCache()
{
    SafeRelease(parent);

    for (auto& it : packages)
        it.second->Release();

    packages.clear();
}

void UIPackagesCache::PutPackage(const String& path, UIPackage* package)
{
    auto it = packages.find(path);
    if (it == packages.end())
    {
        packages[path] = SafeRetain(package);
    }
    else
    {
        DVASSERT(it->second == package);
    }
}

UIPackage* UIPackagesCache::GetPackage(const String& path) const
{
    auto it = packages.find(path);
    if (it != packages.end())
        return it->second;

    if (parent)
        return parent->GetPackage(path);

    return nullptr;
}
}
