#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>
#include <Reflection/ReflectedStructure.h>

class HasClassFilter : public FindFilter
{
public:
    HasClassFilter(const DAVA::String& requiredClass);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::String requiredClass;
    const DAVA::ReflectedStructure::Field* refMember = nullptr;
};
