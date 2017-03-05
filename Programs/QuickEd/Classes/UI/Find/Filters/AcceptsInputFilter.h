#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>
#include <Base/IntrospectionBase.h>

class AcceptsInputFilter : public FindFilter
{
public:
    AcceptsInputFilter();

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    const DAVA::InspMember* inspMember = nullptr;
};
