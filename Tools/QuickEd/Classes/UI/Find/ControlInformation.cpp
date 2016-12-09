#include "ControlInformation.h"

#include "PackageInformation.h"

ControlInformation::ControlInformation(const DAVA::FastName& name_)
    : name(name_)
{
}

ControlInformation::ControlInformation(const DAVA::FastName& name_, const std::shared_ptr<PackageInformation> prototypePackage_, const DAVA::FastName& prototype_)
    : name(name_)
    , prototypePackage(prototypePackage_)
    , prototype(prototype_)
{
}

const DAVA::FastName& ControlInformation::GetName() const
{
    return name;
}

void ControlInformation::AddChild(const std::shared_ptr<ControlInformation>& child)
{
    children.push_back(child);
}
