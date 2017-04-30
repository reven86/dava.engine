#include "UI/Find/Filters/HasErrorsFilter.h"

#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

HasErrorsFilter::HasErrorsFilter()
{
}

bool HasErrorsFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool HasErrorsFilter::CanAcceptControl(const ControlInformation* control) const
{
    return control->HasErrors();
}
