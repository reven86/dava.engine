#include "FindIterator.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

#include "UI/Find/PackageInformationBuilder.h"

#include "UI/UIPackageLoader.h"

using namespace DAVA;

FindIterator::FindIterator()
{
}

FindIterator::~FindIterator()
{
}

void FindIterator::CollectFiles(FileSystemCache* cache, const FindFilter& filter, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes)
{
    QStringList files = cache->GetFiles("yaml");
    int index = 0;

    PackageInformationCache packagesCache;

    for (QString& pathStr : files)
    {
        FilePath path(pathStr.toStdString());
        index++;
        if (path.GetFrameworkPath().find("~res:/UI/TechTree/") == -1 &&
            path.GetFrameworkPath().find("~res:/UI/Fonts/") == -1)
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
                else
                {
                }
            }
            else
            {
                DVASSERT(false);
                Logger::Debug("  [failed]");
            }
        }
    }

    std::sort(items.begin(), items.end());
}

const DAVA::Vector<FindItem>& FindIterator::GetItems()
{
    return items;
}

void FindIterator::CollectControls(const FilePath& path, const std::shared_ptr<ControlInformation>& control, const FindFilter& filter, bool inPrototypeSection)
{
    if (filter.CanAcceptControl(control))
    {
        items.push_back(FindItem(path, control->GetPathToControl(), inPrototypeSection));
    }

    for (const std::shared_ptr<ControlInformation>& child : control->GetChildren())
    {
        CollectControls(path, child, filter, inPrototypeSection);
    }
}
