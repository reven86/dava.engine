#include "FindCollector.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

#include "UI/Find/PackageInformationBuilder.h"

#include "UI/UIPackageLoader.h"

using namespace DAVA;

FindCollector::FindCollector(const FileSystemCache* cache_, std::unique_ptr<FindFilter> filter_, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes_)
    : cache(cache_)
    , filter(std::move(filter_))
    , prototypes(prototypes_)
{
}

FindCollector::~FindCollector()
{
}

const DAVA::Vector<FindItem>& FindCollector::GetItems() const
{
    return items;
}

void FindCollector::CollectFiles()
{
    QStringList files = cache->GetFiles("yaml");

    PackageInformationCache packagesCache;

    for (const QString& pathStr : files)
    {
        FilePath path(pathStr.toStdString());
        PackageInformationBuilder builder(&packagesCache);

        if (UIPackageLoader(*prototypes).LoadPackage(path, &builder))
        {
            const std::shared_ptr<PackageInformation>& package = builder.GetPackage();
            if (filter->CanAcceptPackage(package))
            {
                for (const std::shared_ptr<ControlInformation>& control : package->GetControls())
                {
                    CollectControls(path, control, false);
                }
                for (const std::shared_ptr<ControlInformation>& prototype : package->GetPrototypes())
                {
                    CollectControls(path, prototype, true);
                }
            }
        }
    }

    std::sort(items.begin(), items.end());
}

void FindCollector::CollectControls(const FilePath& path, const std::shared_ptr<ControlInformation>& control, bool inPrototypeSection)
{
    if (filter->CanAcceptControl(control))
    {
        Logger::Debug("FilePath: %s", path.GetAbsolutePathname().c_str());
        items.push_back(FindItem(path, control->GetPathToControl()));
    }

    for (const std::shared_ptr<ControlInformation>& child : control->GetChildren())
    {
        CollectControls(path, child, inPrototypeSection);
    }
}
