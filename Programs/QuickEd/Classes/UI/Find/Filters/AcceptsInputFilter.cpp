#include "UI/Find/Filters/AcceptsInputFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <UI/UIControl.h>

using namespace DAVA;

AcceptsInputFilter::AcceptsInputFilter()
{
    inspMember = UIControl::TypeInfo()->Member(FastName("noInput"));
}

bool AcceptsInputFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool AcceptsInputFilter::CanAcceptControl(const ControlInformation* control) const
{
    const VariantType& noInput = control->GetControlPropertyValue(inspMember);
    if (noInput.GetType() == VariantType::TYPE_BOOLEAN)
    {
        return noInput.AsBool() == false;
    }
    else
    {
        return true;
    }
}
