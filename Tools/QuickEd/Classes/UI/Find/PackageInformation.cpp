#include "PackageInformation.h"

#include "Debug/DVAssert.h"

PackageInformation::PackageInformation(const DAVA::String& path_)
    : path(path_)
{
}

const DAVA::String& PackageInformation::GetPath() const
{
    return path;
}

void PackageInformation::AddImportedPackage(const std::shared_ptr<PackageInformation>& package)
{
    importedPackages.push_back(package);
}

void PackageInformation::AddControl(const std::shared_ptr<ControlInformation>& control)
{
    controls.push_back(control);
}

void PackageInformation::AddPrototype(const std::shared_ptr<ControlInformation>& prototype)
{
    prototypes.push_back(prototype);
}

const DAVA::Vector<std::shared_ptr<PackageInformation>>& PackageInformation::GetImportedPackages() const
{
    return importedPackages;
}

const DAVA::Vector<std::shared_ptr<ControlInformation>>& PackageInformation::GetPrototypes() const
{
    return prototypes;
}

const DAVA::Vector<std::shared_ptr<ControlInformation>>& PackageInformation::GetControls() const
{
    return controls;
}

std::shared_ptr<ControlInformation> PackageInformation::FindPrototypeByName(const DAVA::FastName& name) const
{
    for (const std::shared_ptr<ControlInformation>& prototype : prototypes)
    {
        if (prototype->GetName() == name)
        {
            return prototype;
        }
    }
    return std::shared_ptr<ControlInformation>();
}

PackageInformationCache::PackageInformationCache()
{
}

void PackageInformationCache::Put(const std::shared_ptr<PackageInformation>& package)
{
    DVASSERT(packages.find(package->GetPath()) == packages.end());
    packages[package->GetPath()] = package;
}

std::shared_ptr<PackageInformation> PackageInformationCache::Find(const DAVA::String& path)
{
    auto it = packages.find(path);
    if (it != packages.end())
    {
        return it->second;
    }

    return std::shared_ptr<PackageInformation>();
}
