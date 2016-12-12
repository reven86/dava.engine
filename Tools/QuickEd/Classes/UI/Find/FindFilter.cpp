#include "FindFilter.h"

#include "UI/Find/PackageInformation.h"
#include "Model/PackageHierarchy/ControlNode.h"

using namespace DAVA;

FindFilter::FindFilter(const DAVA::String& packagePath_, const DAVA::FastName& prototypeName_)
    : packagePath(packagePath_)
    , prototypeName(prototypeName_)
{
}

FindFilter::~FindFilter()
{
}

bool FindFilter::CanAcceptPackage(const std::shared_ptr<PackageInformation>& package) const
{
    if (package->GetPath() == packagePath)
    {
        return true;
    }

    const DAVA::Vector<std::shared_ptr<PackageInformation>>& packages = package->GetImportedPackages();
    return std::find_if(packages.begin(), packages.end(), [this](const std::shared_ptr<PackageInformation>& pack) {
               return pack->GetPath() == packagePath;
           }) != packages.end();
}

bool FindFilter::CanAcceptControl(const std::shared_ptr<ControlInformation>& control) const
{
    return control->GetPrototype() == prototypeName && control->GetPrototypePackagePath() == packagePath;
}
