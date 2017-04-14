#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>
#include <QRegExp>

class ControlNameFilter : public FindFilter
{
public:
    ControlNameFilter(const DAVA::String& pattern, bool caseSensitive);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    QRegExp regExp;
};
