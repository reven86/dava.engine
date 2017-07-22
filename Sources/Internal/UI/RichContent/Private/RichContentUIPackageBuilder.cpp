#include "UI/RichContent/Private/RichContentUIPackageBuilder.h"

#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIPackagesCache.h"

namespace DAVA
{
RichContentUIPackageBuilder::RichContentUIPackageBuilder(UIPackagesCache* _cache)
    : DefaultUIPackageBuilder(_cache)
{
}

RefPtr<UIControl> RichContentUIPackageBuilder::CreateControlByName(const String& customClassName, const String& className)
{
    if (ObjectFactory::Instance()->IsTypeRegistered(customClassName))
    {
        return RefPtr<UIControl>(ObjectFactory::Instance()->New<UIControl>(customClassName));
    }
    return RefPtr<UIControl>(ObjectFactory::Instance()->New<UIControl>(className));
}

std::unique_ptr<DefaultUIPackageBuilder> RichContentUIPackageBuilder::CreateBuilder(UIPackagesCache* packagesCache)
{
    return std::make_unique<RichContentUIPackageBuilder>(packagesCache);
}
}
