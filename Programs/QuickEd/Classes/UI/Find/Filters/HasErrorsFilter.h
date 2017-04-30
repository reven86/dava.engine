#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include "UI/Find/Filters/FindFilter.h"

class HasErrorsFilter : public FindFilter
{
public:
    HasErrorsFilter();

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;
};
