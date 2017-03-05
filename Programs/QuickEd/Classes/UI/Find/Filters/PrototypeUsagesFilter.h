#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>

class PrototypeUsagesFilter : public FindFilter
{
public:
    PrototypeUsagesFilter(const DAVA::String& packagePath, const DAVA::FastName& prototypeName);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::String packagePath;
    DAVA::FastName prototypeName;
};
