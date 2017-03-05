#include "UI/Find/Filters/ControlNameFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

ControlNameFilter::ControlNameFilter(const DAVA::String& pattern, bool caseSensitive)
    : regExp(pattern.c_str(), caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)
{
}

bool ControlNameFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool ControlNameFilter::CanAcceptControl(const ControlInformation* control) const
{
    return regExp.exactMatch(control->GetName().c_str());
}
