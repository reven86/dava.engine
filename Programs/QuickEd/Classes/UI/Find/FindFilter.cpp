#include "FindFilter.h"

#include "UI/Find/PackageInformation.h"

using namespace DAVA;

FindFilter::FindFilter()
{
}

FindFilter::~FindFilter()
{
}

PrototypeUsagesFilter::PrototypeUsagesFilter(const DAVA::String& packagePath_, const DAVA::FastName& prototypeName_)
    : packagePath(packagePath_)
    , prototypeName(prototypeName_)
{
}

PrototypeUsagesFilter::~PrototypeUsagesFilter()
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

CompositeFilter::CompositeFilter(const DAVA::Vector<std::shared_ptr<FindFilter>>& filters_)
    : filters(filters_)
{
}

CompositeFilter::~CompositeFilter()
{
}

bool CompositeFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return std::all_of(filters.begin(), filters.end(),
                       [&package](const std::shared_ptr<FindFilter>& filter)
                       {
                           return filter->CanAcceptPackage(package);
                       });
}

bool CompositeFilter::CanAcceptControl(const ControlInformation* control) const
{
    return std::all_of(filters.begin(), filters.end(),
                       [&control](const std::shared_ptr<FindFilter>& filter)
                       {
                           return filter->CanAcceptControl(control);
                       });
}

ControlNameFilter::ControlNameFilter(const DAVA::String& pattern, bool caseSensitive)
    : regExp(pattern.c_str(), caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)
{
}

ControlNameFilter::~ControlNameFilter()
{
}

bool ControlNameFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool ControlNameFilter::CanAcceptControl(const ControlInformation* control) const
{
    return regExp.exactMatch(control->GetName().c_str());
}

HasComponentFilter::HasComponentFilter(UIComponent::eType componentType)
    : requiredComponentType(componentType)
{
}

HasComponentFilter::~HasComponentFilter()
{
}

bool HasComponentFilter::CanAcceptPackage(const PackageInformation* package) const
{
    return true;
}

bool HasComponentFilter::CanAcceptControl(const ControlInformation* control) const
{
    return control->HasComponent(requiredComponentType);
}
