#include "UI/Find/Filters/PrototypeUsagesFilter.h"
#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

using namespace DAVA;

PrototypeUsagesFilter::PrototypeUsagesFilter(const DAVA::String& packagePath_, const DAVA::FastName& prototypeName_)
    : packagePath(packagePath_)
    , prototypeName(prototypeName_)
{
}

bool PrototypeUsagesFilter::CanAcceptPackage(const PackageInformation* package) const
{
    if (package->GetPath() == packagePath)
    {
        return true;
    }

    bool imports = false;

    package->VisitImportedPackages(
    [&imports, this](const PackageInformation* package)
    {
        if (package->GetPath() == packagePath)
        {
            imports = true;
        }
    });

    return imports;
}

bool PrototypeUsagesFilter::CanAcceptControl(const ControlInformation* control) const
{
    return control->GetPrototype() == prototypeName && control->GetPrototypePackagePath() == packagePath;
}
