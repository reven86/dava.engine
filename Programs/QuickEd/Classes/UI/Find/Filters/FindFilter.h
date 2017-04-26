#pragma once

class PackageInformation;
class ControlInformation;

class FindFilter
{
public:
    virtual ~FindFilter() = default;

    virtual bool CanAcceptPackage(const PackageInformation* package) const = 0;
    virtual bool CanAcceptControl(const ControlInformation* control) const = 0;
};
