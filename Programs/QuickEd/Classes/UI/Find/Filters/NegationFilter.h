#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>

class NegationFilter : public FindFilter
{
public:
    NegationFilter(std::shared_ptr<FindFilter> filter);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    std::shared_ptr<FindFilter> filter;
};
