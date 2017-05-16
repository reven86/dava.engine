#include "UI/Find/Filters/CompositeFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <algorithm>

using namespace DAVA;

CompositeFilter::CompositeFilter(const DAVA::Vector<std::shared_ptr<FindFilter>>& filters_)
    : filters(filters_)
{
}

FindFilter::ePackageStatus CompositeFilter::AcceptPackage(const PackageInformation* package) const
{
    ePackageStatus result = ePackageStatus::PACKAGE_FOUND;
    for (const std::shared_ptr<FindFilter>& filter : filters)
    {
        ePackageStatus status = filter->AcceptPackage(package);
        if (status == PACKAGE_NOT_INTERESTED)
        {
            return PACKAGE_NOT_INTERESTED;
        }
        if (status == PACKAGE_CAN_ACCEPT_CONTROLS)
        {
            result = PACKAGE_CAN_ACCEPT_CONTROLS;
        }
    }
    return result;
}

bool CompositeFilter::AcceptControl(const ControlInformation* control) const
{
    return std::all_of(filters.begin(), filters.end(),
                       [&control](const std::shared_ptr<FindFilter>& filter)
                       {
                           return filter->AcceptControl(control);
                       });
}
