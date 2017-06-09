#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>

class CompositeFilter : public FindFilter
{
public:
    CompositeFilter(const DAVA::Vector<std::shared_ptr<FindFilter>>& filters);

    ePackageStatus AcceptPackage(const PackageInformation* package) const override;
    bool AcceptControl(const ControlInformation* control) const override;

private:
    DAVA::Vector<std::shared_ptr<FindFilter>> filters;
};
