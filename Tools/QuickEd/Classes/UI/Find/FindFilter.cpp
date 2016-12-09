#include "FindFilter.h"

#include "UI/Find/PackageInformationBuilder.h"
#include "Model/PackageHierarchy/ControlNode.h"

using namespace DAVA;

FindFilter::FindFilter(const DAVA::String& packagePath_, const DAVA::String& prototypeName_)
    : packagePath(packagePath_)
    , prototypeName(prototypeName_)
{
}

FindFilter::~FindFilter()
{
}

bool FindFilter::CanAcceptPackage(PackageInformationBuilder& builder) const
{
    std::shared_ptr<PackageInformation> package = builder.GetPackage();
    const DAVA::Vector<std::shared_ptr<PackageInformation>>& packages = package->GetImportedPackages();
    return std::find_if(packages.begin(), packages.end(), [this](const std::shared_ptr<PackageInformation>& pack) {
               return pack->GetPath() == packagePath;
           }) != packages.end();
}

bool FindFilter::CanAcceptRootControl(const ControlNode* node) const
{
    return false;
}

bool FindFilter::CanAcceptPrototype(const ControlNode* node) const
{
    return false;
}
