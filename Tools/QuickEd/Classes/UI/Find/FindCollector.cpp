#include "FindCollector.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

#include "UI/Find/PackageInformationBuilder.h"

#include "UI/UIPackageLoader.h"

using namespace DAVA;

FindCollector::FindCollector()
{
}

FindCollector::~FindCollector()
{
}

void FindCollector::CollectFiles(const FileSystemCache* cache, const FindFilter& filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes)
{
    QStringList files = cache->GetFiles("yaml");

    PackageInformationCache packagesCache;

    for (const QString& pathStr : files)
    {
        FilePath path(pathStr.toStdString());
        if (path.GetFrameworkPath().find("~res:/UI/TechTree/") == -1 && path.GetFrameworkPath().find("~res:/UI/Fonts/") == -1)
        {
            PackageInformationBuilder builder(&packagesCache);

            if (UIPackageLoader(prototypes).LoadPackage(path, &builder))
            {
                const std::shared_ptr<PackageInformation>& package = builder.GetPackage();
                if (filter.CanAcceptPackage(package))
                {
                    for (const std::shared_ptr<ControlInformation>& control : package->GetControls())
                    {
                        CollectControls(path, control, filter, false);
                    }
                    for (const std::shared_ptr<ControlInformation>& prototype : package->GetPrototypes())
                    {
                        CollectControls(path, prototype, filter, true);
                    }
                }
            }
        }
    }

    std::sort(items.begin(), items.end());
}

const DAVA::Vector<FindItem>& FindCollector::GetItems()
{
    return items;
}

void FindCollector::CollectControls(const FilePath& path, const std::shared_ptr<ControlInformation>& control, const FindFilter& filter, bool inPrototypeSection)
{
    if (filter.CanAcceptControl(control))
    {
        items.push_back(FindItem(path, control->GetPathToControl()));
    }

    for (const std::shared_ptr<ControlInformation>& child : control->GetChildren())
    {
        CollectControls(path, child, filter, inPrototypeSection);
    }
}
