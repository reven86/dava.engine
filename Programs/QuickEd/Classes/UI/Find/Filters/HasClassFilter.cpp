#include "UI/Find/Filters/HasClassFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <UI/UIControl.h>
#include <Utils/Utils.h>

using namespace DAVA;

HasClassFilter::HasClassFilter(const String& requiredClass_)
    : requiredClass(requiredClass_)
{
    inspMember = UIControl::TypeInfo()->Member(FastName("classes"));
}

bool HasClassFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool HasClassFilter::CanAcceptControl(const ControlInformation* control) const
{
    const VariantType& classesStr = control->GetControlPropertyValue(inspMember);
    if (classesStr.GetType() == VariantType::TYPE_STRING)
    {
        Vector<String> classes;
        Split(classesStr.AsString(), " ", classes);

        return std::find(classes.begin(), classes.end(), requiredClass) != classes.end();
    }
    else
    {
        return false;
    }
}
