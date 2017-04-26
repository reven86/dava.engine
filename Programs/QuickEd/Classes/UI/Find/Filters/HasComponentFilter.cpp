#include "UI/Find/Filters/HasComponentFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

HasComponentFilter::HasComponentFilter(const DAVA::Type* componentType)
    : requiredComponentType(componentType)
{
}

bool HasComponentFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool HasComponentFilter::CanAcceptControl(const ControlInformation* control) const
{
    return control->HasComponent(requiredComponentType);
}
