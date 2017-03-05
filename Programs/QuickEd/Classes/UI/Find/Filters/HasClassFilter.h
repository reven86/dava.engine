#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>
#include <Base/IntrospectionBase.h>

class HasClassFilter : public FindFilter
{
public:
    HasClassFilter(const DAVA::String& requiredClass);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::String requiredClass;
    const DAVA::InspMember* inspMember = nullptr;
};
