#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

class PackageInformation;

class ControlInformation
{
public:
    ControlInformation(const DAVA::FastName& name);
    ControlInformation(const DAVA::FastName& name, const std::shared_ptr<PackageInformation> prototypePackage, const DAVA::FastName& prototype);
    ControlInformation(const ControlInformation& other);

    const DAVA::FastName& GetName() const;
    void AddChild(const std::shared_ptr<ControlInformation>& child);

private:
    DAVA::FastName name;
    std::shared_ptr<PackageInformation> prototypePackage;
    DAVA::FastName prototype;

    DAVA::Vector<std::shared_ptr<ControlInformation>> children;
};
