#pragma once

#include "UI/Find/PackageInformation/PackageInformation.h"

class PackageNode;

class PackageNodeInformation
: public PackageInformation
{
public:
    PackageNodeInformation(const PackageNode* packageNode);

    DAVA::String GetPath() const override;

    void VisitImportedPackages(const DAVA::Function<void(const PackageInformation*)>& visitor) const override;
    void VisitControls(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;
    void VisitPrototypes(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;

private:
    const PackageNode* packageNode;
};
