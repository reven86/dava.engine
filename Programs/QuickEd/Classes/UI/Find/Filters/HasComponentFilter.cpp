#include "UI/Find/Filters/HasComponentFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

HasComponentFilter::HasComponentFilter(UIComponent::eType componentType)
    : requiredComponentType(componentType)
{
}

FindFilter::ePackageStatus HasComponentFilter::AcceptPackage(const PackageInformation* package) const
{
    return PACKAGE_CAN_ACCEPT_CONTROLS;
}

bool HasComponentFilter::AcceptControl(const ControlInformation* control) const
{
    return control->HasComponent(requiredComponentType);
}
