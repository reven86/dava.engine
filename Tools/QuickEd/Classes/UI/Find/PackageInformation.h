#pragma once

#include "Base/BaseTypes.h"
#include "ControlInformation.h"

class PackageInformation
{
public:
    PackageInformation(const DAVA::String& path);

    const DAVA::String& GetPath() const;

    void AddImportedPackage(const std::shared_ptr<PackageInformation>& package);
    void AddControl(const std::shared_ptr<ControlInformation>& control);
    void AddPrototype(const std::shared_ptr<ControlInformation>& prototype);

    const DAVA::Vector<std::shared_ptr<PackageInformation>>& GetImportedPackages() const;
    std::shared_ptr<ControlInformation> FindPrototypeByName(const DAVA::FastName& name) const;

private:
    DAVA::String path;
    DAVA::Vector<std::shared_ptr<PackageInformation>> importedPackages;
    DAVA::Vector<std::shared_ptr<ControlInformation>> controls;
    DAVA::Vector<std::shared_ptr<ControlInformation>> prototypes;
};

class PackageInformationCache
{
public:
    PackageInformationCache();

    void Put(const std::shared_ptr<PackageInformation>& package);
    std::shared_ptr<PackageInformation> Find(const DAVA::String& path);

private:
    DAVA::Map<DAVA::String, std::shared_ptr<PackageInformation>> packages;
};
