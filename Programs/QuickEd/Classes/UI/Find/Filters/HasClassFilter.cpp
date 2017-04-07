#include "UI/Find/Filters/HasClassFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <UI/UIControl.h>
#include <Utils/Utils.h>

using namespace DAVA;

HasClassFilter::HasClassFilter(const String& requiredClass_)
    : requiredClass(requiredClass_)
{
    for (const auto& field : ReflectedTypeDB::Get<UIControl>()->GetStructure()->fields)
    {
        if (field->name == "classes")
        {
            refMember = field.get();
        }
    }
}

bool HasClassFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool HasClassFilter::CanAcceptControl(const ControlInformation* control) const
{
    const Any& classesStr = control->GetControlPropertyValue(*refMember);
    if (classesStr.CanCast<String>())
    {
        Vector<String> classes;
        Split(classesStr.Cast<String>(), " ", classes);

        return std::find(classes.begin(), classes.end(), requiredClass) != classes.end();
    }
    else
    {
        return false;
    }
}
