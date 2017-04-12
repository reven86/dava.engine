#include "UI/Find/Filters/NegationFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

NegationFilter::NegationFilter(std::shared_ptr<FindFilter> filter_)
    : filter(filter_)
{
}

bool NegationFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return filter->CanAcceptPackage(package);
}

bool NegationFilter::CanAcceptControl(const ControlInformation* control) const
{
    return !filter->CanAcceptControl(control);
}
