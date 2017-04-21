#include "UI/Find/Filters/CompositeFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <algorithm>

using namespace DAVA;

CompositeFilter::CompositeFilter(const DAVA::Vector<std::shared_ptr<FindFilter>>& filters_)
    : filters(filters_)
{
}

bool CompositeFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return std::any_of(filters.begin(), filters.end(),
                       [&package](const std::shared_ptr<FindFilter>& filter)
                       {
                           return filter->CanAcceptPackage(package);
                       });
}

bool CompositeFilter::CanAcceptControl(const ControlInformation* control) const
{
    return std::all_of(filters.begin(), filters.end(),
                       [&control](const std::shared_ptr<FindFilter>& filter)
                       {
                           return filter->CanAcceptControl(control);
                       });
}
