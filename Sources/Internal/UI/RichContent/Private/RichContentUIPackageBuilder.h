#pragma once

#include "UI/DefaultUIPackageBuilder.h"

namespace DAVA
{
class UIPackagesCache;

class RichContentUIPackageBuilder : public DefaultUIPackageBuilder
{
public:
    RichContentUIPackageBuilder(UIPackagesCache* _cache = nullptr);

protected:
    RefPtr<DAVA::UIControl> CreateControlByName(const String& customClassName, const String& className) override;
    std::unique_ptr<DefaultUIPackageBuilder> CreateBuilder(UIPackagesCache* packagesCache) override;
};
}